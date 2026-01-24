#include "thread_pool.h"

namespace fbiu {

ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back(&ThreadPool::worker_thread, this);
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();
    
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
        queued_tasks++;
    }
    condition.notify_one();
}

void ThreadPool::wait() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    wait_condition.wait(lock, [this] {
        return queued_tasks == 0 && active_tasks == 0;
    });
}

void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] {
                return stop || !tasks.empty();
            });
            
            if (stop && tasks.empty()) {
                return;
            }
            
            if (!tasks.empty()) {
                task = std::move(tasks.front());
                tasks.pop();
                queued_tasks--;
                active_tasks++;
            }
        }
        
        if (task) {
            task();
            active_tasks--;
            wait_condition.notify_all();
        }
    }
}

} // namespace fbiu
