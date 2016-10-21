
# Arrays: Left Rotation
A left rotation operation on an array of size  shifts each of the array's elements **1** unit to the left. For example, if **2** left rotations are performed on array [**1, 2, 3, 4, 5**], then the array would become [**3, 4, 5, 1, 2**].

Given an array of _n_ integers and a number, _d_, perform _d_ left rotations on the array. Then print the updated array as a single line of space-separated integers.

### Input Format
The first line contains two space-separated integers denoting the respective values of _n_ (the number of integers) and _d_ (the number of left rotations you must perform). 
The second line contains _n_ space-separated integers describing the respective elements of the array's initial state.

### Constraints
* 1 <= n <= 10^5
* 1 <= d <= n
* 1 <= a(i) <= 10^6
## Output Format

Print a single line of _n_ space-separated integers denoting the final state of the array after performing _d_ left rotations.

### Sample Input
5 4  
1 2 3 4 5
### Sample Output
5 1 2 3 4
## Explanation
When we perform _d = 4_ left rotations, the array undergoes the following sequence of changes:  
[1, 2, 3, 4, 5] -> [2, 3, 4, 5, 1] -> [3, 4, 5, 1, 2] -> [4, 5, 1, 2, 3] -> [5, 1, 2, 3, 4]  
Thus, we print the array's final state as a single line of space-separated values, which is **5 1 2 3 4**.