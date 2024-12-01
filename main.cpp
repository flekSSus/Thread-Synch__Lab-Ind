#include<iostream>
#include<pthread.h>
#include<queue>
#include<vector>
#include<unistd.h>

struct Order
{
    std::string id_;
    double price_;

    Order()=delete;
    Order(std::string id, double price): id_(id),price_(price){}

    std::string GetId(){ return id_; }

    friend std::ostream& operator<<(std::ostream& os,const Order& obj)
    {
        os<<"\nName: "<<obj.id_<<"  Price: "<<obj.price_<<'\n';
        return os;
    }
};



std::queue<Order> gOrders;
std::vector<std::pair<Order,double>> gStorage;
std::vector<pthread_t*> gThreads;

pthread_mutex_t mtxStream;
pthread_mutex_t mtxCommon;


void* CreateOrder(void* pOrderName)
{

    std::string orderName(*static_cast<std::string*>(pOrderName));
    std::cout<<"  "<<orderName<<"   ";

    pthread_mutex_lock(&mtxCommon);
    
    for(auto& i: gStorage)
        if(i.first.GetId()==orderName && i.second>0)
        {
            gOrders.push(i.first);    
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

int main() 
{
    int choice(0);

    pthread_mutex_init(&mtxStream,nullptr);
    pthread_mutex_init(&mtxCommon,nullptr);

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
                gThreads.push_back(&orderT);
            
                break;
            }
            case 3: 
            {
                break;
            }
            case 0:
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;

        }
        pthread_mutex_unlock(&mtxStream);
        usleep(500000);
        
    } 
    while (choice>0 && choice<4);

    pthread_mutex_lock(&mtxStream);
    std::cout << "Exiting the program." << std::endl;
    pthread_mutex_unlock(&mtxStream);


    pthread_mutex_destroy(&mtxStream);
    pthread_mutex_destroy(&mtxCommon);

}

