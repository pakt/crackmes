// keygen for keygenm2 by tamaroth
// 
// 25.03.2012
// p_k
#include <assert.h>          
#include <string.h>          
#include <stdio.h>          
#include <stdlib.h>          
#include <stdint.h>
#include <time.h>
#include <gmp.h>
#include "skein.h"
#include "SHA3api_ref.h"

#include "kg.h"

#define HASH_SIZE 32

extern void init_smth();
extern void do_smth64(uint8_t *, size_t, uint8_t *); //msg,len,out

void print_bytes(uint8_t *bytes, uint16_t byteslen)
{
	int i;
	for (i=0; i<byteslen; i++) {
		printf("%02x ", bytes[i]);
	}
    printf("\n");
}

void hash(uint8_t *msg, size_t msg_size, uint8_t *result){
    Skein_256_Ctxt_t ctx;

    Skein_256_Init(&ctx, 256);
    Skein_256_Update(&ctx, msg, msg_size);
    Skein_256_Final(&ctx, result);
}

void product(mpz_t p, int *tab, int count){
    int i;
    mpz_t tmp;

    mpz_set_ui(p, 1);

    for(i=0;i<count;i++){
        mpz_mul_ui(p, p, tab[i]);
    }

}

void factor(mpz_t n, int *primes, int count, int *pi, int *ei, int *factors){
    int i,total, e;
    unsigned int p;

    total = 0;
    for(i=0;i<count;i++){
        p = primes[i];
        if(mpz_divisible_ui_p(n, p)){
            e = 0;
            while(mpz_divisible_ui_p(n, p)){
                mpz_divexact_ui(n, n, p);
                e++;
            }
            pi[total] = i;
            ei[total] = e;
            total++;
        }
    }

    *factors = total;
}

void calc_dlog(mpz_t dlog, mpz_t p, char *dlogs[], int *pi, int *ei, int factors){
    int i,j,idx;
    mpz_t t;

    mpz_init(t);
    mpz_set_ui(dlog, 0);
    
    mpz_sub_ui(p,p,1); //phi(p)=p-1
    for(i=0;i<factors;i++){
        idx = pi[i];
        mpz_init_set_str(t, dlogs[idx], 10);
        mpz_mul_ui(t, t, ei[i]);
        mpz_add(dlog, dlog, t);
        mpz_mod(dlog, dlog, p);
    }
    mpz_add_ui(p,p,1);
    mpz_clear(t);
}

// phash[] - prime -> index
// pi[] - indexes in primes[] tab of primes in factorisation of n
// e[] - exponents of these primes
int is_smooth(mpz_t n, mpz_t prod){
    int ok=0, eq;
    mpz_t t,d;

    mpz_inits(t,d,NULL);
    mpz_set(t, n);

    while(1){
        mpz_gcd(d, t, prod); //d doesn't have to fit in int
        if(mpz_cmp_ui(d,1) == 0){
            eq = mpz_cmp_ui(t, 1);
            if(eq != 0){
                ok=0;
                break; //fail: gcd=1, but t!=1
            }
            else{ //gcd=1, t=1
                ok=1;
                break;
            }
        }
        while(mpz_divisible_p(t, d)){
            mpz_divexact(t, t, d);
        }
    }

    mpz_clear(t);
    return ok;
}

inline void do_hash(char *msg, mpz_t h, mpz_t p){
    uint8_t smth[8]={0}, result[HASH_SIZE];
    char *ptr;

    memset(result, 0, HASH_SIZE);

    do_smth64(msg, strlen(msg), smth);
    //print_bytes(smth, sizeof(smth));

    hash(smth, sizeof(smth), result);
    //print_bytes(result, HASH_SIZE);

    mpz_import(h, HASH_SIZE, 1, 1, 1, 0, result);
    mpz_mod(h, h, p);

    //ptr = mpz_get_str (NULL, 16, h);
    //printf("h mod p = %s\n", ptr);
}

void dump_pair(int p, int e){
    if(e>1)
        printf("%d^%d", p, e);
    else
        printf("%d", p);
}

void dump_pe(int *primes, int *pi, int *ei, int count){
    int i;

    assert(count>0);
    dump_pair(primes[pi[0]], ei[0]);
    if(count>1){
        for(i=1;i<count;i++){
            printf(" * ");
            dump_pair(primes[pi[i]], ei[i]);
        }
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    mpz_t a, b, p, g, h, prod, dlog;
    char *ps = "71725366866332810664388831579";
    char *gs = "40876369110700248071181199787";
    #define MAX_NAME 32
    char name[MAX_NAME];
    char rnd[2*MAX_NAME];
    char *ptr;
    int total;
    clock_t start;
    double delta;
    #define MAX_FACTORS 128
    int pi[MAX_FACTORS], ei[MAX_FACTORS], factors=0;
    int r;

    if(argc<2){
        printf("Usage: %s <name>\n", argv[0]);
        return 1;
    }
    
    if(strlen(argv[1])>MAX_NAME-1){
        printf("name too long\n");
        return 1;
    }
    
    strcpy(name, argv[1]);

    srand(time(0));

    mpz_inits(h, prod, dlog, NULL); 
    mpz_init_set_str (p, ps, 10);
    mpz_init_set_str (g, gs, 10);

    product(prod, primes, PRECOMP_COUNT);
    ptr = mpz_get_str (NULL, 10, prod);
    //printf("prod = %d\n", strlen(ptr));

    init_smth();
    start = clock();

    total = 0;
    while(1){
        total += 1;

        r = rand();
        sprintf(rnd, "%s%d", name, r);

        do_hash(rnd, h, p);

        if(is_smooth(h, prod)){
            delta = ((double)clock() - start)/ CLOCKS_PER_SEC;
            ptr = mpz_get_str (NULL, 10, h);
            printf("smooth h = %s, time: %fs\n", ptr, delta);
            factor(h, primes, PRECOMP_COUNT, pi, ei, &factors);
            printf("factorization: "); dump_pe(primes, pi, ei, factors);
            calc_dlog(dlog, p, dlps, pi, ei, factors);
            ptr = mpz_get_str (NULL, 16, dlog);
            printf("*** reg info:\n%s\n%d\n%s\n", name, r, ptr);
            break;
        }
        if(total%10000==0){
            delta = ((double)clock() - start)/ CLOCKS_PER_SEC;
            printf("delta: %f, speed: %f h/s\n", delta, (double)total/delta);
        }
    }

    return 0;
}

