#ifndef SKRAND_INCLUDE
#define SKRAND_INCLUDE 1

#ifndef Skein_512_Ctxt_t
#include "skein.h"
#endif

typedef struct {
   Skein_512_Ctxt_t * ctx;
   unsigned char buf[64];
   int remain;
} skrand_t;

/* Initialize an instance, with optional seed-loading */
skrand_t * skrand_init(char *filename);

/* Free an instance, with optional seed-saving for next run */
void skrand_free(skrand_t *skrand, char *filename);

/* Read len random 8-bit values into buffer */
void skrand_read(skrand_t *skrand, char *buf, int len);

/* Add len bytes of entropy to the system */
void skrand_stir(skrand_t *skrand, char *buf, int len);

/* Return a number from 0 to n-1 */
unsigned int skrand_int(skrand_t *skrand, unsigned int n);

#endif
