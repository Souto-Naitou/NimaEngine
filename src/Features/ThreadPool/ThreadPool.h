#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

/// <summary>
/// スレッドプールクラス
/// </summary>
class ThreadPool
{
public:
    ThreadPool(ThreadPool const&) = delete; // Disable copy constructor
    ThreadPool& operator=(ThreadPool const&) = delete; // Disable copy assignment
    ThreadPool(ThreadPool&&) = delete; // Disable move constructor
    ThreadPool& operator=(ThreadPool&&) = delete; // Disable move assignment

    static ThreadPool* GetInstance()
    {
        static ThreadPool instance(8);
        return &instance;
    }

    ThreadPool(size_t num_threads)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers.emplace_back([this]
            {
                for (;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this]
                        {
                            return this->stop || !this->tasks.empty();
                        });

                        if (this->stop && this->tasks.empty()) return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    ~ThreadPool();

    void enqueue(std::function<void()> task);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};