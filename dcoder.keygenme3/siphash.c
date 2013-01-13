#include "siphash.h"


static inline uint64_t rotl64(uint64_t u, int s)
{
        return (u << s) | (u >> (64 - s));
}


static inline void sipround(uint64_t* v0, uint64_t* v1, uint64_t* v2, uint64_t* v3)
{
        *v0 += *v1;
        *v1 = rotl64(*v1, 13);
        *v1 ^= *v0;
        *v0 = rotl64(*v0, 32);

        *v2 += *v3;
        *v3 = rotl64(*v3, 16);
        *v3 ^= *v2;

        *v2 += *v1;
        *v1 = rotl64(*v1, 17);
        *v1 ^= *v2;
        *v2 = rotl64(*v2, 32);

        *v0 += *v3;
        *v3 = rotl64(*v3, 21);
        *v3 ^= *v0;
}


static inline void sipcompress2(uint64_t* v0, uint64_t* v1, uint64_t* v2, uint64_t* v3, uint64_t m)
{
        *v3 ^= m;

        sipround(v0, v1, v2, v3);
        sipround(v0, v1, v2, v3);

        *v0 ^= m;
}


static inline uint64_t get64le(void const* data, size_t ix)
{
        uint8_t const* p = (uint8_t const*)data + ix * 8;
        uint64_t ret = 0;

        for (size_t i = 0; i < 8; ++i) {
                ret |= (uint64_t)p[i] << (i * 8);
        }

        return ret;
}


static inline void put64le(uint64_t v, void* out)
{
        uint8_t* p = (uint8_t*)out;

        for (size_t i = 0; i < 8; ++i) {
                p[i] = (uint8_t)(v >> (i * 8));
        }
}


static inline uint8_t get8(void const* data, size_t ix)
{
        return *((uint8_t const*)data + ix);
}


static inline uint64_t siplast(void const* data, size_t size)
{
        uint64_t last = 0;

        for (size_t i = 0; i < size % 8; ++i) {
                last |= (uint64_t)get8(data, size / 8 * 8 + i) << (i * 8);
        }
        last |= (uint64_t)(size % 0xff) << (7 * 8);

        return last;
}


void siphash24(uint8_t const* key, void const* data, size_t size, uint8_t* out)
{
        uint64_t key0 = get64le(key, 0);
        uint64_t key1 = get64le(key, 1);

        uint64_t v0 = key0 ^ 0x736f6d6570736575ull;
        uint64_t v1 = key1 ^ 0x646f72616e646f6dull;
        uint64_t v2 = key0 ^ 0x6c7967656e657261ull;
        uint64_t v3 = key1 ^ 0x7465646279746573ull;

        for (size_t i = 0; i < size / 8; ++i) {
                sipcompress2(&v0, &v1, &v2, &v3, get64le(data, i));
        }
        sipcompress2(&v0, &v1, &v2, &v3, siplast(data, size));

        v2 ^= 0xff;

        sipround(&v0, &v1, &v2, &v3);
        sipround(&v0, &v1, &v2, &v3);
        sipround(&v0, &v1, &v2, &v3);
        sipround(&v0, &v1, &v2, &v3);

        put64le(v0 ^ v1 ^ v2 ^ v3, out);
}

