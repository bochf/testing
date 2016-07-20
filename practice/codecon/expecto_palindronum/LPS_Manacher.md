# Longest Palindromic Substring - Manacher Algorithm

A **palindrome** is a word, phase, number, or other sequence of characters which reads same backward or forward. The **longest palindromic substring** problem is a problem of finding the maximum-length contiguous substring of a given string that is also a palindrome. For example the longest palindromic substring of "banana" is "anana". The longest palindromic substring is not guaranteed to be unique; for example, in the string "abracadabra", there is no palindromic substring with length greater than three, but there are two palindromic substring with length three, namly, "aca", and "ada".

[Manacher(1975)](https://en.wikipedia.org/wiki/Longest_palindromic_substring#CITEREFManacher1975) found a linear algorithm for listing all the palindromes that appear at the start of a given string. However, the same algorithm can also be used to find all maximum palindromic substrings anywhere within the input string, again in the linear time. Therefore it provides a linear time solution to the longest palindromic substring problem.

## Manacher's Algorithm
To find in linear time a longest palindrome in a string, an algorithm may take advantage of the following characteristics or observations about a palindrome and a sub-palindrome:
+ The left side of a palindrome is a mirror image of its right side.
+ Assume there are 3 palindromes in the string, namly left palindrome (**LP**), center palindrome (**CP**), and right palindrome (**RP**) based on their center position.
  + (Case 1) **RP**'s center is within the right side of **CP** will have exactly the same length as that of **LP** anchored at the mirror center on the left side, if **LP** is within the bounds of **CP** by at least one character. And, **LP** and **RP** are identical. For example: 123abcbxyzyxbcba456
  + (Case 2) If **LP** extends beyond the left bound of **CP**, then the **RP**'s length is same the length from its own center to the right outermost character of **CP**.  This length is the same from the center of **LP** to the left outermost character of **CP**. For example: 123xabcbaxyzyxabcba456
  + (Case 3) If **LP** meets the left bound of **CP**, then the **RP**'s length is guaranteed to have at least the length from its own center to the right outermost character of **CP**. For example: 123abcbaxyzyxabcbaxy456
    + To find the length of **RP** under case 3, the next character after the right outermost of **CP** would then to be compared with its mirror character about the center of **RP**, until there is no match or no more characters to compare.
  + (Case 4) Neither **LP** nor **CP** provides information to help determine the palindromic length of **RP**, if **RP**'s center is outside the right side of **CP**.
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

  for (size_t i = 0; i < s.size(); ++i) {
    size_t r = radius(s, i);
    p.push_back(r);
    if (max < r) {
      max = r;
    }
  }
  
  return max;
}
```
There are 2 layers of loop in the code, the outer loop goes through each element in the string to find palindrome, the inner loop compares left and right elements of the center until mismatch or reaches boundary of the string.
As we discussed above, there might be some information we can leverage when calculate the **RP** length.
Consider this example: **S = babcbabcbaccba**  
![case 1](https://lh3.googleusercontent.com/vkIS68mhoCmTtJ0-ECDFpDMjTbccyBoaq2U9rfKMy9ELaw-gW_QfHYQWFPccovD9nEGEo952knmZ6UGSpyBQjHnac6_6WZJbJpaMQBQiiDtsMyuuQEhsrygi-mkJg2arrPJpICgQBjpVwiUs2J18ASq3EmrItFhSeieiQu_S_Oa-vbTdyAUSC5HvkpLLpg4UO3T1GeGOL4Tmgym4mCPz0kHsX99GqMWfTlXdM8XhaQxlxORrx-KwoW1_G3ROQIGz8iCZcSDhOSIdGLFYLE_CRy-4otNDPz2pUPuVTn2zXnLxbcP_WkqdFbI7fzVC3SQXz6m8RaP8BISyGERbwOWxXp4ywHrNAnke40wbLfPmziq9rZk65V9PG0QJnKh5Q4MACU5cDnvyg2YlovjmkNfPUtHecj-ihb4tPH96PZeVnAJoH6fK_5JpKfv2ybdLPmAlqdpbBqFj2SbEh4iPY-KSSA9oqXRi5MKcH2GB4hy6UMf9bfN1IncqPdpmnP9MtUG7pWt-027ybWHAj18HOis_mIMCV3E3FYJ6b-Ofq8bxKghAd1tYi7vEH3kCtlDM-JKRDhqugaPEQn04BJN7H8_Twd_MGiOH_g=w640-h99-no)
Assume we have processed T[11], which is the longest (so far) palindrome's center, T[11] centric palindrome covers from T[2] to T[20], P[11] = 9. Now we arrived T[13], and we need to calculate P[13] (indicated by the question mark).  
To determine which property we can use, let's first look at the mirror index around P[11], which is P[9] = 1, so T[9] centric palindrome (indicated by green i') is fully covered by T[11]'s palindrome, so we can use property of *case 1* to get P[13]=P[9]=1.
![case 1](https://lh3.googleusercontent.com/ErETPqBumweCmzDmmY1BAAFyR8jkYEwRJ-jrgjJlVA0WLufgRTiOakdyU4bAPYyy4uK-3wrg53RxRXPeeJaN3-6OTFib09pCGD3t7WTm0nKuix2V3ZLnpwEKf5nRZgucA-12wg1zhxgHOTf21os3G3AlUZT9m8eYKTaY1nxPL5zXIkZQvHbd0qIbrRT_dgPqpfaBfJhYJPxFisrH1jz1BgUOpC4vc7kBBrG5PjZeSwhk2R_9viWtobq3bgDVfYuWrep6J6RSI3HFYtJU6KtpAoI8RtIEXUnpqsutqkV4eXeTsHIA64t7w1SWJXUidoqmUkkRcRHOaKButiJ_XGnr98DvhQlxi6Pwq4xW75wsg7LCTMqgCsWH5j1GyWx6lI3FtoaVsOSh8sTymeNjjfpPKJdj8o3OJ6rdoFsHIQYaywdefSxn7Wqhg2BmVtWt-Z5uMKdsY5tVa10Asp3YoFUzWSIQ3ZJEJMARM4UDChqgSqeL1mHkCB6w26dT0pxh8svq4MBKVdHZ1S1yeTg8sEL5IploEABCreqdZYQogN2sQ7gmRjY2B9EeJgoASOk7XR0Y-pkUHlRu3LgqxTlctTBGaqXQn8jyfQ=w640-h99-no)
Similiarly, we can get P[14]=P[8]=0
So the function radius can be improved by referring the exsting P values:
``` c++
size_t radius(const string& s,   // transformed string
              const size_t i,    // index of element to be proceed
              vector<size_t>& p, // saves radius of each character
              const size_t c     // the center of current most right palindrome
              ) {
	if (i < c + p[c]) {
		size_t ii = 2 * c - i;   // the mirrored point of i around c
		if (ii - p[ii] > c - p[c]) { // palindrome ii is within palindrome c
			p.push_back(p[ii]);
			return p[i];
		}
	}
	
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
	
	p.push_back(step-1);
	return p[i];
}
```
*and we need to remember the most right palindrome, function lps is changed to:*  
``` c++
size_t lps(const string& s) {
  vector<size_t> p;
  size_t max = 0;
  size_t c = 0;  // the index of center of the most right palindrome

  // p[0] must be 0
  p.push_back(0);
  for (size_t i = 1; i < s.size(); ++i) {
    size_t r = radius(s, i, p, c);
    if (max < r) {
      max = r;
      if (c+p[c] < i+p[i]) {
      	c = i;
      }
    }
  }
  
  return max;	
}
```

Continue dealing with the string above, we arrived T[15], where the symmetric character is T[7]:
![case 2](https://lh3.googleusercontent.com/Ug4C-eEOD8wDCLBGLAs2pOBziq-jSvvKhwPMTkWnu5slDALUg5UlCQtki1FE4kVKx90d-8T2Vo20aeiR0FJhB5BkOApJ065wOSFH8TeaUwm8nFk5ddrLDr84DN-_rLG2vg1vhhPkiWOiWwtIpt61nHvXVAd3JZsxCiIfU2CdTWpcvfyxxhVaDnuxI0tW0XNlbgS1JebnuPQeAm_gtRJfm7yER_ybg72bUySkBgI_zdy2uv91fTDTLY6KYn8jr4nQwQalSAXvFCyii5U__fgMzenQ1m5ykYsvn9LH1GSSKNLFBmZQ06HmmeMlxMW8byMfEnmsDHswBxwXMw3ZguWPcufpclmIZGkdPXRGVDtWftLpPehoynBagEdKPDwCNRa8JTqowduvnNdJKSvPDsQugOasbNshkolC0PN4HTt1SWKlTzbGTtHkJqilrLFBUF2xSKIfTbXiLTBO6E97Th6CLNIP9SLo7bYV7MfACErF_ZA_g2wevMbFeucw3Pochij6anINXaxo1p_jWRRrGQsTjbW8gE0otuZ1b_kLUUKBxUSCKIO9CykEM9CYz97yuZccQyK7XDmm9fgvmYAB7YunJuT2FhWGPw=w640-h99-no)
But P[7] is 7, the **LP** expands all the way across the left edge of **CP**, now we should use property of *case 2*, the new palindrome is at least i'-L long.
![case 2](https://lh3.googleusercontent.com/W8EwJ05dcM9OC8jjfOiIMVtVfXWkNatS_dQ3ch0oegWxnGtpCwEXjb9XjDo2rjicECOaBp8AbakvcNX21QJW5ILnYFCZHxXUjCfErbsAe_1gJ6OiGuzUbBxu1z38RBClo3vk2WFMpjaMi9U1subr3xYz2oLfy36gJ9yv3QF_f3fFCR6qwKRhSSjo1KLmeOt5tWyGvdPm8_yT7KiLI6F9WLu9G9G5Q9fi-tpJtG9IgiParNRoqToOFIqvuwMOdt5YYTuXjZwQsCIpEx0gYmdAVSo71OBh-lZdU3KJ09j0b0cFJ6Svw24-KaKEn2TT9uRgFvfSyqn4cVraRx7EFCHAhzmmcG3k1ixGsiFsJVxImLFOck3WT8bbgNTjuVU-3hC71__sue8esM9gi2MLqCyASiD3XPwjRll4vbLl7a_1CU4Q-aj-uqLeVaTVsmv55JnAa7umeXRuNNaVeBoypf4rJGQxx0MMHwtBE7aQAcbttyYWBOtxQhBLgaV9yPmHXS1VowI7abLcQQkKz8kALF4-Yoo1wGnz0-XC_SeTtQRnRQL-ePyO3S3IgCophulqcj9RWEH_kI6JH7nez4CieSU1ygBNDl2EwA=w640-h99-no)
*within the range of L to R, all the character are symmetric around C, solid green lines must match for both side, and dotted green lines match too. But since the red lines are out of the center palindrome, they must not match, so P[15] is i'-L = R - i = 20 -15 = 5*
So we can optimize function radius further:
``` c++
size_t radius(const string& s,   // transformed string
              const size_t i,    // index of element to be proceed
              vector<size_t>& p, // saves radius of each character
              const size_t c     // the center of current most right palindrome
              ) {
	if (i < c + p[c]) {
		size_t ii = 2 * c - i;   // the mirrored point of i around c
		size_t l = c - p[c];     // the left edge of palindrome c
		size_t ll = ii - p[ii];  // the left edge of palindrome ii
		if (ll > l) { // palindrome ii is within palindrome c
			p.push_back(p[ii]);
			return p[i];
		}
		else if (ll < l) {  // palindrome ii is beyond palindrome c
			p.push_back(l - ll);
			return p[i];
		}
	}
	
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
	
	p.push_back(step-1);
	return p[i];
}
```
What if **LP** and **CP** left edge are same? We don't know the exact number of P[i], all we knew is P[i] >= P[i'], and we have to do character matching by expending pass the right edge, like case 3 described.
Now the function radius becomes:
``` c++
size_t radius(const string& s,   // transformed string
              const size_t i,    // index of element to be proceed
              vector<size_t>& p, // saves radius of each character
              const size_t c     // the center of current most right palindrome
              ) {
	size_t step = 1;           // temp variable to record radius of a palindrome
	if (i < c + p[c]) {
		size_t ii = 2 * c - i;   // the mirrored point of i around c
		size_t l = c - p[c];     // the left edge of palindrome c
		size_t ll = ii - p[ii];  // the left edge of palindrome ii
		if (ll > l) { // palindrome ii is within palindrome c
			p.push_back(p[ii]);
			return p[i];
		}
		else if (ll < l) {  // palindrome ii is beyond palindrome c
			p.push_back(l - ll);
			return p[i];
		}
		else {
			step += p[ii];
		}
	}
	
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
	
	p.push_back(step-1);
	return p[i];
}
```
Now we finished the complete LPS program, but wait, it still have room to improve.
Look at the if statement to determine palindrome ii is in or out of the palindrome c, it can be simplified as p[i] >= min(right_edeg-i, p[ii]), and we can do an extra compare even if palindrome ii is in or out of palindrome c.
``` c++
size_t radius(const string& s,   // transformed string
              const size_t i,    // index of element to be proceed
              vector<size_t>& p, // saves radius of each character
              const size_t c     // the center of current most right palindrome
              ) {
	p.push_back(0);            // p[i] is initially 0 before any matching
	size_t right = c + p[c];
	size_t ii = 2 * c - i;     // the mirrored point of i around c
		
	if (i < right) {
		p[i] = min(p[ii], right - i);
	}
	
	size_t end = s.size() - 1; // the right boundary of the string
	while ((p[i] < i) && (i + p[i] < end)) {
		if (s[i-p[i]-1] == s[i+p[i]+1]) {	
			p[i]++; // test outter pair characters if match
		}
		else {
			break;  // stop comparison
		}
	}
	
	return p[i];
}
```