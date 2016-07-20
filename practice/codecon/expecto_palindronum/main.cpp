#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>

#include "palindrome.h"

using namespace std;

void print(const string& s, const vector<size_t>& p) {
	if (s.size() != p.size())
		return;
	size_t size = s.size();
	
	for (size_t i = 0; i < size; i++)
		cout << setw(3) << left << i;
	cout << endl;
	
	for (size_t i = 0; i < size; i++)
		cout << setw(3) << left << s[i];
	cout << endl;

	for (size_t i = 0; i < size; i++)
		cout << setw(3) << left << p[i];
	cout << endl;
}

int main(int argc, char* argv[]) {
	switch (argc) {
		case 2: {
			string s(argv[1]);
			Palindrome p(s);
			cout << "longest palindrome substring " << p.lps() << endl;
		}
		break;
		case 3: {
			string s(argv[1]);
			Palindrome p(s);
			cout << "longest palindrome substring from " << 0 << " is " << p.lps0() << endl;
		}
		break;
		default:
		cout << "usage: " << argv[0] << " string" << endl;
		return 0;
	}

	
	
	
	return 0;
}
