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

	L.sort([](x,y)->{ x.id > y.id });

	L.save(akmdb);
}

