#ifndef SRPC_UTILS_THREAD_POOL_H
#define SRPC_UTILS_THREAD_POOL_H
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <functional>
#include <future>

class ThreadPool {
public:
    using Task = std::function<void()>;
    explicit ThreadPool(int n);
    ~ThreadPool();

    void stop() {
        std::lock_guard<std::mutex> guard(_mu);
        _stop.store(true); // 用什么内存序？
        _cv.notify_all();
    }

    // 添加任务到线程池中的任务队列中去，该任务会被线程拿去并执行
    // param f: callable 对象
    // param args: 参数包 需要传给 callable 对象，因为 Task 使用统一的 void(*f)() 类型，所以需要用 bind 绑定一下
    // return: 返回一个用于访问异步操作结果的对象，可以通过该对象获取任务执行完后的返回值 
    // reference: [std::future](https://en.cppreference.com/w/cpp/thread/future)
    template <typename Function, class... Args>
    std::future<typename std::result_of<Function(Args...)>::type> add_task(const Function& f, Args... args);
private:
    // 线程池容量：能容纳多少线程
    size_t _capacity;

    // 标志线程池是否关闭
    std::atomic<bool> _stop;

    // 保护共享变量
    std::mutex _mu;

    // 条件变量用于同步
    std::condition_variable _cv;

    // 线程对象存储在 vector 中
    std::vector<std::thread> _threads;

    // 任务队列
    std::queue<Task> _task_queue;
};

ThreadPool::ThreadPool(int n)
    : _capacity(0)
    , _stop(true)
    , _mu()
    , _cv()
    , _threads()
{
    _capacity = n <= 0 ? 1 : n;
    for (int i = 0;i < _capacity; ++i) {
        _threads.emplace_back([&](){
                // 等待线程池启动
                {
                    std::unique_lock<std::mutex> lock(_mu);
                    if (_stop.load())
                        _cv.wait(lock, [&]{ return !_stop.load(); });
                }
                printf("thread %ld started\n", ::syscall(SYS_gettid));

                while (!_stop.load()) { // 用什么内存序？
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(_mu);
                        _cv.wait(lock, [&]() { return _stop.load() or !_task_queue.empty(); });
                        if (_stop.load())  break;
                        task = _task_queue.front(); _task_queue.pop();
                    }
                    task();
                }

                printf("thread %ld finished\n", ::syscall(SYS_gettid));
            });
    }

    {
        std::unique_lock<std::mutex> lock(_mu);
        _stop.store(false);
        _cv.notify_all();
    }
}

ThreadPool::~ThreadPool() {
    assert(_stop.load());
    for (int i = 0; i < _threads.size(); ++i) {
        if (_threads[i].joinable())
            _threads[i].join();
    }
}

template <typename Function, class... Args>
std::future<typename std::result_of<Function(Args...)>::type> 
ThreadPool::add_task(const Function& f, Args... args) {
    std::packaged_task<void()> task(std::bind(f, args...));
    std::future ret = task.get_future();
    {
        std::unique_lock<std::mutex> lock(_mu);
        _task_queue.push(std::move(task));
        _cv.notify_all();
    }
    return ret;
}

#endif
