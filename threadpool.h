#pragma once
#include <condition_variable>
#include <functional>
#include <iostream>
#include <future>
#include <vector>
#include <thread>
#include <queue>
using namespace std;

class threadpool {
	public:

        using job = std::function<void()>;
        
        explicit threadpool(std::size_t num_threads) {
            start(num_threads);
        }

        threadpool(){
            num_threads = 0;
        }

        void set_num_threads(size_t threads) {
            num_threads = threads;
            start(num_threads);
        }

        template<class T>
        auto enqueue(T task)->std::future<decltype(task())> {
            auto wrapper = std::make_shared<std::packaged_task<decltype(task()) ()>>(std::move(task));
    
            {
                std::unique_lock<std::mutex> lock{job_lock};
                job_queue.emplace([=] {
                    (*wrapper)();
                });
            }
    
            notification.notify_one();
            return wrapper->get_future();
        }

        ~threadpool() {
            stop();
        }
    

        std::size_t num_threads;
    
    private:
        std::vector<std::thread> mThreads;
    
        std::condition_variable notification;
    
        std::mutex job_lock;
        bool stop_thread = false;
    
        std::queue<job> job_queue;
    
        void start(std::size_t numThreads) {

            for (unsigned int i = 0; i < numThreads; ++i)
            {
                mThreads.emplace_back([=] {
                    while (true) {
                        job current_job;
    
                        {
                            std::unique_lock<std::mutex> lock{job_lock};
                            notification.wait(lock, [=] { return stop_thread || !job_queue.empty(); });
                            if (stop_thread && job_queue.empty())
                                break;
    
                            //moving the current func to the front and exec it
                            //later
                            current_job = std::move(job_queue.front());
                            job_queue.pop();
                        }
    
                        current_job();
                    }
                });
            }
        }
    
        //to join all the threads back to together
        void stop() noexcept {
            {
                std::unique_lock<std::mutex> lock{job_lock};
                stop_thread = true;
            }
    
            notification.notify_all();
    
            for (auto &thread : mThreads)
                thread.join();
        }
};
