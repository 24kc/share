#include <iostream>
#include "Mean.h"
using namespace std;

int main()
{

	akm::Mean<3, 5, 7, 15>  mean;
//	Mean<0, 1, 2>  m1; // 编译失败, 周期必须大于0
//	Mean<3, -1, 5, 7>  m2; // 编译失败, 周期必须大于0

	// 执行update(int) NUM次
	constexpr int NUM = 30;
	for (int i=0; i<NUM; ++i)
		mean.update(i);

	// 输出结果
	for (int i=0; i<4; ++i) {
		cout<<"周期为"<<mean.T[i]<<"的均值: ";
		for (auto x : mean.get_means(i))
			cout<<x<<' ';
		cout<<endl;
	}
}
