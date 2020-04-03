#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

namespace akm {

template<size_t N>
class thread_pool {
  public:
	thread_pool();
	thread_pool(const thread_pool&) = delete;
	thread_pool& operator= (const thread_pool&) = delete;
	~thread_pool();

	template<class F, class... Args>
	auto push(F&& f, Args&&... args)
		-> std::future<typename std::result_of<F(Args...)>::type>;
	// 向线程池提交任务, 返回future

	template<class F, class... Args>
	void thread(F&& f, Args&&... args);
	// 向线程池提交任务

	void join();
	// 等待所有已提交任务完成

  private:
	void thread_loop();

  private:
	std::thread threads[N];
	std::queue<std::function<void()>> tasks;

	std::mutex mutex;
	std::condition_variable condition;

	int flags;
	int nfree;
	constexpr static int STOP = 0x1;
	constexpr static int JOIN = 0x2;
}; // class thread_pool

template<size_t N>
thread_pool<N>::thread_pool(): flags(0)
{
	for (size_t i=0; i<N; ++i)
		threads[i] = std::thread(&thread_pool<N>::thread_loop, this);
}

template<size_t N>
thread_pool<N>::~thread_pool()
{
	flags &= ~JOIN;
	flags |= STOP;
	condition.notify_all();
	for (size_t i=0; i<N; ++i)
		threads[i].detach();
}

template<size_t N>
template<class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type>
thread_pool<N>::push(F&& f, Args&&... args)
{
	using return_type = typename std::result_of<F(Args...)>::type;
	auto task = std::make_shared<std::packaged_task<return_type()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
	);

	{
		std::lock_guard<std::mutex> lock(mutex);
		tasks.emplace([task]{ (*task)(); });
	}
	condition.notify_one();

	return task->get_future();
}

template<size_t N>
template<class F, class... Args>
void
thread_pool<N>::thread(F&& f, Args&&... args)
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		tasks.emplace(std::bind(f, args...));
	}
	condition.notify_one();
}

template<size_t N>
void
thread_pool<N>::join()
{
	std::unique_lock<std::mutex> lock(mutex);
	flags |= STOP|JOIN;
	nfree = 0;
	condition.notify_all();
	condition.wait(lock, [this]{ return nfree == N; });
}

template<size_t N>
void
thread_pool<N>::thread_loop()
{
	for (;;) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(mutex);
			condition.wait(lock, [this]{ return (flags & STOP) || ! tasks.empty(); });
			if ( (flags & STOP) && tasks.empty() ) {
				if ( flags & JOIN ) {
					if ( ++nfree == N ) {
						flags &= ~(STOP|JOIN);
						condition.notify_all();
					} else
						condition.wait(lock, [this]{ return nfree == N || ! tasks.empty(); });
					task = std::move([]{});
				} else
					return;
			} else {
				task = std::move(tasks.front());
				tasks.pop();
			}
		}
		task();
	}
}

} // namespace akm

#endif // _THREAD_POOL_H_

