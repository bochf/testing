#include <iostream>
#include <string>

#include "base_arithmetic.h"

using namespace std;

int main(int argv, char * argc[]) {
	string x, y;
	cin >> x;
	cin >> y;

	cout << convertBaseN(x.c_str()) + convertBaseN(y.c_str()) << endl;

	return 0;
}
