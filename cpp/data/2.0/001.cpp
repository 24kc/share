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
#include "data.h"

int main()
{
	int N = sizeof(data) / sizeof(Data);
	static_list<Data> L;

	for (int i=0; i<N; ++i) {
		L.push_back(data[i]);
	}

	L.save("data.akmdb");
}

