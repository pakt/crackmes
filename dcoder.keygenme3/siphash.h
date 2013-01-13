#ifndef __SIPHASH_H__
#define __SIPHASH_H__

#include <stddef.h>
#include <stdint.h>


void siphash24(uint8_t const* key16, void const* data, size_t size, uint8_t* out8);


#endif

