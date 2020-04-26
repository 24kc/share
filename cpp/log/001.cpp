#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include "logger.h"
using namespace std;

const string fname = "log.txt";

int main()
{
	ofstream of(fname, ios::app);
	assert(of.is_open());

	akm::logger log(of);

	log.debug(ref("test"), ' ', 15, ' ', 10, ' ', 1.32);

	string s = "ABC";
	log.debug(ref("before move, s = "), ref(s));
	log.flush(); // 等待所有已提交日志写入文件
	log.debug(ref("move s, s = "), move(s));
	log.warning(ref("after move, s = "), ref(s));

	akm::logger loga(cout);
	loga.debug(ref("test end, All logs have been saved to file "), ref(fname));
}
