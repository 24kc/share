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

int main()
{
	static_list<Data> L;

	L.load("data.akmdb");

	// select * from data;
	for (auto &x : L) {
		cout<<x.id<<' '<<x.name<<' '<<x.weight<<' '<<x.comment<<'\n';
	}

	cout<<"\nQuery OK !"<<endl;
	cout<<L.size()<<" rows in set"<<endl;
}

