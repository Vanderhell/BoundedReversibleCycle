#include "brc.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "frozen_vectors.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static uint8_t seen[4096]; static uint64_t cases; static uint32_t rng=UINT32_C(0x9E3779B9);
static uint32_t next32(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static uint32_t gcd32(uint32_t a,uint32_t b){while(b!=0u){uint32_t r=a%b;a=b;b=r;}return a;}
static void fail(const char*t,uint32_t n,uint32_t d,uint32_t x,uint32_t step,uint32_t len){printf("BRC_HW_TEST_FAIL test=%s N=%" PRIu32 " delta=%" PRIu32 " x=%" PRIu32 " step=%" PRIu32 " length=%" PRIu32 " cases=%" PRIu64 "\n",t,n,d,x,step,len,cases);abort();}
static void cycle(uint32_t n,uint32_t delta,uint32_t start)
{
    brc32_ctx_t c;uint32_t x=start,k,expected=n/gcd32(n,delta);memset(seen,0,sizeof seen);
    if(!brc32_init_with_delta_v1(&c,n,delta,UINT32_C(0x13579BDF)))fail("init",n,delta,start,0,0);
    for(k=0;k<expected;++k){uint32_t y;if(x>=n||seen[x]!=0u)fail("repeat",n,delta,x,k,k);seen[x]=1u;y=brc32_forward_v1(x,&c);if(y>=n||brc32_inverse_v1(y,&c)!=x||brc32_forward_v1(brc32_inverse_v1(x,&c),&c)!=x)fail("inverse",n,delta,x,k,expected);x=y;++cases;}
    if(x!=start)fail("cycle_close",n,delta,x,expected,expected);
    for(k=0;k<n;++k)if(seen[k]!=0u){uint32_t z=brc32_forward_v1(k,&c);if(brc32_inverse_v1(z,&c)!=k)fail("state_inverse",n,delta,k,0,expected);}
}
static void large_samples(void)
{
    static const uint32_t ns[]={65535u,65536u,65537u,1000000u,UINT32_MAX};unsigned i;uint32_t j;
    for(i=0;i<sizeof ns/sizeof ns[0];++i){brc32_ctx_t c;if(!brc32_init_v1(&c,ns[i],UINT32_C(0xFFFFFFFF)))fail("large_init",ns[i],0,0,0,0);rng=UINT32_C(0xC001D00D);for(j=0;j<100000u;++j){uint32_t x=next32()%ns[i],y=brc32_forward_v1(x,&c);if(y>=ns[i]||brc32_inverse_v1(y,&c)!=x||brc32_forward_v1(brc32_inverse_v1(x,&c),&c)!=x)fail("large_sample",ns[i],c.delta,x,y,0);++cases;}}
}
static void frozen_vectors(void)
{
    unsigned i;
    for(i=0u;i<BRC_HW_VECTOR_COUNT;++i){const brc_hw_vector_t*v=&brc_hw_vectors[i];if(v->is32!=0u){brc32_ctx_t c;if(!brc32_init_with_delta_v1(&c,v->n,v->delta,v->seed)||c.delta!=v->delta||brc32_forward_v1(v->x,&c)!=v->y||brc32_inverse_v1(v->y,&c)!=v->z)fail("frozen32",v->n,v->delta,v->x,v->y,0);}
        else {brc16_ctx_t c;if(v->n>UINT16_MAX||v->delta>UINT16_MAX||v->x>UINT16_MAX||v->y>UINT16_MAX||v->z>UINT16_MAX||!brc16_init_with_delta_v1(&c,(uint16_t)v->n,(uint16_t)v->delta,v->seed)||c.delta!=(uint16_t)v->delta||brc16_forward_v1((uint16_t)v->x,&c)!=(uint16_t)v->y||brc16_inverse_v1((uint16_t)v->y,&c)!=(uint16_t)v->z)fail("frozen16",v->n,v->delta,v->x,v->y,0);}++cases;
    }
}
static void generated_domain(uint32_t n,uint32_t seed)
{
    brc32_ctx_t c32; brc16_ctx_t c16; uint32_t x32=0,k; uint16_t x16=0;
    if(!brc32_init_v1(&c32,n,seed)||gcd32(n,c32.delta)!=1u)fail("generated_init",n,0,0,0,0);
    memset(seen,0,sizeof seen);
    for(k=0;k<n;++k){uint32_t y;if(seen[x32]!=0u)fail("generated_repeat",n,c32.delta,x32,k,k);seen[x32]=1u;y=brc32_forward_v1(x32,&c32);if(y>=n||brc32_inverse_v1(y,&c32)!=x32)fail("generated_inverse",n,c32.delta,x32,k,n);x32=y;++cases;if((k&2047u)==2047u)vTaskDelay(1);}
    if(x32!=0u)fail("generated_close",n,c32.delta,x32,n,n);
    if(!brc16_init_v1(&c16,(uint16_t)n,seed)||gcd32(n,c16.delta)!=1u)fail("generated16_init",n,0,0,0,0);
    memset(seen,0,sizeof seen);
    for(k=0;k<n;++k){uint16_t y;if(seen[x16]!=0u)fail("generated16_repeat",n,c16.delta,x16,k,k);seen[x16]=1u;y=brc16_forward_v1(x16,&c16);if(y>=n||brc16_inverse_v1(y,&c16)!=x16)fail("generated16_inverse",n,c16.delta,x16,k,n);x16=y;++cases;if((k&2047u)==2047u)vTaskDelay(1);}
    if(x16!=0u)fail("generated16_close",n,c16.delta,x16,n,n);
}
void app_main(void)
{
    static const uint32_t seeds[]={0u,UINT32_C(0x13579BDF)};static const uint32_t points[]={2u,3u,17u,31u,32u,33u,255u,256u,257u,1024u,4095u,4096u};unsigned s,i,run;uint32_t n;
    printf("BRC_HW_READY target=ESP32-S3 optimization=%s\n",HW_OPT_LABEL);
    for(run=0;run<HW_RUN_COUNT;++run){cases=0u;
        for(s=0;s<2u;++s)for(n=1;n<=4096u;++n){generated_domain(n,seeds[s]);if((n&1023u)==0u)printf("BRC_PROGRESS run=%u seed=%u N=%" PRIu32 " cases=%" PRIu64 "\n",run+1u,s,n,cases);}
        for(i=0;i<sizeof points/sizeof points[0];++i){uint32_t m;for(m=0;m<5u;++m)cycle(points[i],1u,m%points[i]);}
        cycle(12u,5u,0u);cycle(12u,7u,4u);cycle(12u,11u,11u);cycle(12u,4u,2u);
        cycle(30u,6u,0u);cycle(60u,15u,13u);cycle(100u,25u,99u);large_samples();frozen_vectors();
        printf("BRC_HW_TEST_PASS run=%u cases=%" PRIu64 " mismatches=0 free_heap=%u stack_high_water_words=%u\n",run+1u,cases,(unsigned)esp_get_free_heap_size(),(unsigned)uxTaskGetStackHighWaterMark(NULL));
    }
    vTaskDelay(pdMS_TO_TICKS(3000));esp_restart();
}
