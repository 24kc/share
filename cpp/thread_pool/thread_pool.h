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
class basic_thread_pool {
  private:
	basic_thread_pool();
	basic_thread_pool(const basic_thread_pool&) = delete;
	basic_thread_pool& operator= (const basic_thread_pool&) = delete;
	~basic_thread_pool() = default;

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
}; // class basic_thread_pool

template<size_t N>
basic_thread_pool<N>::basic_thread_pool()
{
	flags = 0;
	nexit = 0;
	for (size_t i=0; i<N; ++i)
		threads[i] = std::thread(&basic_thread_pool<N>::thread_loop, this);
}

template<size_t N>
void
basic_thread_pool<N>::stop()
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
basic_thread_pool<N>::thread(F&& f, Args&&... args)
{
	static_assert(std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>,
		"akm::basic_thread_pool<N> arguments must be invocable after conversion to rvalues"
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
basic_thread_pool<N>::join()
{
	std::unique_lock<std::mutex> lock(mutex);

	nfree = 0;
	flags |= STOP|JOIN;
	condition.notify_all();

	cv_join.wait(lock);
}

template<size_t N>
void
basic_thread_pool<N>::thread_loop()
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
	thread_pool();
	thread_pool(nullptr_t); // 构建空线程池
	thread_pool(thread_pool&& other);
	thread_pool(const thread_pool&) = delete;
	thread_pool& operator= (thread_pool&& other);
	~thread_pool();

	template<class F, class... Args>
	void thread(F&& f, Args&&... args)
	{
		basic_pool->thread(std::forward<F>(f), std::forward<Args>(args)...);
	}
	// 向线程池提交任务

	void join()
	{
		basic_pool->join();
	}
	// 等待所有已提交任务完成

	void swap(thread_pool& other);
	// 交换2个线程池

  private:
	basic_thread_pool<N> *basic_pool;
}; // class thread_pool

template<size_t N>
thread_pool<N>::thread_pool(): basic_pool(new basic_thread_pool<N>()) { }

template<size_t N>
thread_pool<N>::thread_pool(nullptr_t null): basic_pool(nullptr) { }

template<size_t N>
thread_pool<N>::thread_pool(thread_pool&& other): thread_pool<N>(nullptr)
{
	swap(other);
}

template<size_t N>
thread_pool<N>::~thread_pool()
{
	if ( basic_pool )
		basic_pool->stop();
}

template<size_t N>
thread_pool<N>&
thread_pool<N>::operator=(thread_pool&& other)
{
	swap(other);
	return *this;
}

template<size_t N>
void
thread_pool<N>::swap(thread_pool& other)
{
	std::swap(basic_pool, other.basic_pool);
}

} // namespace akm

#endif // _THREAD_POOL_H_
