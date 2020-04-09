#include <iostream>
#include <string>
#include <fstream>
#include <ctype.h>
using namespace std;

int main(int argc, char **argv)
{
	string exe = argv[0];

	if ( argc < 3 ) {
		cerr<<exe<<" [in file] [out file]"<<endl;
		return 1;
	}

	string ifname = argv[1];
	string ofname = argv[2];

	ifstream in(ifname);
	ofstream out(ofname);

	if ( ! in.is_open() || ! out.is_open() ) {
		cerr<<exe<<"fail to open files"<<endl;
		return 1;
	}

	bool skip_space;
	string s, line;
	for (;;) {
		getline(in, s);
		if ( in.eof() )
			break;
		if ( s.find("extern") == string::npos )
			continue;
		line = s;
		while ( s.find(';') == string::npos ) {
			getline(in, s);
			line += s;
			if ( in.eof() )
				break;
		}
		s.swap(line);
		line.clear();
		skip_space = true;
		size_t pos;
		if ( (pos = s.find("__attribute__")) != string::npos ) {
			size_t endpos = s.find(';');
			s.erase(pos, endpos - pos);
		}
		int ch;
		for (auto c : s) {
			if ( isspace(c) ) {
				if ( skip_space )
					continue;
				else {
					ch = ' ';
					skip_space = true;
				}
			} else {
				ch = c;
				skip_space = false;
			}
			line.push_back(ch);
		}
		line.push_back('\n');
		pos = line.find(';') - 1;
		while ( isspace(line[pos]) ) {
			line.erase(pos, 1);
			--pos;
		}
		out.write(line.c_str(), line.size());
	}
}

