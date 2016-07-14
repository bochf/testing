#ifndef __UTIL_H__
#define __UTIL_H__

#include <cstdint>

/**
 * @function getPair
 * @brief find the multiplicator which makes the units of product
 * equals to the given number, all the parameters and return value
 * are one digit, and the multiplicand must be 1, 3, 7, or 9.
 *
 * @param[in] n, multiplicand
 * @param[in] p, last digit of product
 * @return multiplicator, one digit number
 */
uint8_t getPair(const uint8_t n, const uint8_t p);

/**
 * @function removeOnes
 * @brief remove all the "1" in tail, e.g. 1234561 => 123456, 33224157111 => 33224157
 *
 * @param[in][out] n, the number to be manipulated
 * @return number of "1" were removed
 */
uint8_t removeOnes(uint64_t &n);

#endif
