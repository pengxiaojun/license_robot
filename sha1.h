/* #define LITTLE_ENDIAN * This should be #define'd already, if true. */
/* #define SHA1HANDSOFF * Copies data before messing with it. */

#ifndef __GR_SHA1_H__
#define __GR_SHA1_H__

#define SHA1HANDSOFF

#include <stdio.h>
#include <string.h>
#include <sys/types.h>	/* for u_int*_t */
#include <stdint.h>

/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(uint32_t state[5], const unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, const unsigned char* data, uint32_t len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

#ifdef __cplusplus
}
#endif

#endif /*__GR_SHA1_H__*/


/* sample of usage
#define BUFSIZE 4096

int
main(int argc, char **argv)
{
    SHA1_CTX ctx;
    unsigned char hash[20], buf[BUFSIZE];
    int i;

    for(i=0;i<BUFSIZE;i++)
        buf[i] = i;

    SHA1Init(&ctx);
    for(i=0;i<1000;i++)
        SHA1Update(&ctx, buf, BUFSIZE);
    SHA1Final(hash, &ctx);

    printf("SHA1=");
    for(i=0;i<20;i++)
        printf("%02x", hash[i]);
    printf("\n");
    return 0;
}
*/

