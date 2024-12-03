#include<iostream>
#include<pthread.h>
#include<deque>
#include<vector>
#include<unistd.h>
#include<algorithm>

struct Order
{
    std::string id_;
    double price_;

    Order()=delete;
    Order(std::string id, double price): id_(id),price_(price){}

    std::string GetId(){ return id_; }
    double GetPrice(){ return price_; }

    bool operator==(const Order& rhs){ return this->id_ == rhs.id_; }

    friend std::ostream& operator<<(std::ostream& os,const Order& obj)
    {
        os<<"\nName: "<<obj.id_<<"  Price: "<<obj.price_<<'\n';
        return os;
    }

};



std::deque<Order> gOrders;
std::vector<std::pair<Order,double>> gStorage;

pthread_mutex_t mtxStream;
pthread_mutex_t mtxCommon;
pthread_mutex_t mtxFinish;
pthread_cond_t condForCheque;


void* CreateOrder(void* pOrderName)
{

    std::string orderName(*static_cast<std::string*>(pOrderName));

    pthread_mutex_lock(&mtxCommon);
    
    for(auto& i: gStorage)
        if(i.first.GetId()==orderName && i.second>0)
        {
            gOrders.push_back(i.first);    
            --i.second;
            pthread_mutex_unlock(&mtxCommon);
            return nullptr;
        }
    
    pthread_mutex_unlock(&mtxCommon);

    pthread_mutex_lock(&mtxStream);
    std::cout<<"There is no such item in storage\n";
    pthread_mutex_unlock(&mtxStream);

    return nullptr;
}

void* RemoveOrder(void* pOrderName)
{
    
    std::string orderName(*static_cast<std::string*>(pOrderName));

    pthread_mutex_lock(&mtxCommon);
    
    auto itFind(std::find(gOrders.begin(),gOrders.end(),Order(orderName,0)));
    if(itFind==gOrders.end())
    {
        pthread_mutex_lock(&mtxStream);
        std::cout<<"There is no such item in order queue\n";
        pthread_mutex_unlock(&mtxStream);
    }
    else
    {
        for(auto& i: gStorage)
            if(i.first.GetId()==orderName)
                ++i.second;
        gOrders.erase(itFind);
    }

    
    pthread_mutex_unlock(&mtxCommon);

    return nullptr;
}

void* ShowOrders(void*)
{
    pthread_mutex_lock(&mtxCommon);

    std::deque<Order> OrdersCopy(gOrders);
    int totalPrice(0); 

    pthread_mutex_lock(&mtxStream);
    usleep(100000);
    std::cout<<"\n   Your shopping cart \n";
    for(;!OrdersCopy.empty();)
    {
        Order el(OrdersCopy.front());
        totalPrice+=el.GetPrice();
        std::cout<<el.GetId()<<"\t\t"<<el.GetPrice()<<"$"<<'\n';
        OrdersCopy.pop_front();
    }
    std::cout<<"__________________________\n"<<"Total: "<<totalPrice<<'\n';
    pthread_mutex_unlock(&mtxStream);
    pthread_mutex_unlock(&mtxCommon);

    pthread_mutex_lock(&mtxFinish);
    pthread_cond_signal(&condForCheque);
    pthread_mutex_unlock(&mtxFinish);

    return nullptr;
}

void* FinishShopping(void*)
{
    pthread_mutex_lock(&mtxStream);

    pthread_t showOrdersT;
    pthread_create(&showOrdersT,nullptr,ShowOrders,nullptr);
    usleep(100000);
    pthread_mutex_unlock(&mtxStream);

    pthread_mutex_lock(&mtxFinish);
    pthread_cond_wait(&condForCheque,&mtxFinish);


    pthread_mutex_lock(&mtxCommon);

    while(!gOrders.empty()) 
        gOrders.pop_front();
    pthread_mutex_unlock(&mtxCommon);


    std::cout<<"\nThanks, have a nice day!\n";

    pthread_mutex_unlock(&mtxStream);
    return nullptr;
}

int main() 
{
    int choice(0);

    pthread_mutex_init(&mtxStream,nullptr);
    pthread_mutex_init(&mtxCommon,nullptr);
    pthread_mutex_init(&mtxFinish,nullptr);
    pthread_cond_init(&condForCheque,nullptr);

    gStorage.push_back(std::make_pair<Order,double>(Order("apple", 1),10));
    gStorage.push_back(std::make_pair<Order,double>(Order("phone", 12),4));
    gStorage.push_back(std::make_pair<Order,double>(Order("plate", 4),23));
    gStorage.push_back(std::make_pair<Order,double>(Order("eggplant", 2),1));
    gStorage.push_back(std::make_pair<Order,double>(Order("mug", 6),0));

    do 
    {
        pthread_mutex_lock(&mtxStream);

        std::cout << "\nMenu:\n";
        std::cout << "1. List goods\n";
        std::cout << "2. Add order\n";
        std::cout << "3. Remove order\n";
        std::cout << "4. Shopping cart\n";
        std::cout << "5. Finish shoping\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) 
        {
            case 1:
            {
                
                pthread_mutex_lock(&mtxCommon);

                std::cout << "\nAll products: \n";
                for(auto& i:gStorage)
                    std::cout<<i.first<<"Amount: "<<i.second<<'\n';
                std::cout<<'\n';

                pthread_mutex_unlock(&mtxCommon);

                break;    
            }
            case 2: 
            {

                std::string orderName("no-info");

                std::cout << "\nEnter item name to add: ";
                std::cin >> orderName;

                pthread_t orderT;
                pthread_create(&orderT,nullptr,CreateOrder,&orderName);
                
                pthread_mutex_unlock(&mtxStream);
            
                break;
            }
            case 3: 
            {
                std::string orderName("no-info");

                std::cout << "\nEnter item name to remove: ";
                std::cin >> orderName;

                pthread_t removeT;
                pthread_create(&removeT,nullptr,RemoveOrder,&orderName);


                break;
            }
            case 4:
            {
                pthread_t showOrdersT;
                pthread_create(&showOrdersT,nullptr,ShowOrders,nullptr);

                break;
            }
            case 5:
            {
                pthread_t finishShoppingT;
                pthread_create(&finishShoppingT,nullptr,FinishShopping,nullptr);

                break;
            }
            case 0:
            {
                std::cout << "Exiting the program\n"; 
                break;
            }
            default:
                std::cout << "Invalid choice. Please try again\n";

        }
        pthread_mutex_unlock(&mtxStream);
        usleep(500000);
        
    }
    while (choice!=0);



    pthread_mutex_destroy(&mtxStream);
    pthread_mutex_destroy(&mtxCommon);
    pthread_mutex_destroy(&mtxFinish);
    pthread_cond_destroy(&condForCheque);

}

