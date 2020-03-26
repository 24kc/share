#define _MY_NEW_CPP_
#include "my_new.h"

#include <mutex>

//#define MY_MPSIZE  ( 24-'k' +999999 )

namespace {
/*
class my_new_init {
  public:
	my_new_init() {
		static char mem[MY_MPSIZE];
		akm::mp = mp_init(mem, sizeof(mem), MP_THROW);
	}
} my_new_init;
*/
std::mutex mp_mutex;

} // namespace <anonymous>

void*
operator new (size_t size)
{
	std::lock_guard<std::mutex> guard(mp_mutex);
	return mp_alloc(akm::mp, size);
}

void*
operator new[] (size_t size)
{
	return operator new(size);
}

void
operator delete (void *mem) noexcept
{
	std::lock_guard<std::mutex> guard(mp_mutex);
	mp_free(akm::mp, mem);
}

void
operator delete[] (void *mem) noexcept
{
	operator delete(mem);
}

