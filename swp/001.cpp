#include <iostream>
using namespace std;

class T {
	char *data;

  public:
	T() { data = new char[100]; }
	~T() { delete[] data; }
};

int main()
{
	T *t = new T();
	t->~T();
	delete t;

}

