#include <ynet/net/thread_pool.h>

using namespace ynet;

ThreadPool::ThreadPool(int num_threads) : stop(false) {
    for(int i = 0; i < num_threads; i++) {
        workers.emplace_back([this]() {
            while(1) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() { return stop || !tasks.empty(); });
                if(stop && tasks.empty()) return ;
                auto task = std::move(tasks.front());
                tasks.pop();
                lock.unlock();
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    std::unique_lock<std::mutex> lock(mtx);
    stop = true;
    lock.unlock();
    cv.notify_all();
    for(auto& w : workers) {
        w.join();
    }
}

void ThreadPool::submit(std::function<void()> task) {
    std::unique_lock<std::mutex> lock(mtx);
    tasks.push(std::move(task));
    lock.unlock();
    cv.notify_one();
}

