/*****************************************************************************
 * proots - Compute primitive root generators using prime proof files
 * Aaron Logue 2013
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "bigint.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "getopt2.h"

#define USE_PROOFS  1

/*****************************************************************************
 * long_pow_mod()
 ****************************************************************************/
int2x_type long_pow_mod(int2x_type base, int2x_type exponent,
                        int2x_type modulus) {
 int2x_type result = 1;
    while (exponent) {
        if (exponent & 1) {
           result = result * base % modulus;
        }
        exponent = exponent >> 1;
        base = base * base % modulus;
    }
    return result;
}

/*****************************************************************************
 * rabin_miller()
 ****************************************************************************/
int rabin_miller(int a, int2x_type n) {
 unsigned int s, i;
 int2x_type d, b;

   s = 0;
   d = n - 1;
   while ((d & 1) == 0) {
       d = d >> 1;
       s++;
   }

   b = long_pow_mod(a, d, n);
   if (b == 1) {
      return 1;
   }
   for (i=0; i < s-1; i++) {
       if (b == (n-1)) {
          return 1;
       }
       b = long_pow_mod(b, 2, n);
   }
   if (b == (n-1)) {
      return 1;
   }
   return 0;
}

/*****************************************************************************
 * is_prime()
 * 32 bit primes are proven with 2, 7, and 61
 * https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test
 * if n < 1,373,653, it is enough to test a = 2 and 3;
 * if n < 9,080,191, it is enough to test a = 31 and 73;
 * if n < 4,759,123,141, it is enough to test a = 2, 7, and 61;
 * if n < 1,122,004,669,633, it is enough to test a = 2,13,23, and 1662803;
 * if n < 2,152,302,898,747, it is enough to test a = 2,3,5,7, and 11;
 * if n < 3,474,749,660,383, it is enough to test a = 2,3,5,7,11, and 13;
 * if n < 341,550,071,728,321, it is enough to test a = 2,3,5,7,11,13, and 17.
 * According to that last one, 48 bit primes can be proven with 7 tests.
 ****************************************************************************/
int is_prime(int2x_type num) {
   if (num == 2) {
      return 1;
   }
   if (num <= 1 || (num & 1) == 0) {
      return 0;
   }
   if (rabin_miller(2, num) == 1 &&
      (num <= 7 || rabin_miller(7, num) == 1) &&
      (num <= 61 || rabin_miller(61, num) == 1)) {
      return 1;
   }
   return 0;
}

/*****************************************************************************
 * brute_force_roots()
 * Find generators for a given small prime.
 * This performs an exhaustive search of base^x mod prime and checks to
 * see if we are able to try all x from 2 to prime-1 before hitting base.
 * It's too slow for primes > 10M or so but came in handy to verify the
 * proven prime construction and primitive root finding by Fermat's Little
 * Theorem functions.
 ****************************************************************************/
int brute_force_roots(int_type prime, int start, unsigned int max) {
 unsigned int i, base;
 int2x_type x, X;

   if (is_prime(prime)) {
      /* hunt for first n generators of 1 to p-1 */
      i = 1;
      base = start;
      while (base < prime) {
         for (x=2; x<prime; x++) {
            X = long_pow_mod(base, x, prime);
            if (X == base) {
               break;
            }
         }
         if (x == prime) {
            printf("%d ", base);
            fflush(0);
            if (i >= max) {
               break;
            }
            i++;
         }
         base++;
      }
      printf("\n");
   } else {
      printf("%d does not appear to be prime.\n", prime);
   }
   return 0;
}

/*****************************************************************************
 * Case insensitive memory compare, requiring complete match.
 * If successful, returns pointer to next character in buffer.
 ****************************************************************************/
char * strmatch(unsigned char * buf, unsigned char * str) {
   while (*str) {
      if (tolower(*buf) != tolower(*str)) {
         return(NULL);   /* Mismatch before end of input string */
      }
      buf++;
      str++;
   }
   return(buf);                    /* Input string found at buf */
}

/*****************************************************************************
 * find_primitive_roots()
 * Using primes proof file, read the prime that begins with what is passed
 * in rootptr, read its factors, check that all factors are prime, then find
 * up to max roots beginning at start.
 ****************************************************************************/
void find_primitive_roots(char *rootptr, int start, int max, char *fproofs) {
 FILE *fp;
 char *buf, *sptr, *dptr, *rc;
 int i, j, count, primitive, notprime;
 bigint_t *prime, *n_1, *g, *X;
 bigint_t **e;
 int base = 10;

   count = 0;
   buf = (char *)malloc(50000);
   if (!buf) {
      printf("Couldn't allocate memory to read proofs file.\n");
      return;
   }
   fp = fopen(fproofs, "r");
   if (!fp) {
      printf("File %s not found.\n", fproofs);
      return;
   }
   prime = bigint_init(32, 0);
   n_1   = bigint_init(32, 0);
   g     = bigint_init(32, 0);
   X     = bigint_init(32, 0);
   do {
      rc = fgets(buf, 49999, fp);
      if (rc && strmatch(buf, rootptr)) {
         /* Count the factors */
         count = 0;
         sptr = buf;
         while (*sptr) {
            if (*sptr == ',') {
               count++;
            }
            sptr++;
         }
         /* Test for hex in the row. If any, switch input base to hex */
         sptr = buf;
         while (*sptr) {
            if (strchr("ABCDEFabcdef", *sptr)) {
               base = 16;
               break;
            }
            sptr++;
         }

         /* extract prime */
         dptr = sptr = buf;
         while (*dptr && *dptr != ',') {
            dptr++;
         }
         if (*dptr == ',') {
            *dptr++ = 0;
            bigint_atoi(prime, sptr, base);
            bigint_copy(n_1, prime);
            bigint_subtract_int(n_1, 1);
            sptr = dptr;

            printf("Match found. Verifying primality of factors of p-1...\n");
            /* allocate space for and extract factors of n_1 */
            notprime = 0;
            e = (bigint_t **)malloc(sizeof(bigint_t *) * count);
            for (i=0; i<count; i++) {
               e[i] = bigint_init(32, 0);
               while (*dptr++) {
                  if (*dptr == ',') {
                     *dptr = 0;
                  }
               }
               bigint_atoi(e[i], sptr, base);
               if (!bigint_is_prime(e[i], 27)) {
                  notprime = i+1;
               }
               sptr = dptr;
            }
            if (notprime) {
               printf("Skipping ");
               bigint_print(prime, base);
               printf(", ");
               bigint_print(e[notprime-1], base);
               printf(" is composite.\n");
            } else {
               printf("Looking for %d primitive root%s >= %d of %d-bit prime:\n",
                  max, max>1?"s":"", start, bigint_width(prime));
               bigint_print(prime, base);
               printf("\n");
               printf( "Root g is primitive if g ^ ((p-1)/f) mod p <> 1 for all factors f of p-1\n");
               /* Calculate exponents */
               for (i=0; i<count; i++) {
                  bigint_copy(X, e[i]);
                  bigint_divide(n_1, X, e[i]);
               }
               /* Produce up to max generators from start */
               bigint_set(g, start);
               j = 0;
               while (j < max && bigint_compare(g, n_1) < 0) {
                  primitive = 1;
                  for (i=0; i<count; i++) {
                     /* Does a ^ ((n-1)/f) mod prime <> 1 for all factors? */
                     bigint_pow_mod(g, e[i], prime, X);
                     if (bigint_compare_int(X, 1) == 0) {
                        /* g is not a primitive root */
                        primitive = 0;
                     }
                  }
                  if (primitive) {
                     bigint_print(g, 10);
                     printf(" ");
                     fflush(0);
                     j++;
                  }
                  /* Advance to next generator candidate */
                  bigint_add_int(g, 1, g);
               }
               printf("\n");
            }
            /* Free exponents */
            for (i=0; i<count; i++) {
               bigint_free(e[i]);
            }
            free(e);
         }
      }
   } while (rc);

   if (!count) {
      printf("%s not found in proofs file.\n", rootptr);
   }
   bigint_free(prime);
   bigint_free(n_1);
   bigint_free(g);
   bigint_free(X);
   fclose(fp);
   free(buf);
}

/*****************************************************************************
 * show_usage()
 ****************************************************************************/
void show_usage(char *me) {
   printf("Usage: %s -r <prime> [-n <num>] [-s <start>] [-c] [<file>]\n", me);
   printf("\n");
   printf("\t-r\tFind primitive roots of number beginning with this\n");
   printf("\t-n\tCompute this many roots (default is 1)\n");
   printf("\t-s\tSkip testing roots below this value\n");
   printf("\t-c\tNo proofs file; find roots of literal constant\n");
   printf("\n");
   printf("Examples:\n");
   printf("%s -r 4203 -n 10      Search primes.proof for '4203...'\n", me);
   printf("%s proofs.002 -r 2824 Override name of proofs file to search in\n", me);
   printf("%s -r b7f23 -s 47     See if 47 is a generator for 'b7f23...'\n", me);
   printf("%s -r 97 -n 100 -c    Find all roots of the literal prime 97\n", me);
   printf("\n");
   printf(
"This program finds primitive roots (the 'g' in g^x mod p) of a prime number\n"
"with a row in a proofs file of the form p,f1,f2,f3 where p is the prime\n"
"number we're seeking roots of and f1,f2,f3 are the prime factors of p-1.\n"
"These rows are written by pgen as it constructs primes using Maurer's\n"
"algorithm. In order to find proven primitive roots, all 3 factors of p-1\n"
"must be prime (don't use -m in pgen when constructing the final prime.)\n"
"Primitive roots are desirable for use in Diffie-Hellman because they produce\n"
"a finite cyclic group of maximum size (1 to p-1) for a given prime p.\n" 
);    
   return;
}

/*****************************************************************************
 * proots main()
 ****************************************************************************/
int main(int argc, char *argv[]) {
 unsigned int start, num, flags;
 char c;
 char *fproofs;
 char *rootptr;

   fproofs   = "./primes.proof";
   start     = 2;
   num       = 1;
   flags     = USE_PROOFS;
   rootptr   = NULL;

   while ((c = getopt2(argc,argv,"cn:r:s:")) > 0) {
      switch(c) {
       case 'c':
         flags &= ~USE_PROOFS;
         break;
       case 'n':
         num = atoi(optarg2);
         if (num < 1) {
            num = 1;
         }
         break;
       case 'r':
         rootptr = optarg2;
         break;
       case 's':
         start = atoi(optarg2);
         if (start < 2) {
            start = 2;
         }
         break;
       case '*':
         fproofs = optarg2;
         flags |= USE_PROOFS;
         break;
       case '?':
       default:
         show_usage(argv[0]);
         return 0;
      }
   }
   if (!rootptr || !*rootptr) {
      show_usage(argv[0]);
      return 0;
   }

   if (flags & USE_PROOFS) {
      find_primitive_roots(rootptr, start, num, fproofs);
   } else {
      brute_force_roots(atoi(rootptr), start, num);
   }
   return 1;
}
