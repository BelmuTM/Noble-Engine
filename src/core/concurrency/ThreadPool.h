#pragma once
#ifndef NOBLEENGINE_THREADPOOL_H
#define NOBLEENGINE_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <thread>

class ThreadPool {
public:
    explicit ThreadPool(const size_t threadCount) {
        running.store(true);

        for (size_t i = 0; i < threadCount; i++) {

            workerThreads.emplace_back([this] {
                while (running || !tasksQueue.empty()) {
                    std::function<void()> task;

                    {
                        // Acquire the mutex to safely access the shared tasks queue
                        std::unique_lock lock(mutex);
                        // Wait until there is at least one task in the queue or the pool is destroyed to execute tasks
                        condition.wait(lock, [this] { return !tasksQueue.empty() || !running; });

                        if (tasksQueue.empty()) continue;

                        // Pop the task off the queue
                        task = std::move(tasksQueue.front());
                        tasksQueue.pop();
                    }

                    // Execute the task
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        running.store(false);

        condition.notify_all();

        for (auto& thread : workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    template<typename Function, typename... Args>
    auto enqueue(Function&& func, Args&&... args) {
        using ReturnType = std::invoke_result_t<Function, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            [f = std::forward<Function>(func), ... a = std::forward<Args>(args)]() mutable {
                return f(a...);
            }
        );

        std::future<ReturnType> taskResult = task->get_future();

        {
            std::unique_lock lock(mutex);
            tasksQueue.emplace([task] { (*task)(); });
        }

        condition.notify_one();

        return taskResult;
    }

private:
    std::vector<std::thread>          workerThreads;
    std::queue<std::function<void()>> tasksQueue;
    std::mutex                        mutex;
    std::condition_variable           condition;

    std::atomic<bool> running{false};
};

#endif // NOBLEENGINE_THREADPOOL_H
