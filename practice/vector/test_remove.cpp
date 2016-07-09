/**
 * verify how remove will impact the element position in vector
 */

#include <vector>
#include <iostream>

using namespace std;

void print(const vector<int>& v) {
	vector<int>::const_iterator i = v.begin();
	while (i != v.end()) {
		cout << *i << " ";
		++i;
	}
	cout << endl;
}

int main() {
	int a[] = {1, 2, 3, 3, 4, 5};
	vector<int> v(a, a + sizeof(a)/sizeof(int));
	print(v);

	for (size_t i = 0; i < v.size(); ++i) {
		if (v[i] == 3) {
			cout << "found v[" << i << "]==" << v[i] << endl;
			v.erase(v.begin() + i);
		}
	}

	print(v);

	return 0;
}
