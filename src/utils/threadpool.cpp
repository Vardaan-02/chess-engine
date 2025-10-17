#include "utils/threadpool.h"

ThreadPool::ThreadPool(size_t numOfThreads): shutdownFlag(false)
{

    auto workerFn = 
    [this]() {
        while(true){
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(qMutex);
                cv.wait(lock, [this]() { return !tasks.empty() || (shutdownFlag.load() == true); });

                if(shutdownFlag.load() == true && tasks.empty()) return; 

                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    };

    for(size_t i = 0; i < numOfThreads; ++i){
        //compiler will see that std::thread has a constructor that accepts lambdas (like workerFn) so it will create the std::thread by itself
        workerThreads.emplace_back(workerFn);        
    }
}

ThreadPool::~ThreadPool(){
    shutdownFlag.store(true);
    cv.notify_all();
    for(auto& v : workerThreads) if(v.joinable()) v.join();
}