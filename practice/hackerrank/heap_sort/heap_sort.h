#ifndef __HEAP_SORT_H_
#define __HEAP_SORT_H_

#include <vector>
#include <cstdlib>

class HeapSort
{
	public:
		/**
		 * @brief sort an array
		 */
		static void sort(std::vector<int>& v);
		
		/**
		 * @brief build max heap
		 * repeatly call maxHeapify from bottom to top to move the greater number up
		 */
		static void build(std::vector<int>& v);
		
/**
 * @brief heapify
 * for any given node, check the sub tree satisfy the max heap rule, e.g.
 * parent node always greater than child nodes
 * 
 * if the current node has child(ren), compare the value of current node and its
 * child(ren), if current node > children, stop heapify.
 * Otherwise, swap the value of current node and the greater child, make sure 
 * max is always on top, then continue heapify the sub tree on the greater child
 * to make sure the sub tree satisfes the rule as well.
 */
		static void maxHeapify(std::vector<int>& v, const size_t start, const size_t end);
		
		/**
		 * @brief verify parent is always greater than children
		 */
		static bool verify(const std::vector<int>& v, const size_t node);

	private:

};

#endif
