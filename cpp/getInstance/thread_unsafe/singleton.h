#ifndef _SINGLETON_H_
#define _SINGLETON_H_

namespace akm {
using namespace std;

class T {
  public:
	static T& getInstance();
	friend ostream& operator<< (ostream&, const T&);

  private:
	string data;
	static T* instance;
	static T& (*getInstancePrivate)();

	T();
	T(const T&) = delete;
	T& operator= (const T&) = delete;
	~T() = default;

	static T& f1();
	static T& f2();
};

T* T::instance;
T& (*T::getInstancePrivate)() = T::f1;

T::T()
{
	cout<< "T()" <<endl;
	data = "ABC傻逼";
}

T&
T::getInstance()
{
	return getInstancePrivate();
}

T&
T::f1()
{
	instance = new T();
	getInstancePrivate = f2;
	return getInstancePrivate();
}

T&
T::f2()
{
	return *instance;
}

ostream&
operator<< (ostream& out, const T& t)
{
	out<<t.data;
	return out;
}

} // namespace akm;

#endif

