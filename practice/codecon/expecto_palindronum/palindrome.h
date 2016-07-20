#ifndef __PALINDROME_H__
#define __PALINDROME_H__

#include <string>
#include <vector>

/**
 * @brief adding speicial character in between
 * @function transform
 * @param s, the given string
 */
void transform(std::string& s);

/**
 * @brief calculate the radius of a palindrome in the string
 * @function radius
 * @param s, the given string
 * @param i, the index of the element in the string, which will the center of the palindrome
 * @param p, the array saves radius of each element
 * @param c, the index of the center of the most right palindrome
 * @return number of matched character pairs.
 */
size_t radius(const std::string& s, const size_t i, std::vector<size_t>& p, const size_t c);

/**
 * @brief get the length of longest palindrome substring
 * @function lps
 * @param s, the given string
 * @return length of longest palindrome substring
 */
size_t lps(const std::string& s);

class Palindrome {
    public:
    Palindrome(std::string& _s) : s(_s) {};
    
    /**
     * @brief get the longest palindrome substring
     * @function lps
     * @return one of the longest palindrome substring in the string
     */
    virtual const std::string lps();
    
    /**
     * @brief get the longest palindrome substring which is start from Nth element
     * @function lpsN()
     * @return the longest palindrome substring
     */
    virtual const std::string lps0();
    
    private:
    std::vector<size_t> p;
    std::string s;
    
    void transform(const std::string& s, std::string& t);
    size_t calculate();
};

#endif
