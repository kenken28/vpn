/*****************************************************************************
 * pgenp - Generate random primes using probabilistic methods.
 * Aaron Logue 2013
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "bigint.h"
#ifdef WIN32
#include <windows.h>
#include "skrand.h"
#endif
#include "getopt2.h"

#define ADD_ENTROPY 1

#ifdef WIN32
skrand_t *skrand;
#else
unsigned int fdrand;
#endif

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
 * write_prime()
 ****************************************************************************/
void write_prime(char *fprimes, bigint_t *p, int outbase) {
 FILE *fp;
 char *sptr;
 char buf[80];

   if (fprimes && *fprimes) {
      fp = fopen(fprimes, "a");
      if (fp) {
         sptr = bigint_itoa(p, outbase);
         fwrite(sptr, 1, strlen(sptr), fp);
         free(sptr);
         strcpy(buf, "\n");
         fwrite(buf, 1, strlen(buf), fp);
         fclose(fp);
      }
   }
   return;
}

/****************************************************************************
 * bigint_random()
 * This function reads and writes a pseudorandom generator seed file each
 * time it's called. On Windows, you may want to implement a faster version
 * of this by calling skrand_init() at the beginning of your program and
 * leaving the pseudorandom stream open through the run to avoid reading
 * and rewriting skrand.bin on every call.
 ***************************************************************************/
void my_bigint_random(bigint_t * var, unsigned int bits) {
 unsigned int i, elements;

   elements = bits >> SHIFTBITS;
   if (bits & (INTBITS-1)) {
      elements++;
   }
   if (elements > var->elements) {
      bigint_extend(var, elements - var->elements);
   }
#ifdef WIN32
   if (skrand) {
      for (i=0; i<elements; i++) {
         skrand_read(skrand, (char *)&var->b[i], sizeof(int_type));
      }
   }
#else
   if (fdrand >= 0) {
      for (i=0; i<elements; i++) {
         read(fdrand, &var->b[i], sizeof(int_type));
      }
   }
#endif
   i = bits % INTBITS;
   if (i) {
      var->b[elements - 1] >>= INTBITS - i;
   }
   while (elements < var->elements) {
      var->b[elements] = 0;
      elements++;
   }
   return;
}

#ifdef WIN32
/*****************************************************************************
 * get_gibberish()
 ****************************************************************************/
void get_gibberish(char *passphrase, int len) {
 int i, c;
   i = 0;
   while (i < len) {
      c = getchar();
      if (c == '\n') {
         break;
      }
      passphrase[i] = c;
      i++;
   }
   passphrase[i] = 0;
   return;
}
#endif

/*****************************************************************************
 * show_usage()
 ****************************************************************************/
void show_usage(char *me) {
   printf("Usage: %s -b <bits> [outfile] [-n <num>] [-o <base>]\n", me);
#ifdef WIN32
   printf("or     %s -e\n", me);
#endif
   printf("\n");
   printf("\t-b\tGenerate primes of given width (64,80,128,256,etc)\n");
   printf("\t-n\tGenerate this many primes (default is 1)\n");
   printf("\t-o\tUse this number base for output (including file)\n");
#ifdef WIN32
   printf("\t-e\tStir RNG entropy pool with keyboard input (do at least once!)\n");
#endif
   printf("\n");
   printf("Examples:\n");
   printf("%s -b 1024 rm1024.txt  Write a 1024-bit prime to rm1024.txt\n", me);
   printf("%s -b 64 -n 3 -o 2     Display 3 random 64-bit primes in binary\n", me);
   printf("\n");
   return;
}

/*****************************************************************************
 * pgenp main()
 ****************************************************************************/
int main(int argc, char *argv[]) {
 unsigned int bits, max_primes, flags, outbase;
 char c;
 bigint_t *prime;
 char *fprimes;
 char temp[10];
 int certainty;

   fprimes    = NULL;
   bits       = 0;
   max_primes = 1;
   outbase    = 10;
   flags      = 0;
   certainty  = -1;

   while ((c = getopt2(argc,argv,"eb:n:o:c:")) > 0) {
      switch(c) {
       case 'e':
         flags |= ADD_ENTROPY;
         break;
       case 'b':
         bits = atoi(optarg2);
         if (bits < 2) {
            bits = 2;
         }
         break;
       case 'c':
         certainty = atoi(optarg2);
         if (certainty < 0) {
            certainty = 0;
         }
         break;
       case 'n':
         max_primes = atoi(optarg2);
         if (max_primes < 1) {
            max_primes = 1;
         }
         break;
       case 'o':
         outbase = atoi(optarg2);
         if (outbase < 2) {
            outbase = 2;
         }
         if (outbase > 16) {
            outbase = 16;
         }
         break;
       case '?':
       default:
         show_usage(argv[0]);
         return 0;
      }
   }

#ifdef WIN32
   /* Perturb the random number generator? */
   if (flags & ADD_ENTROPY) {
    char buf[81];
      printf("Type up to 80 chars of gibberish, Enter when done\n");
      get_gibberish(buf, 80);
      skrand = skrand_init("skrand.bin");
      if (skrand) {
         skrand_stir(skrand, buf, strlen(buf));
         skrand_free(skrand, "skrand.bin");
      }
      return 0;
   }
#endif
   if (bits == 0) {
      show_usage(argv[0]);
      return 0;
   }

   /* Open up global instance of platform RNG so as not to thrash opens */
#ifdef WIN32
   skrand = skrand_init("skrand.bin");
   /* Stir hash state to be sure multiple instances diverge */
   sprintf(temp, "%d", GetCurrentProcessId());
   skrand_stir(skrand, temp, strlen(temp));
#else
   fdrand = open("/dev/urandom", O_RDONLY);
#endif

   if (certainty == -1) {
      certainty = 7;       /* 7 is sufficient to prove 48-bit primes */
      if (bits > 48) {
         certainty += 50;  /* I pulled an additional 50 tests out of a hat */
      }
   }
   prime = bigint_init(32, 0);
   while (max_primes--) {
      do {
         my_bigint_random(prime, bits);
         SET_BIT(prime->b, 0);
         SET_BIT(prime->b, bits - 1);
      } while (!bigint_is_prime(prime, certainty));
      bigint_print(prime, outbase);
      printf("\n");
      if (fprimes) {
         write_prime(fprimes, prime, outbase);
      }
#ifdef WIN32
      skrand_free(skrand, "skrand.bin");
      skrand = skrand_init("skrand.bin");
#endif
   }

#ifdef WIN32
   skrand_free(skrand, "skrand.bin");
#else
   close(fdrand);
#endif
   bigint_free(prime);
   return 1;
}
