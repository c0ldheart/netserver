#include "ThreadPool.h"

/*这段代码实现了生产者消费者模型的同步操作，
其中生产者是add函数，将任务添加到任务队列中，消费者是线程池中的线程，从任务队列中取出任务并执行。
通过互斥锁和条件变量的配合，实现了对任务队列的同步访问和线程池的同步控制，确保了生产者和消费者之间的同步操作。
当任务队列为空时，线程进入等待状态，等待生产者唤醒，而当线程池被销毁时，线程会退出循环并执行线程的销毁操作。*/

// 构造函数，创建具有指定数量线程的线程池
ThreadPool::ThreadPool(int size)
    : stop(false) // 初始化停止标志为false
{
    for (int i = 0; i < size; ++i) { // 循环创建size个线程
        // emplace_back函数用于在vector的末尾添加元素，并返回指向该元素的迭代器
        threads.emplace_back(std::thread([this]() { // 用lambda函数定义每个线程的工作函数
            while (true) { // 无限循环
                std::function<void()> task; // 定义一个函数对象task
                {
                    // unique_lock和lock_guard类似，但是可以手动控制加锁和解锁
                    // std::unique_lock<std::mutex>构造函数需要一个互斥锁对象
                    // 这里的tasks_mtx是一个std::mutex对象，用于保护任务队列
                    std::unique_lock<std::mutex> lock(tasks_mtx); // 对tasks_mtx加锁
                    // cv.wait()会等待条件变量，直到满足指定条件，唤醒该线程并解锁锁定的互斥锁
                    // 这里的条件是任务队列不为空或线程池停止
                    // 如果条件不满足，则线程将一直等待
                    cv.wait(lock, [this]() { // 等待条件变量
                        return stop || !tasks.empty(); // 条件为任务队列不为空或线程池停止
                    });
                    // 如果线程池已经停止并且任务队列为空，则退出线程
                    if (stop && tasks.empty())
                        return;
                    // 从任务队列头取出一个任务
                    task = tasks.front();
                    tasks.pop(); // 从队列中移除该任务
                }
                task(); // 执行任务
            }
        }));
    }
}

// 析构函数，销毁线程池
ThreadPool::~ThreadPool()
{
    {
        // 同上，在{}作用域内对std::mutex加锁
        std::unique_lock<std::mutex> lock(tasks_mtx);
        stop = true; // 设置停止标志为true
    }
    cv.notify_all(); // 唤醒所有等待条件变量的线程
    // 循环等待所有线程完成
    for (std::thread& th : threads) {
        if (th.joinable())
            th.join(); // 如果线程可以加入，则加入线程
    }
}

// 添加任务到任务队列中
void ThreadPool::add(std::function<void()> func)
{
    {
        // 同上，在{}作用域内对std::mutex加锁
        std::unique_lock<std::mutex> lock(tasks_mtx);
        // 如果线程池已经停止，则抛出异常
        if (stop)
            throw std::runtime_error("ThreadPoll already stop, can't add task any more");
        tasks.emplace(func); // 将函数对象添加到任务队列中
    }
    cv.notify_one(); // 通知一个等待条件变量的线程来执行任务
}