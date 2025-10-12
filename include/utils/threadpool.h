#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <functional>
#include <atomic>

class ThreadPool{
    private:
        std::vector<std::thread> workerThreads;
        std::queue<std::function<void()>> tasks;
        std::mutex qMutex;
        std::condition_variable cv;
        std::atomic<bool> shutdownFlag;

    public:
        ThreadPool(size_t numOfThreads);
        ~ThreadPool();

        template <typename F, typename... Args> 
        auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F,Args...>::type>
        {
            if(shutdownFlag.load() == true){
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            using return_type = typename std::invoke_result<F,Args...>::type;
            //Creating a shared pointer to a packaged task which takes no argument (since we are binding the arguments via std::bind)
            auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f),std::forward<Args>(args)...));
            
            std::future<return_type> future = task->get_future();
            
            {
                std::unique_lock<std::mutex> lock(qMutex);
                tasks.emplace([task]() { (*task)(); });
                cv.notify_one();
            }
            return future;
        }
};