#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace fbiu {

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();
    
    // Add task to queue
    void enqueue(std::function<void()> task);
    
    // Wait for all tasks to complete
    void wait();
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::condition_variable wait_condition;
    
    std::atomic<bool> stop{false};
    std::atomic<int> active_tasks{0};
    std::atomic<int> queued_tasks{0};
    
    void worker_thread();
};

} // namespace fbiu
