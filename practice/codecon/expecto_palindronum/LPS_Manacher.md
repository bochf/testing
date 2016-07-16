# Longest Palindromic Substring - Manacher Algorithm

A **palindrome** is a word, phase, number, or other sequence of characters which reads same backward or forward. The **longest palindromic substring** problem is a problem of finding the maximum-length contiguous substring of a given string that is also a palindrome. For example the longest palindromic substring of "banana" is "anana". The longest palindromic substring is not guaranteed to be unique; for example, in the string "abracadabra", there is no palindromic substring with length greater than three, but there are two palindromic substring with length three, namly, "aca", and "ada".

[Manacher(1975)](https://en.wikipedia.org/wiki/Longest_palindromic_substring#CITEREFManacher1975) found a linear algorithm for listing all the palindromes that appear at the start of a given string. However, the same algorithm can also be used to find all maximum palindromic substrings anywhere within the input string, again in the linear time. Therefore it provides a linear time solution to the longest palindromic substring problem.

## Manacher's Algorithm
To find in linear time a longest palindrome in a string, an algorithm may take advantage of the following characteristics or observations about a palindrome and a sub-palindrome:
+ The left side of a palindrome is a mirror image of its right side.
+ Assume there are 3 palindromes in the string, namly left palindrome (**LP**), center palindrome (**CP**), and right palindrome (**RP**) based on their center position.
  + (Case 1) **RP**'s center is within the right side of **CP** will have exactly the same length as that of **LP** anchored at the mirror center on the left side, if **LP** is within the bounds of **CP** by at least one character. And, **LP** and **RP** are identical. For example: 123abcbxyzyxbcba456
  + (Case 2) If **LP** meets or extends beyond the left bound of **CP**, then the **RP**'s length is guaranteed to have at least the length from its own center to the right outermost character of **CP**. This length is the same from the center of **LP** to the left outermost character of **CP**. For example: 123abcbaxyzyxabcbaxy456
  + To find the length of **RP** under case 2, the next character after the right outermost of **CP** would then to be compared with its mirror character about the center of **RP**, until there is no match or no more characters to compare.
  + (Case 3) Neither **LP** nor **CP** provides information to help determine the palindromic length of **RP**, if **RP**'s center is outside the right side of **CP**.
+ It is therefore desirable to have a palindrome as a reference (i.e. the **CP**) that processes characters furtherest to the right in a string when determine from left to right the palindromic length of a substring in the string.
+ For even-length palindromes, the center is at the boundary of two characters in the middle.

## Implementation
+ There are 2 cases of palindrome, odd-length and even-length. To simplify the algorithm, we will transform the given string S to T by adding a special charactor (e.g. '#') at each end and in between letters, so T is always odd-length, and we can consistently deal with all the palindromic substring as odd-length palindrome.
```
Example:
S = 12abcba34 (9 characters)
T = #1#2#a#b#c#b#a#3#4# (19 characters)
S = 12abba34 (8 characters)
T = #1#2#a#b#b#a#3#4# (17 characters)
```
``` c++
Code:
void transform(std::string& s) {
  if (s.empty())
    return;
    
	size_t len = s.size();
	s.append(1, '#');
	for (size_t i = 0; i < len; ++i) {
		// move the first character to the last and append a '#'
		s.append(1, s[0]);
		s.append(1, '#');
		s.erase(s.begin());
	}
}
```
+ For each character in T, calculate the radius of palindromic span, from the center to either outermost side and save the radius in array **P**. A single character palindrome radius is 0, 3 character palindrome radius is 1.
```
Example:
T = # a # b # a # a # b # a #
P = 0 1 0 3 0 1 6 1 0 3 0 1 0
Now we immediatly see longest palindrome length is 6.

```
``` c++
Code:
size_t radius(const string& s, const size_t i) {
	size_t step = 1;           // temp variable to record radius of a palindrome
	size_t begin = 0;          // the left boundary of the string
	size_t end = s.size() - 1; // the right boundary of the string
	
	while ((begin + step <= i) && (i + step <= end)) {
		if (s[i-step] == s[i+step]) {	
			step++; // test outter pair characters if match
		}
		else {
			break;  // stop comparison
		}
	}
	
	return step-1;
}

size_t lps(const string& s) {
  vector<size_t> p;
  size_t max = 0;
  size_t index_max = 0;
  
  for (size_t i = 0; i < s.size(); ++i) {
    size_t r = radius(s, i);
    p.push_back(r);
    if (max < r) {
      max = r;
      index_max = i;
    }
  }
  
  return max;
}
```
There are 2 layers of loop in the code, the outer loop goes through each element in the string to find palindrome, the inner loop compares left and right elements of the center until mismatch or reaches boundary of the string.
As we discussed above, there might be some information we can leverage when calculate the **RP** length.
Consider case 1: 123abcbbxyzyxbcba456  
| index |0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|24|25|26|27|28|29|30|31|32|33|34|35|36|37|38|
| T     |#|1|#|2|#|3|#|a|#|b|# |c |# |b |# |x |# |y |# |z |# |y |# |x |# |b |# |c |# |b |# |a |# |4 |# |5 |# |6 |# |
| P     |0|1|0|1|0|1|0|1|0|1|0 |2 |0 |1 |0 |1 |0 |1 |0 |13|0 |1 |0 |1 |0 |1 |0 |2 |0 |1 |0 |1 |0 |1 |0 |1 |0 |1 |0 |
