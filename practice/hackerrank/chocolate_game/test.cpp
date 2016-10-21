
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

// return true if n is odd number
bool isOdd(const int n)
{
  return (n & 1);
}

// wins if the sum of continuous sub-sequence is odd
int numberOfWins(const vector<int>& piles)
{
  if (piles.empty()) {
    return 0;
  }

  int n = piles.size();
  bool odd = false;     // sum of 1st..Kth element is odd or even, K <= n
  int currentOdd = 0;   // number of odd sum
  odd = isOdd(piles[0]);
  for (int i = 1; i < n; ++i) {
    odd ^= isOdd(piles[i]);
    if (odd) {
      currentOdd++;
    }
  }

  int wins = currentOdd;
  // check sums of all continuous sub-sequences, increase wins if sum is odd
  // go through the sub-sequence as
  // [1, 2], [1, 2, 3], [1..4], [1..n];
  // [2, 3], [2..4], [2..n];
  // ...
  // [n-1, n]
  // for each loop(i), the number of odd sum is:
  // if sum(1, i-1) is odd,
  //   if sum(i-1, i) is odd, number of even sum of loop(i-1)
  //   if sum(i-1, i) is even, number of even sum of loop(i-1) - 1
  // if sum(1, i-1) is even,
  //   if sum(i-1, i) is odd, number of odd sum of loop(i-1) - 1
  //   if sum(i-1, i) is even, number of odd sum of loop(i-1)
  for (int i = 2; i < n; ++i) {
    int lastSubsequence = n - (i - 1);
    int lastOdd = currentOdd;
    int lastEven = lastSubsequence - lastOdd;
    int previousFirstSumSubsequence = piles[i - 2] + piles[i - 1];

    if (isOdd(piles[i - 2])) {
      currentOdd = lastEven;
      if (!isOdd(previousFirstSumSubsequence)) {
        currentOdd--;
      }
    }
    else {
      currentOdd = lastOdd;
      if (isOdd(previousFirstSumSubsequence)) {
        currentOdd--;
      }
    }

    wins += currentOdd;

    // temp for debug
    cout << "process " << i
         << "\nnumber of subsequence of previous loop is " << lastSubsequence
         << "\nnumber of odd sum of subsequence in last loop is " << lastOdd
         << "\nnumber of even sum of subsequence in last loop is " << lastEven
         << "\nsum of first subsequence in last loop is " << previousFirstSumSubsequence
         << "\ncurrentOdd=" << currentOdd
         << "\ntotal=" << wins
         << endl;
  }

  return wins;
}
int main(int argc, char * argv[])
{
  /* Enter your code here. Read input from STDIN. Print output to STDOUT */
  int n; // number of chocolate piles
  vector<int> piles;  // number of chocolates in each pile

  if (argc  < 2) {
    cin >> n;
    piles.resize(n);
    for (int i = 0; i < n; ++i) {
      cin >> piles[i];
    }
  }
  else {
    fstream fs(argv[1]);
    fs >> n;
    piles.resize(n);
    for (int i = 0; i < n; ++i) {
      fs >> piles[i];
    }
    fs.close();
  }

  cout << numberOfWins(piles) << endl;

  return 0;
}