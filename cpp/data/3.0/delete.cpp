#include <iostream>
#include "static_list.h"
using namespace std;
using namespace akm;

typedef struct {
	int id;
	char name[32];
	int weight;
	char comment[32];
} Data;

const string akmdb("data.akmdb");

int main()
{
	static_list<Data> L;

	L.load(akmdb);

	// delete from data where id>50 and id<100;
	auto it = L.begin();
	while (it != L.end()) {
		auto next_it = it;
		++next_it;
		if ( it->id > 50 && it->id < 100 )
			L.erase(it);
		it = next_it;
	}

	L.save(akmdb);
}

