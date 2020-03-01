#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#include <iostream>

#define MP_MIN_BLOCK  (32)

#define MP_THROW  (0x10)

namespace akm {
using std::ostream;

class mempool {
  public:
	static mempool*
	create(void *mem, int size, int flag);

	mempool() = delete;
	mempool(const mempool&) = delete;
	mempool& operator= (const mempool&) = delete;
	~mempool() = delete;

	void* alloc(int size);
	void* realloc(void *mem, int size);
	void free(void *mem);

	int capacity() const;
	int max_block_size() const;

	void check() const;
	friend ostream& operator<< (ostream &out, const mempool *mp);
	friend ostream& operator<< (ostream &out, const mempool &mp);

  private:
	class node_t;

	node_t *list;
	int list_num;
	int flag;
	void *begin, *end;

	void* alloc_nothrow(int size) noexcept;

	static node_t* mb_init(void*, int);
	static node_t* mb_get_buddy(const mempool*, node_t*);
	static void check_list(node_t*);
};

} // namespace akm

#endif // _MEMPOOL_H_
