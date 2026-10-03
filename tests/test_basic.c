#include "brc.h"
#include <stdio.h>
int main(void)
{
    uint32_t n,seed; const uint32_t seeds[]={0u,1u,UINT32_MAX};
    for(n=1u;n<=1000u;++n) for(seed=0u;seed<3u;++seed) {
        brc32_ctx_t c; uint32_t points[3],i;
        if(!brc32_init_v1(&c,n,seeds[seed])) return 1;
        points[0]=0u;points[1]=n/2u;points[2]=n-1u;
        for(i=0u;i<3u;++i) { uint32_t y=brc32_forward_v1(points[i],&c); if(y>=n||brc32_inverse_v1(y,&c)!=points[i]||brc32_forward_v1(brc32_inverse_v1(points[i],&c),&c)!=points[i]) { fprintf(stderr,"inverse N=%u seed=%u x=%u y=%u\n",n,seeds[seed],points[i],y);return 1; } }
    }
    { brc32_ctx_t c; if(!brc32_init_with_delta_v1(&c,12u,5u,0u)||brc32_init_with_delta_v1(&c,12u,0u,0u)||brc32_init_with_delta_v1(&c,12u,12u,0u)||brc32_init_with_delta_v1(NULL,1u,0u,0u)||!brc32_init_with_delta_v1(&c,1u,0u,0u)) return 1; }
    { static const uint32_t ds[]={1u,11u,5u,7u}; size_t q; for(q=0u;q<sizeof(ds)/sizeof(ds[0]);++q){brc32_ctx_t c;uint32_t x=0u,k;if(!brc32_init_with_delta_v1(&c,12u,ds[q],7u))return 1;for(k=0u;k<12u;++k)x=brc32_forward_v1(x,&c);if(x!=0u)return 1;} }
    puts("inverse, generated coprime steps, invalid delta: PASS"); return 0;
}
