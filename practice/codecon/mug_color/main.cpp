#include <iostream>
#include <string>

#include "mug_color.h"

using namespace std;

int main(int argv, char * argc[]) {
	int n;
	cin >> n;
	
	Mug mug;
	
	string color;
	for (int i = 0; i < n; ++i) {
		cin >> color;
		mug.setColor(color);
	}
	
	cout << mug.getColor() << endl;
	
	return 0;
}
