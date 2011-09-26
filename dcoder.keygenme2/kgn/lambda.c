#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include <mpir.h>
#include <nmod_poly.h>

typedef nmod_poly_t poly_t;

#define MAX_N 52
#define MAX_INTERVAL ((uint64_t)1<<MAX_N)
#define M_SIZE (MAX_N) //too much

typedef struct {
    uint64_t r;
    poly_t x;
    poly_t y;
}triple_t ;

triple_t M[M_SIZE];

#define PRIME 8191
#define GENUS 4
#define H_IS_ZERO 1

#define init_poly(x) nmod_poly_init((x), PRIME)


poly_t d1, e1, e2;
//for hc_add
poly_t sum, s1, s2, s3, a, b, d, c1, c2, u3, v3;
//for hc_mul
poly_t Zx, Zy, Sx, Sy;


unsigned int gen_rand(){
    int x;
    x = (rand()<<16)+rand();

    return x;
}

void dump_poly(poly_t p){
    nmod_poly_print (p); printf ("\n");
}

void load_const(poly_t f, poly_t h, poly_t Px, poly_t Py, poly_t Qx, poly_t Qy){
    char *s_f = "10 8191 3076 1177 6969 294 6512 7340 5891 3050 0 1";
    char *s_Px =  "5 8191 1875 1721 5809 5647 1";
    char *s_Py = "4 8191 6019 3070 1666 688";
    /*
    char *s_Qx = "5 8191 5799 2698 5019 6401 1";
    char *s_Qy = "4 8191 89 5900 1987 2473";
    */

    char *s_Qx = "5 8191 4134 2027 4475 4255 1";
    char *s_Qy = "4 8191 6525 928 1361 6937";

    init_poly(f);
    init_poly(h);
    init_poly(Px);
    init_poly(Py);
    init_poly(Qx);
    init_poly(Qy);

    nmod_poly_set_str(s_f, f);
    nmod_poly_zero(h);
    nmod_poly_set_str(s_Px, Px);
    nmod_poly_set_str(s_Py, Py);
    nmod_poly_set_str(s_Qx, Qx);
    nmod_poly_set_str(s_Qy, Qy);
}

void hc_add(poly_t f, poly_t h, poly_t u1, poly_t v1, poly_t u2, poly_t v2, poly_t u, poly_t v){
    int deg, is_one, coeff;

    nmod_poly_xgcd(d1, e1, e2, u1, u2);

    nmod_poly_add(sum, v1, v2);
    if(!H_IS_ZERO){
        nmod_poly_add(sum, sum, h);
    }

    nmod_poly_xgcd(d, c1, c2, d1, sum);

    nmod_poly_mul(s1, c1, e1);
    nmod_poly_mul(s2, c1, e2);
    //nmod_poly_set(s3, c2);

    nmod_poly_mul(u3, u1, u2);

    deg = nmod_poly_degree(d);
    coeff = nmod_poly_get_coeff_ui(d, 0);
    is_one = deg==0 && coeff==1;
    
    if(!is_one){
        nmod_poly_div(u3, u3, d);
        nmod_poly_div(u3, u3, d);
    }
    
    nmod_poly_mul(a, s1, u1);
    nmod_poly_mul(a, a, v2);

    nmod_poly_mul(b, s2, u2);
    nmod_poly_mul(b, b, v1);

    nmod_poly_mul(sum, v1, v2);
    nmod_poly_add(sum, sum, f);
    nmod_poly_mul(sum, sum, c2); //*s3
    nmod_poly_add(sum, sum, b);
    nmod_poly_add(sum, sum, a);
    if(!is_one){
        nmod_poly_div(sum, sum, d);
    }
    nmod_poly_divrem(a, v3, sum, u3);

    while(1){
        if(!H_IS_ZERO){
            nmod_poly_mul(a, v3, h);
            nmod_poly_sub(s1, f, a);
        }
        else{
            nmod_poly_set(s1, f);
        }

        nmod_poly_mul(b, v3, v3);
        nmod_poly_sub(s1, s1, b);
        nmod_poly_div(s1, s1, u3);
        nmod_poly_zero(s2);
        if(!H_IS_ZERO){
            nmod_poly_sub(s2, s2, h);
        }
        nmod_poly_sub(s2, s2, v3);
        nmod_poly_divrem(a, s2, s2, s1);

        nmod_poly_set(u3, s1);
        nmod_poly_set(v3, s2);

        if(nmod_poly_degree(u3) <= GENUS)
            break;
    }

    nmod_poly_make_monic(u3, u3);
    
    nmod_poly_set(u, u3);
    nmod_poly_set(v, v3);

}

void hc_mul(poly_t f, poly_t h, uint64_t k, poly_t u1, poly_t v1, poly_t u, poly_t v){
    uint64_t bit;
    int i;
    
    nmod_poly_set(Zx, u1);
    nmod_poly_set(Zy, v1);

    //(1,0) - neutral element
    nmod_poly_zero(Sx);
    nmod_poly_set_coeff_ui(Sx, 0, 1);
    nmod_poly_zero(Sy);

    for(i=0;i<64;i++){

        bit = (uint64_t)1<<i;
        if(bit>k){
            break;
        }

        if(k & bit){
            hc_add(f, h, Sx, Sy, Zx, Zy, Sx, Sy);
        }
        hc_add(f, h, Zx, Zy, Zx, Zy, Zx, Zy);
    }

    nmod_poly_set(u, Sx);
    nmod_poly_set(v, Sy);
    
}

int is_pow2(uint64_t x){
    return (x != 0) && ((x & (x - 1)) == 0);
}

void init_pollard(){
    int i;

    for(i=0;i<M_SIZE;i++){
        init_poly(M[i].x);
        init_poly(M[i].y);
    }
}

unsigned int hash(poly_t p){
    int deg, i;
    unsigned int h=0, coeff;

    deg = nmod_poly_degree(p);
    for(i=0;i<deg;i++){
        coeff = nmod_poly_get_coeff_ui(p, i);
        h ^= coeff;
        h = (h >> 1) ^ (-(h & 1u) & 0xD0000001u); 
    }
    return h;
}

int pollard_lambda(poly_t f, poly_t h, poly_t Px, poly_t Py, poly_t Qx, poly_t Qy, \
                    uint64_t lo, uint64_t hi,  uint64_t *dlog)
{
    uint64_t width, r;
    int64_t c, d;
    int i,j,k,N;
    unsigned int hsh;
    poly_t Rx, Ry, Hx, Hy;

    if(lo >= hi)
        return 0;
    
    width = hi-lo;
    if(!is_pow2(width) || width > (uint64_t)MAX_INTERVAL)
        return 0;

    c = width;
    N = 0;
    while(c>>=1){
        N++;
    }
    N >>= 1;
    N = 1<<N;
    
    init_poly(Rx);
    init_poly(Ry);
    init_poly(Hx);
    init_poly(Hy);

    //printf("N=%d\n", N);
    for(i=0;i<2;i++){
        //printf("i=%d\n", i);
        k = 0;
        while(1<<k < N){
            r = gen_rand();
            r = r & (N-1);
            if(r==0) r = 1;
            //printf("r=%" PRIu64 "\n", r);
            M[k].r = r;
            hc_mul(f, h, r, Px, Py, M[k].x, M[k].y);
            k += 1;
        }
        hc_mul(f, h, hi, Px, Py, Rx, Ry);
        c = hi;
        for(j=0;j<N;j++){
            hsh = hash(Rx)%k;
            hc_add(f, h, Rx, Ry, M[hsh].x, M[hsh].y, Rx, Ry);
            c += M[hsh].r;
        }
        nmod_poly_set(Hx, Qx);
        nmod_poly_set(Hy, Qy);
        d = 0;
        while((c-d)>=(int64_t)lo){
            //printf("c-d=%" PRIu64 "\n", c-d);
            if(nmod_poly_equal(Hx, Rx) && nmod_poly_equal(Hy, Ry)){
                *dlog = c-d;
                return 1;
            }
            hsh = hash(Hx)%k;
            //printf("hsh=0x%08x\n", hsh);
            hc_add(f, h, Hx, Hy, M[hsh].x, M[hsh].y, Hx, Hy);
            d += M[hsh].r;
        }
    }
    *dlog = 0;
    return 0;
}
void benchmark(poly_t f, poly_t h, poly_t Px, poly_t Py, poly_t Qx, poly_t Qy, poly_t u, poly_t v){
    int i;
    clock_t start;
    int iters;

    iters = 1000000;
    start = clock();

    for(i=0;i<iters;i++){
        hc_add(f, h, Px, Py, Qx, Qy, u, v);
    }
    
    printf("iters=%d, time: %f\n", iters, ((double)clock()-start)/CLOCKS_PER_SEC);
}


void test(poly_t f, poly_t h, poly_t Px, poly_t Py, poly_t Qx, poly_t Qy){
    poly_t Rx, Ry, Tx, Ty;
    uint64_t k;
    
    init_poly(Rx);
    init_poly(Ry);
    init_poly(Tx);
    init_poly(Ty);

    k = ((uint64_t)1<<60)+1234;

    hc_mul(f, h, k, Px, Py, Rx, Ry);

    hc_mul(f, h, k/2, Px, Py, Tx, Ty);
    hc_add(f, h, Tx, Ty, Tx, Ty, Tx, Ty);

    printf("k=%" PRIu64 "\n", k);
    printf("k*P:\n");
    dump_poly(Rx);
    dump_poly(Ry);

    printf("k/2*P + k/2*P:\n");
    dump_poly(Tx);
    dump_poly(Ty);
}

int main(int argc, char *argv[]){
    poly_t f, h, Px, Py, Qx, Qy;
    poly_t x, y;
    uint64_t dlog, lo, hi, step;
    //uint64_t k;
    int found;
    clock_t start;

    srand(time(0));

    //init globals
    //req. for speed
    //hc_add variables
    init_poly(d1);
    init_poly(e1);
    init_poly(e2);
    init_poly(sum);
    nmod_poly_zero(sum);
    init_poly(s1);
    init_poly(s2);
    init_poly(s3);
    init_poly(a);
    init_poly(b);
    init_poly(d);
    init_poly(c1);
    init_poly(c2);
    init_poly(u3);
    init_poly(v3);
    //hc_mul variables
    init_poly(Zx);
    init_poly(Zy);
    init_poly(Sx);
    init_poly(Sy);

    load_const(f, h, Px, Py, Qx, Qy);

    init_poly(x);
    init_poly(y);

    /*
    nmod_poly_print (f); printf ("\n");
    nmod_poly_print (h); printf ("\n");

    hc_add(f, h, Px, Py, Qx, Qy, x, y);

    printf("P+Q:\n");
    nmod_poly_print (x); printf ("\n");
    nmod_poly_print (y); printf ("\n");

    //benchmark(f, h, Px, Py, Qx, Qy, x, y);

    test(f, h, Px, Py, Qx, Qy);
    
    k = ((uint64_t)1<<38);
    k += (uint64_t)123456;
    hc_mul(f, h, k, Px, Py, x, y);

    printf("k*P:\n");
    nmod_poly_print (x); printf ("\n");
    nmod_poly_print (y); printf ("\n");
    */

    printf("wait...\n");

    init_pollard();
    dlog=0;
    lo = 0;
    step = MAX_INTERVAL;
    hi = (uint64_t)1<<53;
    while(lo<hi){
        printf("lo=%" PRIu64 "\n", lo);
        start = clock();
        found = pollard_lambda(f, h, Px, Py, Qx, Qy, lo, lo+step, &dlog);
        printf("time: %f\n", ((double)clock()-start)/CLOCKS_PER_SEC);
        if(found){
            printf("dlog=%" PRIu64 "\n", dlog);
            return 0;
        }
        lo += step;
    }

    nmod_poly_clear(d1);
    nmod_poly_clear(e1);
    nmod_poly_clear(e2);
    nmod_poly_clear(sum);
    nmod_poly_clear(s1);
    nmod_poly_clear(s2);
    nmod_poly_clear(s3);
    nmod_poly_clear(a);
    nmod_poly_clear(b);
    nmod_poly_clear(d);
    nmod_poly_clear(c1);
    nmod_poly_clear(c2);
    nmod_poly_clear(u3);
    nmod_poly_clear(v3);
    //hc_mul
    nmod_poly_clear(Zx);
    nmod_poly_clear(Zy);
    nmod_poly_clear(Sx);
    nmod_poly_clear(Sy);

    return 1;
}

