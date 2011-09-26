#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

//flint
#include <fmpz.h>

extern void hash_mix(int x);

#define ROR(x,shift) (x >> shift) | (x << (32 - shift))
#define ROL(x,shift) (x << shift) | (x >> (32 - shift))

void encode_serial(unsigned int *a1, unsigned int *a2, unsigned int *a3, unsigned int *a4, int rounds){
    int c;

    c = rounds;
    do {
        *a4 += *a3;
        *a1 = *a1 ^ *a4;
        *a1 = ROL(*a1, 16);

        *a2 += *a1;
        *a3 = *a3 ^ *a2;
        *a3 = ROL(*a3, 12);

        *a4 += *a3;
        *a1 = *a1 ^ *a4;
        *a1 = ROL(*a1, 8);

        *a2 += *a1;
        *a3 = *a3 ^ *a2;
        *a3 = ROL(*a3, 7);
        c--;
    }while(c);
}

void decode_serial(unsigned int *a1, unsigned int *a2, unsigned int *a3, unsigned int *a4, int rounds)
{
  unsigned int v4; // edi@2
  unsigned int v5; // edi@2
  unsigned int v6; // edi@2
  unsigned int v7; // edi@2
  unsigned int v8; // [sp+8h] [bp-4h]@1

  v8 = rounds;
  do
  {
    v4 = ROR(*a3, 7);
    *a3 = v4;
    *a3 = v4 ^ *a2;
    *a2 -= *a1;
    v5 = ROR(*a1, 8);
    *a1 = v5;
    *a1 = v5 ^ *a4;
    *a4 -= *a3;
    v6 = ROR(*a3, 12);
    *a3 = v6;
    *a3 = v6 ^ *a2;
    *a2 -= *a1;
    v7 = ROR(*a1, 16);
    *a1 = v7;
    *a1 = v7 ^ *a4;
    *a4 -= *a3;
    --v8;
  }
  while ( v8 );

}

// ripped from crackme
void hash(char *name, char *out){
    unsigned int hash_buf[128/4];
    int i;
    int name_len, v15, name_len_masked, v18, tmp;
    char *name_ptr;
    int *v80;

    memset(hash_buf, 0, 128);
    
    hash_buf[0] = 8;
    hash_buf[1] = 1;
    hash_buf[2] = 8;
    i = 10;
    do{
        hash_mix((int)&hash_buf[-1]);
        --i;
    }while(i);

    /*
    printf("first 4 dwords:\n");
    printf("%08x %08x %08x %08x\n", hash_buf[0], hash_buf[1], hash_buf[2], hash_buf[3]);
    */
    //e0432a5b fc8d8d58 bd65b19c 667eaf03

    v80 = (int*)hash_buf;
    v15 = 0;
    tmp = 0;
    name_len = strlen(name);
    name_ptr = name;
    v18 = 8 * name_len;
    if ( 8 * name_len >= 8 )
    {
      name_len_masked = name_len & 0x1FFFFFFF;
      do
      {
        v18 -= 8;
        v80[v15 >> 5] ^= (unsigned char)*name_ptr << 8 * ((v15 >> 3) & 3);
        v15 = tmp + 8;
        ++name_ptr;
        tmp = v15;
        if ( v15 == 8 )
        {
          hash_mix((int)&hash_buf[-1]);
          v15 = 0;
          tmp = 0;
        }
        --name_len_masked;
      }
      while ( name_len_masked );
    }
    if ( v18 )
    {
      v80[v15 >> 5] ^= (unsigned char)*name_ptr << 8 * ((v15 >> 3) & 3);
      tmp += v18;
    }

    //hash_copy_8bytes
    hash_buf[0] ^= 0x80;
    hash_mix((int)&hash_buf[-1]);
    hash_buf[31] ^= 1;
    i = 10;
    do{
        hash_mix((int)&hash_buf[-1]);
        --i;
    }while(i);

    for(i=0;i<8;i++){
        out[i] = ((char*)hash_buf)[i];
    }
}

void split(fmpz_t x, unsigned int *lo, unsigned *hi){
    char s[64];
    uint64_t y;

    fmpz_get_str(s, 10, x);
    sscanf(s, "%"PRIu64"", &y);
    *lo = y & 0xffffffff;
    *hi = y >> (uint64_t)32;
}

void emit_sig(uint64_t hash_u, uint *sn12_lo, uint *sn12_hi, uint *sn34_lo, uint *sn34_hi){
    char hash_str[64];
    fmpz_t sn12, sn34, hash, a, k, dlog, order;

    fmpz_init(sn12);
    fmpz_init(sn34);
    fmpz_init(hash);
    fmpz_init(a);
    fmpz_init(k);
    fmpz_init(dlog);
    fmpz_init(order);

    //k=0x1122334455
    fmpz_set_str(k, "73588229205", 10);
    fmpz_set_str(a, "4297910449086477", 10);
    fmpz_set_str(dlog, "3414275298009790", 10);
    fmpz_set_str(order, "4518471260972087", 10);
    
    sprintf(hash_str, "%" PRIu64 "", hash_u);
    //printf("hash=%s\n", hash_str);
    fmpz_set_str(hash, hash_str, 10);
    fmpz_mod(hash, hash, order);

    /*
    fmpz_print(hash);
    printf("\n");
    //sn34 = ((k_rand-hash-a)*dlog)%order
    */

    fmpz_sub(sn34, k, hash);
    fmpz_sub(sn34, sn34, a);
    fmpz_mul(sn34, sn34, dlog);
    fmpz_mod(sn34, sn34, order);

    /*
    printf("sn34=");
    fmpz_print(sn34);
    printf("\n");
    */

    //sn12 = (hash+a)%order
    fmpz_add(sn12, hash, a);
    fmpz_mod(sn12, sn12, order);

    /*
    printf("sn12=");
    fmpz_print(sn12);
    printf("\n");
    */

    split(sn12, sn12_lo, sn12_hi);
    split(sn34, sn34_lo, sn34_hi);
    
    fmpz_clear(sn12);
    fmpz_clear(sn34);
    fmpz_clear(hash);
    fmpz_clear(a);
    fmpz_clear(k);
    fmpz_clear(dlog);
    fmpz_clear(order);
}

int main(int argc, char *argv[]){
    unsigned int a1, a2, a3, a4;
    //unsigned int tab[]={0x0005ffb7, 0xca2ab634, 0x0005f4b3, 0x7f1418c2};
    unsigned int name_hash[2];
    uint64_t hash64;

    if(argc<2){
        printf("%s <name>\n", argv[0]);
        return 1;
    }

    hash(argv[1], (char*)name_hash);

    hash64 = ((uint64_t)name_hash[1]<<32) + (uint64_t)name_hash[0];
    emit_sig(hash64, &a1, &a2, &a3, &a4);
    encode_serial(&a4, &a3, &a2, &a1, 32);

    printf("%08X%08X%08X%08X\n", a1, a2, a3, a4);

    return 0;
}

