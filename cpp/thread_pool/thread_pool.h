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
	thread_pool_base() = default;
	thread_pool_base(const thread_pool_base&) = delete;
	thread_pool_base& operator= (const thread_pool_base&) = delete;
	~thread_pool_base() = default;

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
class thread_pool {
  public:
	thread_pool();
	thread_pool(const thread_pool&) = delete;
	thread_pool& operator= (const thread_pool&) = delete;
	~thread_pool();

	template<class F, class... Args>
	void thread(F&& f, Args&&... args);
	// 向线程池提交任务

	void join();
	// 等待所有已提交任务完成

  private:
	thread_pool_base<N> *pool;

	static constexpr int STOP = thread_pool_base<N>::STOP;
	static constexpr int JOIN = thread_pool_base<N>::JOIN;
}; // class thread_pool


template<size_t N>
thread_pool<N>::thread_pool()
{
	pool = new thread_pool_base<N>();

	auto& threads = pool->threads;
	auto& flags = pool->flags;
	auto& nexit = pool->nexit;

	flags = 0;
	nexit = 0;
	for (size_t i=0; i<N; ++i)
		threads[i] = std::move(std::thread(&thread_pool_base<N>::thread_loop, pool));
}

template<size_t N>
thread_pool<N>::~thread_pool()
{
	auto& threads = pool->threads;
	auto& mutex = pool->mutex;
	auto& condition = pool->condition;
	auto& flags = pool->flags;

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
thread_pool<N>::thread(F&& f, Args&&... args)
{
	{
		auto& mutex = pool->mutex;
		auto& tasks = pool->tasks;
		std::lock_guard<std::mutex> lock(mutex);
		tasks.emplace(std::bind(f, args...));
	}
	auto& condition = pool->condition;
	condition.notify_one();
}

template<size_t N>
void
thread_pool<N>::join()
{
	auto& mutex = pool->mutex;
	auto& condition = pool->condition;
	auto& cv_join = pool->cv_join;
	auto& flags = pool->flags;
	auto& nfree = pool->nfree;

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

} // namespace akm

#endif // _THREAD_POOL_H_
