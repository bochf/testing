#ifndef __BASE_ARITHMETIC_H__
#define __BASE_ARITHMETIC_H__

typedef unsigned long long ull;

/**
 * @brief convert a base-n number to base-10 integer
 * @function convertBaseN
 * @param[in] x, the base-n number (x >= 0, 2 <= n <= 16)
 * @return base-10 integer
 */
ull convertBaseN(const char * x);

#endif
