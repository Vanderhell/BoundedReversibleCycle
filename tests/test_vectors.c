#include "brc.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *field(char **cursor)
{
    char *start=*cursor,*comma=strchr(start,',');
    if(comma!=NULL){*comma='\0';*cursor=comma+1;}else *cursor=start+strlen(start);
    return start;
}
static int number(char *text,uint32_t *value)
{
    char *end=NULL;uintmax_t n=strtoumax(text,&end,0);
    if(end==text||*end!='\0'||n>UINT32_MAX)return 0;
    *value=(uint32_t)n;return 1;
}
int main(void)
{
    FILE *f=fopen("vectors_v1.csv","r");char line[256];unsigned count=0u;
    if(f==NULL){fputs("cannot open BRC frozen vectors\n",stderr);return 1;}
    if(fgets(line,sizeof(line),f)==NULL){fclose(f);return 1;}
    while(fgets(line,sizeof(line),f)!=NULL){char *cursor=line,*version;uint32_t v[6];size_t i;line[strcspn(line,"\r\n")]='\0';version=field(&cursor);
        for(i=0u;i<6u;++i)if(!number(field(&cursor),&v[i])){fclose(f);fprintf(stderr,"malformed BRC vector row=%u\n",count+1u);return 1;}
        if(strcmp(version,"BRC32-V1")==0){brc32_ctx_t c;if(!brc32_init_with_delta_v1(&c,v[0],v[2],v[1])||c.delta!=v[2]||brc32_forward_v1(v[3],&c)!=v[4]||brc32_inverse_v1(v[4],&c)!=v[5]){fclose(f);fprintf(stderr,"BRC32 frozen row=%u N=%" PRIu32 " delta=%" PRIu32 " x=%" PRIu32 "\n",count+1u,v[0],v[2],v[3]);return 1;}}
        else if(strcmp(version,"BRC16-V1")==0){brc16_ctx_t c;if(v[0]>UINT16_MAX||v[2]>UINT16_MAX||v[3]>UINT16_MAX||v[4]>UINT16_MAX||v[5]>UINT16_MAX||!brc16_init_with_delta_v1(&c,(uint16_t)v[0],(uint16_t)v[2],v[1])||c.delta!=(uint16_t)v[2]||brc16_forward_v1((uint16_t)v[3],&c)!=(uint16_t)v[4]||brc16_inverse_v1((uint16_t)v[4],&c)!=(uint16_t)v[5]){fclose(f);fprintf(stderr,"BRC16 frozen row=%u N=%" PRIu32 " delta=%" PRIu32 " x=%" PRIu32 "\n",count+1u,v[0],v[2],v[3]);return 1;}}
        else {fclose(f);fprintf(stderr,"unknown BRC version row=%u\n",count+1u);return 1;}
        ++count;
    }
    if(ferror(f)||fclose(f)!=0||count!=480u){fprintf(stderr,"BRC frozen vector count=%u expected=480\n",count);return 1;}
    puts("all 480 frozen BRC16/32-V1 compatibility vectors: PASS");return 0;
}
