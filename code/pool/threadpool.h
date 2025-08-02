#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <assert.h>

class ThreadPool
{
public:
    explicit ThreadPool(size_t threadCount = 8) : pool_(std::make_shared<Pool>())
    {
        assert(threadCount > 0);
        for (size_t i = 0; i < threadCount; i++)
        {
            std::thread([pool = pool_]
                        {
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while(true) {
                        // 取任务执行 注意上锁
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        } 
                        else if(pool->isClosed) break;
                        // 等待 如果任务来了就notify
                        else pool->cond.wait(locker);
                    } })
                .detach();
        }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool &&) = default;

    ~ThreadPool()
    {
        if (static_cast<bool>(pool_))
        {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            // 唤醒所有线程
            pool_->cond.notify_all();
        }
    }

    template <class F>
    void AddTask(F &&task)
    {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }

private:
// 用一个结构体封装起来 方便调用
    struct Pool
    {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        // 任务队列 函数类型为void()
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
};

#endif //THREADPOOL_H