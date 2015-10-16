/*****************************************************************************
 * pgen - Generate primes using probabilistic or constructive methods.
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

#define BE_QUIET          1
#define MAKE_R_PRIME      2
#define USE_FPRIMES       4
#define USE_PROVEN_R      8
#define RESET_Q_SOONER   16
#define ADD_ENTROPY      32

bigint_t * composite;
bigint_t * maxprovable;  /* Highest prime provable with 7-parm RM */
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

/*****************************************************************************
 * write_proof()
 * Records n = 2 * q * r + 1 factors in CSV file in rows of prime,2,q,r
 ****************************************************************************/
void write_proof(char *fproofs, bigint_t *p, bigint_t *q, bigint_t *r,
                 int outbase) {
 FILE *fp;
 char *sptr;
 char buf[80];

   if (fproofs && *fproofs) {
      fp = fopen(fproofs, "a");
      if (fp) {
         sptr = bigint_itoa(p, outbase);
         fwrite(sptr, 1, strlen(sptr), fp);
         free(sptr);
         strcpy(buf, ",2,");
         fwrite(buf, 1, strlen(buf), fp);
         sptr = bigint_itoa(q, outbase);
         fwrite(sptr, 1, strlen(sptr), fp);
         free(sptr);
         strcpy(buf, ",");
         fwrite(buf, 1, strlen(buf), fp);
         sptr = bigint_itoa(r, outbase);
         fwrite(sptr, 1, strlen(sptr), fp);
         free(sptr);
         strcpy(buf, "\n");
         fwrite(buf, 1, strlen(buf), fp);
         fclose(fp);
      }
   }
   return;
}

/*****************************************************************************
 * random_prime()
 * This function generates random primes of up to 32 bits. Primes of shorter
 * bit width are allowed, but could easily be forced by setting the high bit
 * of num to 1. The primes generated are proven by miller-rabin 2, 7, 61.
 ****************************************************************************/
unsigned long random_prime(int bits) {
 unsigned long num = 4;  /* Stay in loop until num is prime */

   while (1) {
#ifdef WIN32
      if (skrand) {
         skrand_read(skrand, (char *)&num, (bits + 7) >> 3);
#else
      if (fdrand >= 0) {
         read(fdrand, &num, (bits + 7) >> 3);
#endif
         if (bits % 8) {
            num = num >> (8 - (bits % 8));
         }
         num |= 1;
         num |= (1 << (bits - 1));
      }
      if (rabin_miller(2, num) == 1 &&
         (num <= 7 || rabin_miller(7, num) == 1) &&
         (num <= 61 || rabin_miller(61, num) == 1)) {
         return num;
      }
   }
   return 0;
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

/****************************************************************************
 * bigint_random_n()
 * Returns a uniformly distributed random value from 0 to N-1
 ***************************************************************************/
void my_bigint_random_n(bigint_t * result, bigint_t * n) {
 bigint_t * cutoff, * modn;
 unsigned int bits;

   if (bigint_compare_int(n, 2) == -1) {
      bigint_set(result, 0);
      return;
   }
   cutoff = bigint_init(32, 1);
   modn   = bigint_init(32, 0);

   /* cutoff = 0xFFFFFFFFFFFFFFFF - (0xFFFFFFFFFFFFFFFF % n) - 1 */
   bits = (bigint_width(n) << 1) + 1;
   bigint_shift(cutoff, 0 - bits);   /* cutoff = 0x100000000        */
   bigint_subtract_int(cutoff, 1);   /* cutoff =  0xFFFFFFFF        */
   bigint_mod(cutoff, n, modn);      /* modn   =  0xFFFFFFFF % n    */
   bigint_subtract(cutoff, modn);    /* cutoff =  0xFFFFFFFF - modn */
   bigint_subtract_int(cutoff, 1);
   do {
      my_bigint_random(modn, bits);
   } while (bigint_compare(modn, cutoff) == 1);
   bigint_mod(modn, n, result);

   bigint_free(cutoff);
   bigint_free(modn);
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
 * read_fprimes()
 * Returns 1 if a value from rmm to 2rmm was found in primes file, otherwise 0.
 ****************************************************************************/
int read_fprimes(FILE *fp, bigint_t *rmm, bigint_t *r, int inbase) {
 char *buf, *sptr;
 bigint_t *candidate;
 int rc = 0;

   candidate = bigint_init(32, 0);
   buf = (char *)malloc(10000);
   if (buf) {
      rc = 1;
      while (rc) {
         if (fgets(buf, 9999, fp)) {
            if (inbase == 10) {
               /* Test for hex in the row. If any, switch input base to hex */
               sptr = buf;
               while (*sptr) {
                  if (strchr("ABCDEFabcdef", *sptr)) {
                     inbase = 16;
                     break;
                  }
                  sptr++;
               }
            }
            bigint_atoi(candidate, buf, inbase);
            if (bigint_compare(candidate, rmm) >= 0) {
               bigint_shift(rmm, -1);
               rc = bigint_compare(candidate, rmm);
               bigint_shift(rmm, 1);
               if (rc <= 0) {
                  rc = 1;
                  bigint_copy(r, candidate);
                  break;
               }
            }
         } else {
            rc = 0;
         }
      }
      free(buf);
   }
   bigint_free(candidate);
   return rc;
}

/*****************************************************************************
 * generate_proven_prime()
 * Generate a proven prime, writing factors of p-1 and generator to file.
 * Proof file may wind up with a bunch of smaller proven-but-unused primes
 * written to it.  n = 2 * q * p + 1 is prime if the following are true:
 *   q is a prime factor of n-1, and q > sqrt(n)-1
 *   a^{n-1} = 1 (mod n)            (pick any small prime base a)
 *   gcd( a^(2r) mod n - 1, n) = 1
 *
 * This function spends all of its time coming up with random prime r values
 * to test in the final MAKE_R_PRIME stretch. This can be inhibited with
 * -m which will make tests run considerably faster, but then -r will not
 * be able to find primitive roots because r will almost certainly be
 * composite. 
 *
 * One optimization is to search the primes file for all previously-
 * generated primes of the right size and test them before starting to
 * generate new ones.  Another would be to transmit the range of random r
 * as soon as it's known to a cluster of networked processors and have
 * them contribute primes to test with.
 *
 * If bits == 0, then min/max determines the range.
 ****************************************************************************/
void generate_proven_prime(bigint_t *n, unsigned int bits, bigint_t *minmax,
                           int flags, char *fprimes, int inbase,
                           char *fproofs, int outbase) {
 unsigned int i, have_proof, k;
 bigint_t *q, *r, *rmm, *n_1, *a, *X, *Y;
 FILE *fp;

   /* Don't return random prime on entry to allow constructing small prime */
   if (!(flags & MAKE_R_PRIME)) {
      if (bits) {
         if (bits <= 32) {
            bigint_set(n, 0);
            n->b[0] = random_prime(bits); /* proves up to 32 bits */
            return;
         }
         if (bits <= 48) {
            do {
               my_bigint_random(n, bits);
               SET_BIT(n->b, 0);
               SET_BIT(n->b, bits - 1);
            } while (!bigint_is_prime(n, 7)); /* 7 proves up to 48 bits */
            return;
         }
      } else {
         if (bigint_compare(minmax, maxprovable) < 0) {
            do {
               my_bigint_random_n(n, minmax);
               bigint_add(n, minmax, n);
            } while (!bigint_is_prime(n, 7)); /* 7 proves up to 48 bits */
            return;
         }
      }
   }

   q    = bigint_init(32, 0);
   r    = bigint_init(32, 0);
   rmm  = bigint_init(32, 0);
   n_1  = bigint_init(32, 0);
   a    = bigint_init(32, 2);
   X    = bigint_init(32, 0);
   Y    = bigint_init(32, 0);

   have_proof = 0;
   do {
     /* 
      * 1. Pick q such that q is guaranteed to be > square root of n
      *    k = (bits>>1)+1 + (bits & 1)
      * 2. Pick r such that n = 2 q r + 1 comes out to the correct width
      *    by computing the min and max values for r that would do it,
      *    then using bigint_random_n() to get a number from that range.
      */
      if (bits) {
         k = (bits >> 1) + 1 + (bits & 1);    /* Guarantee q >= sqrt(n) */

         generate_proven_prime(q, k, NULL, flags & BE_QUIET, NULL, 0,
                               fproofs, outbase);

         /* Calculate r range rmm to 2*rmm that will guarantee bits-1 bits */
         bigint_set(X, 1);
         bigint_shift(X, 0 - (int)(bits-2));
         bigint_divide(X, q, rmm);       /* rmm = 0x8000 / q */
      } else {
         /* We only get here if we try to generate large proven r below */
         bigint_sqrt(minmax, X);
         bigint_shift(X, -3);       /* increase the ratio */

         generate_proven_prime(q, 0, X, flags & BE_QUIET, NULL, 0,
                               fproofs, outbase);

         /* Calculate the rmm that will guarantee minmax/2 to minmax */
         bigint_divide(minmax, q, rmm);
         bigint_shift(rmm, 1);
      }

      /* TODO Distribute range requirements to coprocessor cluster */
      if (!(flags & BE_QUIET)) {
         printf("rmm: ");
         bigint_print(rmm , outbase);
         printf("\n");
      }

      if (flags & USE_FPRIMES) {
         fp = fopen(fprimes, "r");
         if (!fp) {
            flags &= ~USE_FPRIMES;
         }
      }

      i = 0;
      do {
         if (flags & MAKE_R_PRIME) {

            if (flags & USE_FPRIMES) {
               if (!read_fprimes(fp, rmm, r, inbase)) {
                  flags &= ~USE_FPRIMES;
                  fclose(fp);
                  if (flags & RESET_Q_SOONER) {
                     i = bits;
                     flags |= USE_FPRIMES; /* Reprocess primes file */
                     continue;
                  }
               }
            }
            if (!(flags & USE_FPRIMES)) {

               /* TODO Generate and test or read from cluster and break */

               if (flags & USE_PROVEN_R) {
                  /* This is faster, but may not even work */
                  generate_proven_prime(r, 0, rmm, BE_QUIET,
                                        NULL, 0, NULL, outbase);
               } else {
                  do {
                     my_bigint_random_n(r, rmm);
                     bigint_add(r, rmm, r);
                     SET_BIT(r->b, 0);
                  } while (!bigint_is_prime(r, 7)); /* postpone costlier test */
               }
               write_prime(fprimes, r, outbase);
            }
            if (!(flags & BE_QUIET)) {
               /* Indicate testing a prime r for final p = 2qr + 1 */
               if (flags & USE_FPRIMES) {
                  printf(".");
               } else {
                  printf("+"); /* Testing and added new prime to primes file */
               }
               fflush(0);
            }

         } else {
            /* Composite factor r is faster to generate by far. */
            my_bigint_random_n(r, rmm);
            bigint_add(r, rmm , r);
            SET_BIT(r->b, 0);
         }

         /* Compute prime candidate and n_1 */
         bigint_mult(q, r, n_1);      /* n_1 = q * r = (n-1)/2 */
         bigint_shift(n_1, -1);       /* n_1 = 2 * q * r       */
         bigint_add_int(n_1, 1, n);   /* n = 2 * q * r + 1     */

         /* Toss candidate if any common divisors with big composite */
         if (bigint_gcd(composite, n, X) == 1) {
            i++;
            continue;
         }

         /* See if a ^ (n-1) mod n == 1 */
         bigint_pow_mod(a, n_1, n, X);
         if (bigint_compare_int(X, 1) != 0) {
            i++;
            continue;
         }

         /* See if gcd( a^(2r) mod n - 1, n) = 1 */
         bigint_shift(r, -1);         /* r = 2r = (n-1)/q */
         bigint_pow_mod(a, r, n, X);
         bigint_subtract_int(X, 1);
         if (bigint_gcd(X, n, Y) == 1) {
            i++;
            continue;                 /* n is not prime */
         }
         bigint_shift(r, 1);          /* Undo previous r = 2r */

         /* Apply more stringent validation of r's primeness */
         if ((flags & MAKE_R_PRIME) && (bits > 48) && !bigint_is_prime(r, 57)) {
            i++;
            continue;
         }

         /* Pocklington says we're prime */
         have_proof = 1;

         /* Write prime and n-1 factors to proof file */
         write_proof(fproofs, n, q, r, outbase);

      } while (!have_proof && ((bits && i < bits) || (minmax && i < bigint_width(rmm) )));
   } while (!have_proof);

   if (!(flags & BE_QUIET)) {
      printf("\n%d bits, %d tries: ", bits, i);
      bigint_print(n, outbase);
      printf(",%s,", (outbase==2)?"10":"2");
      bigint_print(q, outbase);
      printf(",");
      bigint_print(r, outbase);
      printf("\n");
   }

   bigint_free(q);
   bigint_free(r);
   bigint_free(rmm);
   bigint_free(n_1);
   bigint_free(a);
   bigint_free(X);
   bigint_free(Y);
   return;
}

/*****************************************************************************
 * show_usage()
 ****************************************************************************/
void show_usage(char *me) {
   printf("%s - Construct proven primes of the form p = 2qr + 1\n", me);
   printf("\n");
   printf("Usage: %s -b <bits> [-n <num>] [-f] [-m] [-u] [-z] [-o <base>]\n", me);
#ifdef WIN32
   printf("or     %s -e\n", me);
#endif
   printf("\n");
   printf("\t-b\tGenerate primes of given width (64,80,128,256,etc)\n");
   printf("\t-n\tGenerate this many primes (default is 1)\n");
   printf("\t-m\tAllow r of generated prime to be composite\n");
   printf("\t-f\tUse input file of pregenerated primes for values of r\n");
   printf("\t-z\tReset q and repeat when primes file exhausted (sets -f)\n");
   printf("\t-i\tUse this number base to read values from primes file\n");
   printf("\t-o\tUse this number base for output, including to files\n");
   printf("\t-u\tUse Maurer's for r instead of random + Rabin-Miller\n");
   printf("\t-q\tQuiet mode (less screen output in various ways)\n");
#ifdef WIN32
   printf("\t-e\tStir RNG entropy pool with keyboard input (do at least once!)\n");
#endif
   printf("\n");
   printf("Examples:\n");
   printf("%s -b 256        Construct 256-bit prime (to primes.proof)\n", me);
   printf("%s -b 1023 -m    Allow composite factor (pregenerate r values)\n", me);
   printf("%s -b 2048 -z    Use previously generated prime factors\n", me);
   printf("\n");
   printf(
"Use 'pgen -e' to scramble the RNG (if on Windows). Then, to create p=2qr+1\n"
"2048-bit primes, run 'pgen -b 1023 -m -n 300' overnight on a few machines\n"
"to produce a cache of proven 1023-bit primes to try for values of r. Combine\n"
"all of the primes files into one using pmerge and distribute the merged\n"
"list back to all of the machines you're running on. You'll want 500-600\n"
"1023-bit primes, and 100-200 1022-bit primes wouldn't hurt either. Finally,\n"
"run 'pgen -b 2048 -z' on a few machines and wait for a winner.\n"
"Once you have a winner, use 'proots -r NNNNNN -n 10' (where NNNNNN is the\n"
"first few digits of it) to list the first 10 primitive roots and pick one\n"
"to use for a generator. Now you've got g and p for use with g^x mod p.\n"
);
   return;
}

/*****************************************************************************
 * main()
 ****************************************************************************/
int main(int argc, char *argv[]) {
 unsigned int i, bits, max_primes, flags, inbase, outbase;
 char c;
 bigint_t *prime;
 char *fprimes, *fproofs;
 char temp[10];

   fprimes  = "./primes";
   fproofs   = "./primes.proof";
   bits     = 0;
   max_primes = 1;
   inbase   = 10;
   outbase  = 10;
   flags    = MAKE_R_PRIME;

   while ((c = getopt2(argc,argv,"fmquzeb:n:t:i:o:")) > 0) {
      switch(c) {
       case 'e':
         flags |= ADD_ENTROPY;
         break;
       case 'm':
         flags &= ~MAKE_R_PRIME;
         break;
       case 'q':
         flags |= BE_QUIET;
         break;
       case 'f':
         flags |= USE_FPRIMES;
         break;
       case 'z':
         flags |= USE_FPRIMES;
         flags |= RESET_Q_SOONER;
         break;
       case 'u':
         flags |= USE_PROVEN_R;
         break;
       case 'b':
         bits = atoi(optarg2);
         if (bits < 8) {
            bits = 8;
         }
         break;
       case 'n':
         max_primes = atoi(optarg2);
         if (max_primes < 1) {
            max_primes = 1;
         }
         break;
       case 'i':
         inbase = atoi(optarg2);
         if (inbase < 2) {
            inbase = 2;
         }
         if (inbase > 16) {
            inbase = 16;
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
       case '*':
         if (!strcmp(fprimes, "./primes")) {
            fprimes = optarg2;
         } else {
            if (!strcmp(fproofs, "./primes.proof")) {
               fproofs = optarg2;
            }
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
   if (!bits) {
      show_usage(argv[0]);
      return 0;
   }

   /* composite = 3 * 5 * 7 * 11 * ... * 997 (all odd primes < 1000) */
   composite = bigint_init(32, 3);
   for (i=5;i<1000;i+=2) {
      if (is_prime(i)) {
         bigint_mult_int(composite, i, composite);
      }
   }
   prime = bigint_init(32, 0);

   /* Open up global instance of platform RNG so as not to thrash opens */
#ifdef WIN32
   skrand = skrand_init("skrand.bin");
   /* Stir hash state to be sure multiple instances diverge */
   sprintf(temp, "%d", GetCurrentProcessId());
   skrand_stir(skrand, temp, strlen(temp));
#else
   fdrand = open("/dev/urandom", O_RDONLY);
#endif

   maxprovable = bigint_init(32, 0);
   bigint_atoi(maxprovable, "341550071728321", 10);

   if (bits >= 12) {
      while (max_primes--) {
         generate_proven_prime(prime, bits, NULL, flags, fprimes, inbase,
                               fproofs, outbase);
         printf("Prime found: ");
         bigint_print(prime, outbase);
         printf("\n");
         write_prime(fprimes, prime, outbase);
#ifdef WIN32
         skrand_free(skrand, "skrand.bin");
         skrand = skrand_init("skrand.bin");
#endif
      }
   } else {
      printf("12 bits is my minimum constructable prime width. It is\n");
      printf("recommended to use the probabilistic generator, which\n");
      printf("produces proven primes up to 48 bits. If primitive roots\n");
      printf("on small primes are desired, use the probabilistic\n");
      printf("generator and then the brute force root finder in proots,\n");
      printf("which does exhaustive testing in a few minutes on primes\n");
      printf("up to about 10 million on a 1 or 2Ghz machine.\n");
   }

#ifdef WIN32
   skrand_free(skrand, "skrand.bin");
#else
   close(fdrand);
#endif
   bigint_free(prime);
   bigint_free(composite);
   bigint_free(maxprovable);
   return 1;
}
