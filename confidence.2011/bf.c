#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;

uint tab_hi[] = {0x474681bf, 0x76c8876b, 0xed910ed6, 0x236799b9,
			0xbe8ab767, 0x7d156ece, 0x026f5989, 0x04deb312,
			0x09bd6624, 0x137acc48, 0x26f59890, 0x4deb3120,
			0x6393e655, 0x3f6248bf, 0x7ec4917e, 0xfd8922fc,
			0x0357c1ed, 0xfeea07cf, 0xfdd40f9e, 0xfba81f3c,
			0x0f15ba6d, 0x1e2b74da, 0xc4136da1, 0x8826db42,
			0xe8083291, 0xd0106522, 0x58654e51, 0xb0ca9ca2,
			0x61953944, 0xc32a7288, 0x7e116105, 0xfc22c20a};

uint tab_lo[] = {0x8d915d6b, 0x7ed5ef07, 0xfdabde0e, 0x9ea0e9cc,
			0x58b68649, 0xb16d0c93, 0x072d4cf7, 0x0e5a99ee, 0x1cb533dc,
			0x396a67b8, 0x72d4cf70, 0xe5a99ee0, 0xaea46811, 0x38bf85f3,
			0x717f0be6, 0xe2fe17cc, 0xa00b7a48, 0x25e1a141, 0x4bc34283,
			0x97868507, 0x4afa5fde, 0x95f4bfbc, 0x4e1e2aa9, 0x9c3c5553,
			0x5d8fff76, 0xbb1ffeed, 0x13c8a80a, 0x27915014, 0x4f22a029,
			0x9e454052, 0x597dd574, 0xb2fbaae8};

void crc64(uint x, uint *h, uint *l){
	int i;
	uint hi,lo, b;

	hi = lo = b = 0;

	for(i=0;i<32;i++){
		b = (x>>i) & 1;
		if (b){
			hi ^= tab_hi[i];
			lo ^= tab_lo[i];
		}
	}

	*h = hi;
	*l = lo;
}

void test(){

	uint h,l;

	crc64(0xaabbccdd, &h, &l);
	printf("test: 0x%08x 0x%08x\n", h, l);
	crc64(0x3, &h, &l);
	printf("test: 0x%08x 0x%08x\n", h, l);
}

//.text:00401553                 push    160h            ; _DWORD
//.text:00401558                 push    offset computed_hash ; _DWORD
//.text:0040155D                 call    interestingFunc
//.text:00401563                 add     esp, 8
//.text:00401566                 cmp     edx, 53534532h
//.text:0040156C                 jnz     short loc_401579
//.text:0040156E                 cmp     eax, 33444F4Eh
//.text:00401573                 jnz     short loc_401579

//.text:08002C0F                 mov     [ebp+hi_dword], esi
//.text:08002C34                 mov     eax, esi
//.text:08002C36                 xor     eax, 6008E054h
//.text:08002C3B                 mov     edx, [ebp+lo_dword]
//.text:08002C41                 xor     edx, 6B3E997Ah

// 6B3E997A ^ 53534532 = 386DDC48 (lo dword)
// 6008E054 ^ 33444F4E = 534CAF1A (hi dword)
//
// 0x01814a43 -> 0x70cd1b75 0x386ddc48 (h3, l3)
//
//
// h3' = h3 ^ old_l2 == 534caf1a
// h3 == 0x70cd1b75
// old_l2 = 70cd1b75 ^ 534caf1a = 2381b46f
//
//
// 0xffffc8ca -> 0xcb49b5bc 0x2381b46f
//

int main(int argc, char *argv[]){

	uint h,l,dw;

	//test();

	for(dw=0;dw<0xFFFFFFFF;dw++){
		crc64(dw, &h, &l);

		//if(l == 0x386DDC48){
		if(l == 0x2381B46F){
			printf("lol: 0x%08x 0x%08x 0x%08x\n", dw, h, l);
			return 0;
		}

		if(dw % (1<<27) == 0){
			printf("dw=0x%08x\n", dw);
		}
	}

	return 0;
}
