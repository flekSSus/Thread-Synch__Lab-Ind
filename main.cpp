#include<iostream>
#include<queue>
#include<map>
#include<pthread.h>
#include<unistd.h>


struct Order
{
    std::string id_;
    double cost_;

    Order()=delete;
    Order(const std::string id,double cost):id_(id),cost_(cost){}

    friend bool operator<(const Order& obj1, const Order& obj2)
    {
        return obj1.id_ < obj2.id_;
    }

};



pthread_mutex_t mutexA;
std::queue<Order> gArrOrders;
std::vector<bool> gArrIsAvailable; 
std::map<Order,size_t> gArrStorage={ {Order("apple",1.3),3},{Order("pants",0.3),1},
                                    {Order("phone",10),1},{Order("keyboard",14),3} };

void* IsAvailable(void* pArrOrders)
{
    pthread_mutex_lock(&mutexA);
    
    std::cout<<"\nStart checking availability...\n...\n";

    std::queue<Order> arrOrdersCopy(gArrOrders);
    gArrIsAvailable.resize(gArrOrders.size(),false);
    for(size_t i(0);i<gArrIsAvailable.size();++i)
    {
        auto it=gArrStorage.find(arrOrdersCopy.front());
        if( it != gArrStorage.end() && it->second>0)
             gArrIsAvailable[i]=true;
        arrOrdersCopy.pop();
    }

    std::cout<<"Checking was completed\n";
    pthread_mutex_unlock(&mutexA);

    return nullptr;
}

int main()
{
    pthread_mutex_init(&mutexA,nullptr);

    std::string inputStr;
    size_t countOfOrders(0);

    std::cin>>countOfOrders;


    pthread_mutex_lock(&mutexA);
    std::cout<<"\nCreating orders...\n...\n";
    for(size_t i(0);i<countOfOrders;++i)
    {
        std::string tmpId;
        double tmpCost;
        std::cin>>tmpId>>tmpCost;
        gArrOrders.push(Order(tmpId,tmpCost));
    }
    std::cout<<"Created "<<countOfOrders<<" orders\n";
    pthread_mutex_unlock(&mutexA);


    pthread_t isAvailable_t; 
    pthread_create(&isAvailable_t,nullptr,&IsAvailable,&gArrOrders);
    pthread_join(isAvailable_t,nullptr);

    
}
