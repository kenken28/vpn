/*****************************************************************************
 * ptest - Test a given value for primality using a probabilistic method
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

#define USE_PROOFS 1

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
 * show_usage()
 ****************************************************************************/
void show_usage(char *me) {
   printf("Usage: %s <number> [-i <base>] [-o <base>] [-c <certainty>]\n", me);

   printf("\n");
   printf("\t-i\tUse this number base for input\n");
   printf("\t-o\tUse this number base for output, including to files\n");
   printf("\t-c\tOverride number of tests performed (default is 57)\n");
   printf("\n");
   printf("Examples:\n");
   printf("%s 1a4bd37 -i 16    Test and convert number to decimal)\n", me);
   printf("%s 1013 -o 16       Test and convert number to hexadecimal)\n", me);
   printf("\n");
   printf(
"This program tests a given value for primality using a probabilistic test.\n"
);
   return;
}

/*****************************************************************************
 * main()
 ****************************************************************************/
int main(int argc, char *argv[]) {
 unsigned int i, flags, inbase, outbase;
 char c;
 bigint_t *prime, *composite, *temp;
 char *testptr, *sptr;
 int rc;
 int certainty;

   inbase    = 10;
   outbase   = 10;
   testptr   = NULL;
   flags     = 0;
   certainty = -1;

   while ((c = getopt2(argc,argv,"i:o:f:c:")) > 0) {
      switch (c) {
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
       case 'f':
         flags |= USE_PROOFS;
         break;
       case 'c':
         certainty = atoi(optarg2);
         if (certainty < 0) {
            certainty = 0;
         }
         break;
       case '*':
         testptr = optarg2;
         break;
       case '?':
       default:
         show_usage(argv[0]);
         return 0;
      }
   }

   if (!testptr || !*testptr) {
      show_usage(argv[0]);
      return 0;
   }

   prime     = bigint_init(32, 0);
   composite = bigint_init(32, 3);
   temp      = bigint_init(32, 0);

   /* composite = 3 * 5 * 7 * 11 * ... * 997 (all odd primes < 1000) */
   for (i=5;i<1000;i+=2) {
      if (is_prime(i)) {
         bigint_mult_int(composite, i, composite);
      }
   }

   /* Test input for hex. If any, switch input base to hex */
   if (inbase == 10) {
      sptr = testptr;
      while (*sptr) {
         if (strchr("ABCDEFabcdef", *sptr)) {
            inbase = 16;
            break;
         }
         sptr++;
      }
   }

   printf("Testing ");
   bigint_atoi(prime, testptr, inbase);
   bigint_print(prime, outbase);
   printf("\n");

   rc = bigint_compare_int(prime, 2);
   if (rc == -1) {
      printf("Number is not prime because it's %d\n", prime->b[0]);
   } else if (rc == 0) {
      printf("Number is prime.\n");
   } else if ((prime->b[0] & 1) == 0) {
      printf("Number is not prime because it's even\n");
   } else {
      if (bigint_compare_int(prime, 997) > 0 &&
          bigint_gcd(composite, prime, temp) == 1 &&
          bigint_compare(prime, temp) != 0) {
         printf("Number is not prime with divisor of ");
         bigint_print(temp, outbase);
         printf("\n");
      } else {
         if (certainty == -1) {
            certainty = 57;
         }
         if (bigint_is_prime(prime, certainty)) {
            if (bigint_width(prime) <= 48) {
               printf("Number is prime; small enough for Rabin-Miller proof.\n");
            } else {
               printf("Number appears to be prime.\n");
            }
         } else {
            printf("Number is not prime; deemed composite by Rabin-Miller.\n");
         }
      }
   }
   bigint_free(prime);
   bigint_free(composite);
   bigint_free(temp);
   return 1;
}
