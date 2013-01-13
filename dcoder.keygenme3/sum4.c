//Keygen for keygenme3 by Dcoder
//------------------------------
//
//Siphash + high density subset sum problem
//solved with generalized birthday paradox.
//
//pk
//gdtr.wordpress.com
//29.12.2012
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "siphash.h"

#define LG_N 8
#define N (1<<LG_N)
#define BITS 64
#define L 22
#define PER_LIST (1<<L)
#define N_BUCKETS PER_LIST
#define LISTS 4
#define CLASSES 16 

#define KEY_NAME 0
#define KEY_SERIAL 1

typedef unsigned long long u64;
typedef unsigned int u32;

struct elem {
    u64 x;
    u32 low; //low L bits
    int inverted;
    struct elem *c1, *c2;
    u64 idx;
};

struct node { //linked list node
    struct elem* e;
    struct node *next;
};

struct result {
    u64 seed;
    int ri,k;
};

typedef struct node node_t;
typedef struct elem elem_t;
typedef struct result result_t;

node_t *buckets[LISTS+3][PER_LIST];
elem_t *lists[LISTS+3][PER_LIST];

u64 seeds[CLASSES][N];

u32 low(u64 x){
    u32 y;
    y = x;
    return y & ((1<<L)-1);
}

void inv(elem_t *l[]){
    int i;
    for(i=0;i<PER_LIST && l[i]!=NULL;i++){
        l[i]->x = -l[i]->x;
        l[i]->low = low(l[i]->x);
        l[i]->inverted = 1;
    }
}

void sub(elem_t *l[], u64 c){
    int i;
    for(i=0;i<PER_LIST && l[i]!=NULL;i++){
        l[i]->x -= c;
        l[i]->low = low(l[i]->x);
    }
}

void step_idx(u64 *idx, int *j, int *k){
    *k = *idx & 0xFF;
    *idx >>= 8;
    *j = *idx & 0xFF;
    *idx >>= 8;
}

u64 rnd_idx(int n){
    u64 x;
    int i;
    x = 0L;
    for(i=n+LISTS-1;i>=n;i--){
        x <<= 8;
        x |= i & 0xFF;
        x <<= 8;
        x |= rand() & 0xFF;
    }
    return x;
}

u64 idx2sum(u64 idx, u64 seeds[CLASSES][N]){
    int i,j,k;
    u64 s;
    s = 0L;
    assert((1<<LG_N) <= 0x100);
    for(i=0;i<LISTS;i++){
        step_idx(&idx, &j, &k);
        s += seeds[j][k];
    }
    return s;
}

void distribute(elem_t *l[], int n, node_t *buckets[]){
    int i;
    u32 key;
    node_t *head, *new_head, *huge;
    
    huge = (node_t*)malloc(n*sizeof(node_t)); //LEAK

    for(i=0;i<n;i++){
        key = l[i]->low;
        head = buckets[key];
        new_head = huge+i;
        assert(new_head != NULL);
        new_head->next = head; //first head is NULL
        new_head->e = l[i];
        buckets[key] = new_head;
    }
}

int merge(node_t *bu0[], node_t *bu1[], elem_t *lo[]){
    node_t *hd0, *hd1, *tmp;
    elem_t *ptr, *huge;
    int i,j,k;
    i=j=k=0;
    
    huge = malloc(PER_LIST*sizeof(elem_t)); //LEAK

    for(i=0;i<N_BUCKETS;i++){
        hd0 = bu0[i];
        if(hd0 == NULL)
            continue;
        hd1 = bu1[i];
        if(hd1 == NULL)
            continue;
        while(hd0){
            tmp = hd1;
            while(tmp){
                ptr = huge+k;
                ptr->x = hd0->e->x + (-tmp->e->x);
                ptr->low = low(ptr->x);
                ptr->c1 = hd0->e;
                ptr->c2 = tmp->e;
                lo[k] = ptr;
                k++;
                if(k == PER_LIST){
                    goto bail;
                }
                tmp = tmp->next;
            }
            hd0 = hd0->next;
        }
    }
bail:
    return k;
}

int walk(elem_t *e, result_t tab[]){
    int sz, i, j, k;
    u64 idx;
    if(e->c1 == NULL && e->c2 == NULL){
        idx = e->idx;
        for(i=0;i<LISTS;i++){
            step_idx(&idx, &j, &k);
            tab[i].seed = seeds[j][k];
            tab[i].ri = j;
            tab[i].k = k;
        }

        return 4;
    }
    else{
        sz = walk(e->c1, tab);
        sz += walk(e->c2, &tab[sz]);
    }
    return sz;
}

void hash(int key_type, char *msg, int msg_len, u64 *res){
    char *ptr;

    if(key_type == KEY_NAME){
        ptr = "key for name\x00\x00\x00\x00";
    }
    else if(key_type == KEY_SERIAL){
        ptr = "key for serial\x00\x00";
    }
    else{
        assert(0);
    }
    siphash24((uint8_t*)ptr, msg, msg_len, (uint8_t*)res);
}

void dump(elem_t *l[], int n){
    int i;
    for(i=0;i<n;i++){
        printf("%d, %llx\n", i, l[i]->x);
    }
}

void shr(elem_t *l[], int n, int r){
    int i;
    for(i=0;i<n;i++){
        l[i]->x = l[i]->x >> r; //arithmetic shift
        l[i]->low = low(l[i]->x);
    }
}


void fill_seeds(u64 seeds[CLASSES][N]){
    u64 idx, x;
    int i,j;

    for(i=0;i<CLASSES;i++){
        for(j=0;j<N;j++){
            idx = (i<<8) | j;
            hash(KEY_SERIAL, (char*)&idx, 8, &x);
            seeds[i][j] = x;
        }
    }
}

void fill_lists(elem_t *lists[][PER_LIST]){
    int i,j;
    elem_t *ptr, *huge;
    u64 x, idx;
    
    huge = (elem_t*)malloc(LISTS*PER_LIST*sizeof(elem_t)); //LEAK
    assert(huge != NULL);

    idx = 0L;
    for(i=0;i<LISTS;i++){
        for(j=0;j<PER_LIST;j++){
            ptr = huge+i*PER_LIST+j;
            idx = rnd_idx(i*LISTS);
            //printf("rnd idx=%llx\n", idx);
            x = idx2sum(idx, seeds);
            ptr->x = x;
            ptr->low = low(x);
            ptr->c1 = NULL;
            ptr->c2 = NULL;
            ptr->idx = idx;
            ptr->inverted = 0;
            lists[i][j] = ptr;
        }
    }
}

int main(int argc, char *argv[]){
    int i, j, k, sz, sz01, sz23, sz_fin, ri, name_len;
    elem_t *e;
    result_t sol[16];
    u64 s, name_hash;
    char serial[32+1];

    if(argc<2){
        printf("Keygen for keygenme3 by Dcoder\nUsage: %s <name>\n", 
                argv[0]);
        return 1;
    }

    name_len = strlen(argv[1]);
    hash(KEY_NAME, argv[1], name_len, &name_hash);

    memset(lists, 0, (LISTS+3)*PER_LIST*sizeof(elem_t*));
    memset(buckets, 0, (LISTS+3)*PER_LIST*sizeof(node_t*));
    
    fill_seeds(seeds);

    srand(0);
    fill_lists(lists);
    sub(lists[3], name_hash); //x1+..+(xn-c)=0 -> x1+..+xn=c

    //l0,-l1 -- l2,-l3
    inv(lists[1]);
    inv(lists[3]);

    for(i=0;i<LISTS;i++){
        distribute(lists[i], PER_LIST, buckets[i]);
    }

    sz01 = merge(buckets[0], buckets[1], lists[4]);
    sz23 = merge(buckets[2], buckets[3], lists[5]);
    //l01,-l23
    inv(lists[5]);
    shr(lists[4], sz01, L);
    shr(lists[5], sz23, L);
    distribute(lists[4], sz01, buckets[4]);
    distribute(lists[5], sz23, buckets[5]);
    sz_fin = merge(buckets[4], buckets[5], lists[6]);
    for(i=0;i<sz_fin;i++){
        e = lists[6][i];
        // ">>" is arithmetic shift for u64, so we can check for 0
        // instead of a certain power of 2
        if(e->x==0){
            //printf("i=%d, x=%llx\n", i, e->x);
            sz = walk(e, sol);
            assert(sz == 16);
            s = 0L;
            for(j=0;j<sz;j++){
                s += sol[j].seed;
                ri = sol[j].ri;
                k = sol[j].k;
                assert(ri == j);
                //printf("j=%d, ri=%d, k=%02x, s=%llx\n", j, ri, k, s);
                sprintf(&serial[j*2], "%02x", k);
            }
            assert(s == name_hash);
            serial[32]=0;
            printf("%s\n", serial);
        }
    }
    
    return 0;
}

