#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace akm {

template<size_t N>
class thread_pool;

template<size_t N>
class thread_pool_base {
  private:
	thread_pool_base();
	thread_pool_base(const thread_pool_base&) = delete;
	thread_pool_base& operator= (const thread_pool_base&) = delete;
	~thread_pool_base();

	template<class F, class... Args>
	void thread(F&& f, Args&&... args);
	// 向线程池提交任务

	void join();
	// 等待所有已提交任务完成

  private:
	void thread_loop();

  private:
	friend thread_pool<N>;

	std::thread threads[N];

	std::mutex mutex;
	std::condition_variable condition, cv_join;
	std::queue<std::function<void()>> tasks;

	int flags;
	int nfree;
	int nexit;

	static constexpr int STOP = 0x1;
	static constexpr int JOIN = 0x2;
}; // class thread_pool_base

template<size_t N>
thread_pool_base<N>::thread_pool_base()
{
	flags = 0;
	nexit = 0;
	for (size_t i=0; i<N; ++i)
		threads[i] = std::move(std::thread(&thread_pool_base<N>::thread_loop, this));
}

template<size_t N>
thread_pool_base<N>::~thread_pool_base()
{
	std::lock_guard<std::mutex> lock(mutex);
	for (size_t i=0; i<N; ++i)
		threads[i].detach();
	flags &= ~JOIN;
	flags |= STOP;
	condition.notify_all();
}

template<size_t N>
template<class F, class... Args>
void
thread_pool_base<N>::thread(F&& f, Args&&... args)
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
	}
	condition.notify_one();
}

template<size_t N>
void
thread_pool_base<N>::join()
{
	std::unique_lock<std::mutex> lock(mutex);

	nfree = 0;
	flags |= STOP|JOIN;
	condition.notify_all();

	cv_join.wait(lock);
}

template<size_t N>
void
thread_pool_base<N>::thread_loop()
{
	for (;;) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(mutex);
			condition.wait(lock, [&, this]{ return (flags & STOP) || ! tasks.empty(); });
			if ( (flags & STOP) && tasks.empty() ) {
				if ( flags & JOIN ) {
					if ( ++nfree == N ) {
						flags &= ~(STOP|JOIN);
						nfree = 1;
						lock.unlock();
						condition.notify_all();
					} else {
						condition.wait(lock);
						if ( ++nfree == N )
							cv_join.notify_one();
					}
					task = std::move([]{});
				} else {
					if ( ++nexit == N )
						delete this;
					return;
				}
			} else {
				task = std::move(tasks.front());
				tasks.pop();
			}
		}
		task();
	}
}

template<size_t N>
class thread_pool {
  public:
	thread_pool(): pool_base(new thread_pool_base<N>()) {};
	thread_pool(const thread_pool&) = delete;
	thread_pool& operator= (const thread_pool&) = delete;
	~thread_pool() = default;

	template<class F, class... Args>
	void thread(F&& f, Args&&... args)
	{
		pool_base->thread(std::forward<F>(f), std::forward<Args>(args)...);
	}
	// 向线程池提交任务

	void join()
	{
		pool_base->join();
	}
	// 等待所有已提交任务完成

  private:
	thread_pool_base<N> *pool_base;
}; // class thread_pool

} // namespace akm

#endif // _THREAD_POOL_H_
