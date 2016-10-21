#include <heap_sort.h>
#include <vector>

using namespace std;

namespace
{
// swap two integers
void swap(int& a, int& b)
{
  a ^= b;
  b ^= a;
  a ^= b;
}

// check a node has child or not
size_t leftChild(const size_t end, const size_t pos)
{
  size_t child = pos * 2 + 1;
  return (child <= end) ? child : 0;
}

}

void HeapSort::sort(vector<int>& v)
{
  build(v);

  // after build the heap, the root element is the max number
  // continuously move the first element to unsorted bottom and heapify the list
  for (size_t i = v.size() - 1; i != 0; --i) {
    swap(v[0], v[i]);
    maxHeapify(v, 0, i - 1);
  }
}

void HeapSort::build(vector<int>& v)
{
  if (v.empty()) {
    return;
  }

  size_t end = v.size() - 1;
  for (int i = (end-1)/2; i >= 0; --i) { // start from last non-leaf node
    maxHeapify(v, i, end);
  }
}

void HeapSort::maxHeapify(vector<int>& v, const size_t start, const size_t end)
{
  size_t pos = start;
  while (size_t maxChild = leftChild(end, pos)) {
    size_t right = maxChild + 1;
    if (right <= end && v[maxChild] < v[right]) {
      maxChild = right;
    }

    if (v[pos] < v[maxChild]) {
      swap(v[pos], v[maxChild]);
      pos = maxChild;
    }
    else {
      return;
    }
  }
}

bool HeapSort::verify(const vector<int>& v, const size_t pos)
{
  if (v.empty()) {
    return true;
  }

  size_t i = pos;
  size_t end = v.size() - 1;

  while (i < v.size()) {
    size_t child = pos * 2 + 1;
    if (child < end) {
      if (v[i] < v[child] || v[i] < v[child + 1]) {
        return false;
      }
    }
    else if (child == end) {
      if (v[i] < v[child]) {
        return false;
      }
    }
    else {
      return true;
    }
  }
}
