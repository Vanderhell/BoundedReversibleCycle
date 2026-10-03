#include "brc.h"
#include <stdio.h>
int main(void)
{
    uint32_t n,seed;
    for(n=1u;n<=4096u;++n) for(seed=0u;seed<2u;++seed) {
        brc32_ctx_t c; uint8_t visited[4096]={0}; uint32_t x=0u,k;
        if(!brc32_init_v1(&c,n,seed)) return 1;
        for(k=0u;k<n;++k) { if(x>=n||visited[x]!=0u) { fprintf(stderr,"cycle N=%u seed=%u step=%u x=%u\n",n,seed,k,x);return 1; } visited[x]=1u;x=brc32_forward_v1(x,&c); }
        if(x!=0u) { fprintf(stderr,"cycle closure N=%u seed=%u actual=%u\n",n,seed,x);return 1; }
    }
    puts("full-cycle uniqueness and closure N=1..4096, two seeds: PASS"); return 0;
}
