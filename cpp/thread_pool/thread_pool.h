#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include "invoker.h"

namespace akm {

struct state {
        virtual ~state() = default;
        virtual void run() = 0;
};
using state_ptr = std::unique_ptr<state>;

template<class Callable>
struct state_impl : public state
{
        state_impl(Callable&& f) : func(std::forward<Callable>(f)) { }
        void run() { func(); }

        Callable func;
};

template<class Callable>
static state_ptr
make_state(Callable&& f)
{
	using impl = state_impl<Callable>;
	return state_ptr{ new impl{std::forward<Callable>(f)} };
}

template<size_t N>
class thread_pool;

template<size_t N>
class thread_pool_base {
  private:
	thread_pool_base();
	thread_pool_base(const thread_pool_base&) = delete;
	thread_pool_base& operator= (const thread_pool_base&) = delete;
	~thread_pool_base() = default;

	template<class F, class... Args>
	void thread(F&& f, Args&&... args);
	// 向线程池提交任务

	void join();
	// 等待所有已提交任务完成

  private:
	void thread_loop();
	void stop();

  private:
	friend thread_pool<N>;

	std::thread threads[N];

	std::mutex mutex;
	std::condition_variable condition, cv_join;
	std::queue<state_ptr> tasks;

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
		threads[i] = std::thread(&thread_pool_base<N>::thread_loop, this);
}

template<size_t N>
void
thread_pool_base<N>::stop()
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
	static_assert(std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>,
		"akm::thread_pool_base<N> arguments must be invocable after conversion to rvalues"
	);
	{
		std::lock_guard<std::mutex> lock(mutex);
		tasks.push(
			make_state(
				make_invoker(
					std::forward<F>(f),
					std::forward<Args>(args)...
				)
			)
		);
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
		state_ptr task;
		{
			std::unique_lock<std::mutex> lock(mutex);
			condition.wait(lock, [this]{ return (flags & STOP) || ! tasks.empty(); });
			if ( (flags & STOP) && tasks.empty() ) {
				if ( flags & JOIN ) {
					if ( ++nfree == N ) {
						flags &= ~(STOP|JOIN);
						nfree = 1;
						lock.unlock();
						if ( nfree == N )
							cv_join.notify_one();
						else
							condition.notify_all();
					} else {
						condition.wait(lock);
						if ( ++nfree == N ) {
							lock.unlock();
							cv_join.notify_one();
						}
					}
					task = NULL;
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
		if ( task )
			task->run();
	}
}

template<size_t N>
class thread_pool {
  public:
	thread_pool(): pool_base(new thread_pool_base<N>()) {};
	thread_pool(const thread_pool&) = delete;
	thread_pool& operator= (const thread_pool&) = delete;
	~thread_pool() { pool_base->stop(); };

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
