#include <stdio.h>
#include <iostream>

using namespace std;

#define MAX_LOOP 4

void print1010(const size_t n = MAX_LOOP)
{
  for(int i = n; i > 0; --i) {
    for (int j = 1; j >= 0; --j) {
      for (int k = 0; k < i; ++k) {
        cout << j;
      }
    }
  }
  cout << endl;
};

int main(int argc, char * argv[])
{
	print1010(5);
  return 0;
}
