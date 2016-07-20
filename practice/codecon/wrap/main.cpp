//Problem        : Wrapper's Delight
//Language       : C++
//Compiled Using : g++
//Version        : GCC 4.9.1
//Input for your program will be provided from STDIN
//Print out all output from your program to STDOUT

#include <iostream>
#include <string>
#include <algorithm>
#include <climits> 

using namespace std;

void addWord(string& s, const string& word, size_t& len, const size_t limit) {
	if (len + word.size() > limit) {
		s += "\n";
		len = 0;
	}
	s += word;
	len += word.size();
	s += " ";
	++len;
};

string wrap(const string& s, const size_t limit) {
    if (s.empty())
        return "";
        
    string out = "";
    string buffer = "";
    size_t len = 0;
    
    for (size_t i = 0; i < s.size(); ++i) {
    	switch(s[i]) {
    		case ' ':
    			addWord(out, buffer, len, limit);
    			buffer = "";
    			break;
    		case '~':
    			addWord(out, buffer, len, limit);
    			out += "\n";
    			len = 0;
    			buffer = "";
    		break;
    		default:
    			buffer += s[i];
    	}
    }
    addWord(out, buffer, len, limit);

    return out;
};

int main() {
    size_t len;
    string s;
    
    cin >> len;
    getline(cin, s);
    getline(cin, s);
    
    cout << wrap(s, len) << endl;

    return 0;
}
