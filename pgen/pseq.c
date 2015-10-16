/*****************************************************************************
 * pseq - Generate sequential primes using probabilistic methods.
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

/*****************************************************************************
 * iterative_generate()
 ****************************************************************************/
int iterative_generate(bigint_t * prime, int max, int outbase) {
 int i;

   /* We're going to be incrementing by 2, so special-case only even prime */
   i = 1;
   if (bigint_compare_int(prime, 3) < 0) {
      if (outbase == 2) {
         printf("10\n");
      } else {
         printf("2\n");
      }
      bigint_set(prime, 3);
      i = 2;
   }
   /* All primes are odd from here on out */
   prime->b[0] |= 1;
   while (i <= max) {
      if (bigint_is_prime(prime, 57)) {
         bigint_print(prime, outbase);
         printf("\n");
         i++;
      }
      bigint_add_int(prime, 2, prime);
   }
   return 0;
}

/*****************************************************************************
 * show_usage()
 ****************************************************************************/
void show_usage(char *me) {
   printf("Usage: %s <start> [-n <num>] [-i <base>] [-o <base>]\n", me);
   printf("\n");
   printf("\t-n\tGenerate this many primes (default is 1)\n");
   printf("\t-i\tStarting value is in this number base\n");
   printf("\t-o\tUse this number base for output\n");
   printf("\n");
   printf("Examples:\n");
   printf("%s 2 -n 10         Display first 10 primes\n", me);
   printf("%s 999988889999    Display next prime after 999988889999\n", me);
   printf("%s c0ffee00 -o 16  Look for a good morning prime\n", me);
   printf("\n");
   printf(
"\n"
);
   return;
}

/*****************************************************************************
 * main()
 ****************************************************************************/
int main(int argc, char *argv[]) {
 unsigned int i, max_primes, inbase, outbase;
 char c;
 char *startptr, *sptr;
 bigint_t *start;

   max_primes = 1;
   inbase   = 10;
   outbase  = 10;
   startptr = NULL;

   while ((c = getopt2(argc,argv,"n:i:o:")) > 0) {
      switch(c) {
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
         startptr = optarg2;
         break;
       case '?':
       default:
         show_usage(argv[0]);
         return 0;
      }
   }

   if (!startptr || !*startptr) {
      show_usage(argv[0]);
      return 0;
   }

   /* Test for hex in the row. If any, switch input base to hex */
   sptr = startptr;
   while (*sptr) {
      if (strchr("ABCDEFabcdef", *sptr)) {
         inbase = 16;
         break;
      }
      sptr++;
   }

   /* Generate sequential primes. (proven if 48 bits or less) */
   start = bigint_init(32, 0);
   bigint_atoi(start, startptr, inbase);
   if (bigint_compare_int(start, 2) < 0) {
       bigint_set(start, 2);
   }
   iterative_generate(start, max_primes, outbase);
   bigint_free(start);
   return 1;
}
