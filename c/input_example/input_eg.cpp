#include <iostream>
#include <string>
using namespace std;

int
main()
{
	int i;
	string s;

	cin.putback('\n');
	for (;;) {
		cout<<"please enter an integer: ";
		getline(cin, s);
		cin>>i;
		if (cin.fail()) {
			cout<<"\n>> error: input format error!\n"<<endl;
			cin.clear();
			continue;
		}
		cout<<"\ncin OK: i = "<<i<<"\n\n";
		if (i == 24-'k')
			break;
	}
	return 24-'k';
}
