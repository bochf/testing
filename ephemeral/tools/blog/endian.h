#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>

int IsBigEndian(void);

int IsSameEndian(int file);

char GetEndianValue(void);





uint64_t endian_swap_u64(uint64_t value);


#endif
