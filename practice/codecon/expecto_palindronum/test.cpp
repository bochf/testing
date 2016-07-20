//Problem        : Expecto Palindronum
//Language       : C++
//Compiled Using : g++
//Version        : GCC 4.9.1
//Input for your program will be provided from STDIN
//Print out all output from your program to STDOUT

#include <iostream>
#include <string>
#include <algorithm>
#include <climits> 
#include <vector>

using namespace std;

void transform(const string& s, string& t) {
    if (s.empty())
        return;
        
    t = "#";
    for (size_t i = 0; i < s.size(); ++i) {
        t += s[i];
        t += '#';
    }
    t += "$";
}

size_t lps0(const string& s) {
    if (s.empty())
        return 0;
        
    string t;
    transform(s, t);
    
    vector<size_t> p;
    p.push_back(0);
    size_t center = 0;
    size_t right = 0;
    
    for (size_t i = 1; i < t.size() - 1; ++i) {
        if (right > i) {
            p.push_back(min(p[2*center-i], right-i));
        }
        else {
            p.push_back(0);
        }
        
        while (t[i+p[i]+1] == t[i-p[i]-1]) {
            p[i]++;
        }
        
        if (right < i+p[i]) {
            center = i;
            right = i+p[i];
        }
    }
    
    size_t len = 0;
    for (size_t i = 0; i < p.size(); ++i) {
        if ((i == p[i]) && (len < i)) {
            len = i;
        }
    }
    return len;
}

int main() {
    string s;
    
    cin >> s;
    
    cout << s.size()*2- lps0(s) << endl;

    return 0;
}
