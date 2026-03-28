#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
namespace ynet {
    class ThreadPool {
        private:
            std::vector<std::thread> workers;
            std::queue<std::function<void()>> tasks;
            std::mutex mtx;
            std::condition_variable cv;
            bool stop;
        public:
            ThreadPool(int num_threads);
            ~ThreadPool();
            void submit(std::function<void()> task);
    };
}
