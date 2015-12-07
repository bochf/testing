#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>

#define IS_BIG_ENDIAN    1
#define IS_LITTLE_ENDIAN 0

/* ===========================================================================
 * Name: GetEndianValue
 * Description: 
 * Paramaters: 
 * Returns: See defines above
 * Side effects: 
 * Notes: 
 */
int GetEndianValue(void);

/* ===========================================================================
 * Name: SwapEndian16
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side effects: 
 * Notes: 
 */
int16_t SwapEndian16(int16_t in);

/* ===========================================================================
 * Name: SwapEndianU16
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side effects: 
 * Notes: 
 */
uint16_t SwapEndianU16(uint16_t in);

/* ===========================================================================
 * Name: SwapEndian32
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side effects: 
 * Notes: 
 */
int32_t SwapEndian32(int32_t in);

/* ===========================================================================
 * Name: SwapEndianU32
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side effects: 
 * Notes: 
 */
uint32_t SwapEndianU32(uint32_t in);

#endif
