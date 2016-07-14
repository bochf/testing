#include <repunit.h>
#include <iostream>
#include <vector>
#include <cstdint>

using namespace std;

int main(int argc, char * argv[])
{
  /// input
  uint16_t T;
  vector<uint64_t> n;

  cin >> T;
  while (T-- != 0) {
    uint64_t tmp;
    cin >> tmp;
    n.push_back(tmp);
  }

  /// process
  for (size_t i = 0; i < n.size(); ++i) {
    cout << leastK(n[i]) << endl;
  }

  return 0;
}
