# Longest Palindromic Substring - Manacher Algorithm

A **palindrome** is a word, phase, number, or other sequence of characters which reads same backward or forward. The **longest palindromic substring** problem is a problem of finding the maximum-length contiguous substring of a given string that is also a palindrome. For example the longest palindromic substring of "banana" is "anana". The longest palindromic substring is not guaranteed to be unique; for example, in the string "abracadabra", there is no palindromic substring with length greater than three, but there are two palindromic substring with length three, namly, "aca", and "ada".

[Manacher(1975)](https://en.wikipedia.org/wiki/Longest_palindromic_substring#CITEREFManacher1975) found a linear algorithm for listing all the palindromes that appear at the start of a given string. However, the same algorithm can also be used to find all maximum palindromic substrings anywhere within the input string, again in the linear time. Therefore it provides a linear time solution to the longest palindromic substring problem.

## Manacher's Algorithm
To find in linear time a longest palindrome in a string, an algorithm may take advantage of the following characteristics or observations about a palindrome and a sub-palindrome:
1. The left side of a palindrome is a mirror image of its right side.
2. Assume there are 3 palindromes in the string, namly left palindrome, center palindrome, and right palindrome based on their center position.
..1. (Case 1) A third palindrome whose center is within the right side of a first palindrome will have exactly the same length as that of a second palindrome anchored at the mirror center on the left side, if the second palindrome is within the bounds of the first palindrome by at least one character. And, the second and the third palindromes are identical. For example:
```
Given string S = 123abcbxyzyxbcba456
The 1st palindrome P1 = abcbxyzyxbcba
The 2nd palindrome P2 = bcb
The 3rd palindrome P3 = bcb
```
3. (Case 2)