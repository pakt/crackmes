// compile with
// gcc bf.c -O3
// run with go.py
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

inline int  vm_fancy0(int a1)
{
  signed int v2; // [sp+4h] [bp-Ch]@1
  int v3; // [sp+8h] [bp-8h]@1
  unsigned int i; // [sp+Ch] [bp-4h]@1

  v2 = 63689;
  v3 = 0;
  for ( i = 0; i <= 0x1F; ++i )
  {
    v3 = v2 * v3 + i * a1;
    v2 *= 378551;
  }
  return v3;
}

inline unsigned int  vm_fancy1(int a1)
{
  unsigned int v2; // [sp+8h] [bp-8h]@1
  unsigned int i; // [sp+Ch] [bp-4h]@1

  v2 = 1315423911;
  for ( i = 0; i <= 0x1F; ++i )
    v2 ^= i * a1 + 32 * v2 + (v2 >> 2);
  return v2;
}

inline unsigned int  vm_fancy2(int a1)
{
  unsigned int v2; // [sp+14h] [bp-Ch]@1
  unsigned int i; // [sp+1Ch] [bp-4h]@1

  v2 = 0;
  for ( i = 0; i <= 0x1F; ++i )
  {
    v2 = 16 * v2 + i * a1;
    if ( v2 & 0xF0000000 )
      v2 = (v2 ^ ((v2 & 0xF0000000) >> 24)) & 0xFFFFFFF;
  }
  return v2;
}

inline int  vm_fancy3(int a1)
{
  int v2; // [sp+4h] [bp-Ch]@1
  int v3; // [sp+4h] [bp-Ch]@2
  int v4; // [sp+8h] [bp-8h]@2
  unsigned int i; // [sp+Ch] [bp-4h]@1

  v2 = 0;
  for ( i = 0; i <= 0x1F; ++i )
  {
    v3 = 16 * v2 + i * a1;
    v4 = v3 & 0xF0000000;
    if ( v3 & 0xF0000000 )
      v3 ^= (unsigned int)v4 >> 24;
    v2 = ~v4 & v3;
  }
  return v2;
}

inline int  vm_fancy4(int a1)
{
  int v2; // [sp+8h] [bp-8h]@1
  unsigned int i; // [sp+Ch] [bp-4h]@1

  v2 = 0;
  for ( i = 0; i <= 0x1F; ++i )
    v2 = 131 * v2 + i * a1;
  return v2;
}

inline int  vm_fancy5(int a1)
{
  int v2; // [sp+8h] [bp-8h]@1
  unsigned int i; // [sp+Ch] [bp-4h]@1

  v2 = 0;
  for ( i = 0; i <= 0x1F; ++i )
    v2 = (v2 << 6) + i * a1 + (v2 << 16) - v2;
  return v2;
}

inline int  vm_fancy6(int a1)
{
  int v2; // [sp+8h] [bp-8h]@1
  unsigned int i; // [sp+Ch] [bp-4h]@1

  v2 = 0;
  for ( i = 0; i <= 0x1F; ++i )
    v2 = i * a1 ^ -2128831035 * v2;
  return v2;
}


inline unsigned int  vm_fancy7(int a1)
{
  signed int v2; // [sp+0h] [bp-14h]@3
  unsigned int v3; // [sp+Ch] [bp-8h]@1
  unsigned int i; // [sp+10h] [bp-4h]@1

  v3 = 0xAAAAAAAAu;
  for ( i = 0; i <= 0x1F; ++i )
  {
    if ( i & 1 )
      v2 = ~((v3 << 11) + (i * a1 ^ (v3 >> 5)));
    else
      v2 = i * a1 * (v3 >> 3) ^ (v3 << 7);
    v3 ^= v2;
  }
  return v3;
}

#define rol(value, bits)   (((value) << (bits)) | ((value) >> (32 - (bits))))
#define DW(p) *(unsigned int*)(p)

inline unsigned int round0(unsigned int sn2, unsigned char *mem){
    unsigned int r1, r2, hr1,tmp1,tmp2;

    tmp1 = tmp2 = 0;
    r1 = sn2;
    hr1 = r1;
    r1 = r1 & 0xF;
    r1 = r1 << 6;
    tmp1 = r1;
    r1 = hr1;
    while(1){
        hr1 = r1;
        r1 = tmp2;
        r2 = tmp1;
        r1 += 1;
        tmp2 = r1;
        if(r1-1 == r2)
            break;
        r1 = hr1;
        r1 = vm_fancy1(r1);
        r1 = ~r1;
        r1 = vm_fancy3(r1);
        r2 = tmp2;
        r1 = r1 ^ r2;
        r1 = vm_fancy2(r1);
        r1 += 0xdeadbeef;
        r1 = rol(r1, 7);
        r1 = vm_fancy0(r1);
    }
    r1 = hr1;
    // mix_handlers 0

    // cleanup
    tmp1 = 0;
    tmp2 = 0;

    return r1;
}

unsigned int round1(unsigned int sn2, unsigned char *mem){
    unsigned int r1, r2, hr1, tmp1, tmp2, t;

    tmp1 = tmp2 = 0;
    r1 = sn2;
    hr1 = r1;
    r1 = r1 >> 4;
    r1 = r1 & 0xF;
    r1 = r1 << 6;
    tmp1 = r1;
    r1 = hr1;
    while(1){
        hr1 = r1;
        r1 = tmp2;
        r2 = tmp1;
        r1 += 1;
        tmp2 = r1;
        if(r1-1 == r2)
            break;
        r1 = hr1;
        r1 = vm_fancy5(r1);
        t = r1;
        r1 = r2;
        r2 = t;
        r1 = r1*r2;
        r1 = vm_fancy7(r1);
        r1 = r1 ^ r2;
        r1 = vm_fancy6(r1);
        r1 = ~r1;
        r1 = vm_fancy4(r1);
        r1 = rol(r1, 0xd);
    }
    r1 = hr1;
    return r1;
}

int testx(int x, unsigned int sn2, unsigned int res, unsigned char *mem){
    unsigned int r1;

	switch(x){
		case 0:
	        r1 = round0(sn2, mem);
	        break;
		case 1:
			r1 = round1(sn2, mem);
			break;
		default:
        	return 0;
    }

    return (r1==res);
}

int main(int argc, char *argv[]){
    unsigned char mem[0x10000];
    unsigned int r1, sn1, sn2;
    unsigned int total;
    unsigned N, id, end;
    double seconds;

    if(argc<3){
        printf("%s <total slaves> <slave id>\n", argv[0]);
        return 1;
    }

    if(!testx(0, 0x11111111, 0x3a240c00, mem)){
        printf("test0 failed\n");
        return 1;
    }
    if(!testx(1, 0x11111111, 0xA80E02FC, mem)){
        printf("test1 failed\n");
        return 1;
    }
    printf("all good\n");

    N = atoi(argv[1]);
    id = atoi(argv[2]);

    sn2 = 0xFFFFFFFF/N;
    end = (id+1)*sn2;
    sn2 = id*sn2;
    total = 0;
    clock_t start = clock();
    while(sn2 != end){
        r1 = round0(sn2, mem);
        if(r1 == 0x9EE03FC0){
            r1 = round1(sn2, mem);
            if(r1 == 0x38E008E){
                printf("sn2 = 0x%08x\n", sn2);
                //return 0;
            }
        }
        if(sn2 % (1<<20) == 0){
            seconds = ((double)clock() - start) / CLOCKS_PER_SEC;
            seconds ++;
            printf("current: 0x%08x, %d h/sec\n", sn2, total/(int)seconds);
        }
        sn2++;
        total++;
    }

    return 0;
}

