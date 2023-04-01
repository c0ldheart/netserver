#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
class ThreadPool {
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mtx;
    std::condition_variable cv;
    bool stop;

public:
    ThreadPool(int size = std::thread::hardware_concurrency()); // 默认size最好设置为std::thread::hardware_concurrency()
    ~ThreadPool();

    // void add(std::function<void()>);
    template <class F, class... Args>
    auto add(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
};

// 定义一个可变参数模板函数，接受一个函数对象和其参数
template <class F, class... Args>
// 函数返回值类型是一个未来的对象，可以异步获取任务的返回值
auto ThreadPool::add(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    // 定义返回值类型
    using return_type = typename std::result_of<F(Args...)>::type;
    // 创建一个packaged_task对象，并用bind绑定函数和其参数
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    // 通过packaged_task对象获取任务的返回值的future对象
    std::future<return_type> res = task->get_future();

    // 加锁，保证线程安全
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);

        // 如果线程池已经停止，则禁止将任务添加到队列中
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        // 将任务添加到队列中
        tasks.emplace([task]() { (*task)(); });
    }

    // 通知一个等待中的线程开始执行任务
    cv.notify_one();

    // 返回任务的future对象，可以异步获取任务的返回值
    return res;
}