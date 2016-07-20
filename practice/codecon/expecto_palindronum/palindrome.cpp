#include <string>
#include <algorithm>

#include "palindrome.h"

using namespace std;

void transform(string& s) {
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

size_t lps(const string& s) {
  vector<size_t> p;
  size_t max = 0;
  size_t c = 0;  // the index of center of the most right palindrome

	// p[0] must be 0
	//p.push_back(0);
  for (size_t i = 0; i < s.size(); ++i) {
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

const string Palindrome::lps() {
	size_t index = calculate();
	return s.substr((index-p[index])/2, p[index]);
}

const string Palindrome::lps0() {
	if (s.empty())
		return "";
		
	calculate();
	
	size_t index = 0;
	for (size_t i = 0; i < p.size(); ++i) {
		if ((i == p[i]) && (p[index] < p[i])) {
			index = i;
		}
	}
	
	return s.substr(0, p[index]);
}

void Palindrome::transform(const string& s, string& t) {
	if (s.empty())
		t = "";
		
	t = "#";
	
	for (size_t i = 0; i < s.size(); ++i) {
		t += s[i];
		t += '#';
	}
	
	t += "$";
}

size_t Palindrome::calculate() {
	string t;
	transform(s, t);
	
	size_t index = 0; // center of current longest palindrome substring

	size_t c = 0;   // center of the right most palindrome substring
	size_t r = 0;   // right edge of the right most palindrome substring
	p.push_back(0); // radius of the first palindrome substring "#"
	for (size_t i = 1; i < t.size() - 1; ++i) {
		if (r > i)
			p.push_back(min(r-i, p[2*c-i]));
		else
			p.push_back(0);
			
		while (t[i+p[i]+1] == t[i-p[i]-1])
			p[i]++;
			
		if (p[index] < p[i])
			index = i;   // update longest palindrome substring index
		if (r < p[i]+i) {
			c = i;       // update the center of right most palindrome substring
			r = p[i]+i;  // update the right edge of the right most palindrom substring
		}
	}
	
	return index;
}