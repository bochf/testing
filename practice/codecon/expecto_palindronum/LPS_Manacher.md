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
  + 