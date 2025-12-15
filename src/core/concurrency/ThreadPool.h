#pragma once
#ifndef NOBLEENGINE_THREADPOOL_H
#define NOBLEENGINE_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <random>
#include <thread>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(const size_t threadCount) {
        running.store(true);

        queues.resize(threadCount);
        queueMutexes.resize(threadCount);

        for (auto& queueMutex : queueMutexes) {
            queueMutex = std::make_unique<std::mutex>();
        }

        workerThreads.reserve(threadCount);

        for (size_t i = 0; i < threadCount; i++) {
            workerThreads.emplace_back([this, i] { workerLoop(i); });
        }
    }

    ~ThreadPool() {
        running.store(false, std::memory_order_release);

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

        // Round-robin push to queues to reduce contention
        const size_t queueIndex = nextQueue.fetch_add(1, std::memory_order_relaxed) % queues.size();

        {
            std::lock_guard lock(*queueMutexes[queueIndex]);
            queues[queueIndex].emplace_back([task] { (*task)(); });
        }

        condition.notify_one();

        return taskResult;
    }

private:
    void workerLoop(const size_t queueIndex) {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<size_t> distribution(0, queues.size() - 1);

        while (running.load(std::memory_order_acquire)) {
            std::function<void()> task;

            // Fetch tasks in current thread
            {
                // Acquire the mutex to safely access the thread's tasks queue
                std::lock_guard lock(*queueMutexes[queueIndex]);

                // Pop the most recent task off the queue
                if (!queues[queueIndex].empty()) {
                    task = std::move(queues[queueIndex].back());
                    queues[queueIndex].pop_back();
                }
            }

            // If the current thread has no task to process
            if (!task) {
                // Try to steal a task from other threads
                for (size_t i = 0; i < queues.size(); i++) {
                    // Randomly pick the victim to reduce contention (vs round-robin)
                    const size_t victim = distribution(rng);
                    if (victim == queueIndex) continue;

                    std::lock_guard lock(*queueMutexes[victim]);

                    // Found a valid victim, steal its oldest task
                    if (!queues[victim].empty()) {
                        task = std::move(queues[victim].front());
                        queues[victim].pop_front();
                        break;
                    }
                }
            }

            // Execute the task or wait
            if (task) {
                task();
            } else {
                std::unique_lock lock(waitMutex);
                condition.wait(lock, [this] { return !running || hasPendingTasks(); });
            }
        }
    }

    [[nodiscard]] bool hasPendingTasks() const {
        for (size_t i = 0; i < queues.size(); i++) {
            std::lock_guard lock(*queueMutexes[i]);
            if (!queues[i].empty()) return true;
        }
        return false;
    }

    std::vector<std::thread> workerThreads;

    std::vector<std::deque<std::function<void()>>> queues;
    std::vector<std::unique_ptr<std::mutex>>       queueMutexes;

    std::mutex              waitMutex;
    std::condition_variable condition;

    std::atomic<bool> running{false};

    std::atomic<size_t> nextQueue{0};
};

#endif // NOBLEENGINE_THREADPOOL_H
