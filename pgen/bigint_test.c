/*****************************************************************************
 * bigint regression tests and usage examples
 * Aaron Logue 2013
 * Beware that some of these tests assume 32-bit element sizes
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "bigint.h"

#define BIGINTS 4

/*****************************************************************************
 * long_pow_mod()
 ****************************************************************************/
int2x_type long_pow_mod(int2x_type base,
                        int2x_type exponent,
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
 * bigint regression tests - NOTE: a lot of these tests assume values
 * left by previous tests, so beware if adding new tests in the middle.
 ****************************************************************************/
int main(int argc, char *argv[]) {
 int_type i;
 bigint_t *big[BIGINTS];
 unsigned char *str;
 int pass = 1;
 int counts[3];

   /* Test platform compatibility */
   if (sizeof(int_type) != 4) {
      pass = 0;
      printf("%d Error: sizeof(int_type) is %d. Edit bigint.h to make it 4.\n",
             __LINE__, sizeof(int_type));
   }
   if (sizeof(int2x_type) != 8) {
      pass = 0;
      printf("%d Error: sizeof(int2x_type) is %d. Edit bigint.h to make it 8.\n",
             __LINE__, sizeof(int_type));
   }

   /* Test bigint_init() and set ints to 0, 0x555.., 0xaaa.., 0xfff... */
   for (i=0; i<BIGINTS; i++) {
      big[i] = NULL;
      big[i] = bigint_init(INTBITS, i * 0x55555555);
      if (big[i]->elements != 1) {
         pass = 0;
         printf("%d Error: bigint_init %d elements = %d\n",
                __LINE__, i, big[i]->elements);
      }
      if (big[i]->b[0] != i * 0x55555555) {
         pass = 0;
         printf("%d Error: bigint_init %d expected %08x got %08x\n",
                __LINE__, i, i * 0x55555555, big[i]->b[0]);
      }
   }

   /* Test bigint_compare_int() */
   for (i=0; i<BIGINTS-1; i++) {
      if (i < BIGINTS-1) {
         if (bigint_compare_int(big[i], big[i+1]->b[0]) != -1) {
            pass = 0;
            printf("%d Error: bigint_compare_int %u,%u says %x >= %x\n",
                   __LINE__, i, i+1, big[i]->b[0], big[i+1]->b[0]);
         }
      }
      if (i > 0) {
         if (bigint_compare_int(big[i], big[i-1]->b[0]) != 1) {
            pass = 0;
            printf("%d Error: bigint_compare_int %u,%u says %x <= %x\n",
                   __LINE__, i, i-1, big[i]->b[0], big[i-1]->b[0]);
         }
      }
   }

   /* Subtracting 1 from big[0] should make it equal to big[3] */
   i = big[1]->b[0];
   bigint_set(big[1], 1);
   if (bigint_compare_int(big[1], 1) != 0 || big[1]->elements != 1) {
      pass = 0;
      printf("%d Error: bigint_set failed to set big[1] equal to 1\n", __LINE__);
   }
   bigint_subtract(big[0], big[1]);
   if (bigint_compare(big[0], big[3]) != 0 || big[0]->elements != 1) {
      pass = 0;
      printf("%d Error: bigint_subtract big[0] != big[3]\n", __LINE__);
   }
   big[1]->b[0] = i;
   bigint_set(big[0], 0);
   if (bigint_compare_int(big[0], 0) != 0 || big[0]->elements != 1) {
      pass = 0;
      printf("%d Error: bigint_set failed to set big[0] equal to 0\n", __LINE__);
   }

   /* Test bigint_nonzero */
   for (i=0; i<BIGINTS; i++) {
      if (bigint_nonzero(big[i]) != (i != 0)) {
         pass = 0;
         printf("%d Error: bigint_nonzero returns %d for value of %x\n",
            __LINE__, bigint_nonzero(big[i]), big[i]->b[0]);
      }
   }

   /* Test shifting, simple mult_int, and divide_int */
   bigint_shift(big[1], -1);
   if (big[1]->elements > 1) {
      pass = 0;
      printf("%d Error: bigint_shift big[1] -1 unnecessarily added an element\n",
             __LINE__);
   }
   if (bigint_compare(big[1], big[2]) != 0) {
      pass = 0;
      printf("%d Error: bigint_shift big[1] -1 resulted in %x instead of %x\n",
             __LINE__, big[1]->b[0], big[2]->b[0]);
   }
   bigint_shift(big[1], -1);   /* result s/b 17-bit value 0x155555554 */
   if (big[1]->elements != 2) {
      pass = 0;
      printf("%d Error: bigint_shift big[1] -1 failed to extend\n", __LINE__);
   }
   bigint_mult_int(big[2], 2, big[2]);
   if (big[2]->elements != 2) {
      pass = 0;
      printf("%d Error: bigint_mult_int big[2] 2 failed to extend\n", __LINE__);
   }
   fflush(0);
   if (bigint_compare(big[1], big[2]) != 0) {
      pass = 0;
      printf("%d Error: shift -1 and mult 2 should be equivalent. %x%08x <> %x%08x\n",
             __LINE__, big[1]->b[1], big[1]->b[0], big[2]->b[1], big[2]->b[0]);
   }
   bigint_shift(big[1], 1);
   bigint_divide_int(big[2], 2, big[2]);
   if (bigint_compare(big[1], big[2]) != 0) {
      pass = 0;
      printf("%d Error: shift 1 and divide by 2 should be equivalent. %x%08x <> %x%08x\n",
             __LINE__, big[1]->b[1], big[1]->b[0], big[2]->b[1], big[2]->b[0]);
   }
   bigint_mult_int(big[1], big[2]->b[0], big[1]);
   if (big[1]->b[1] != 0x71c71c70 || big[1]->b[0] != 0xe38e38e4) {
      pass = 0;
      printf("%d Error: bigint_mult_int 0x71c71c70e38e38e4 <> %x%08x\n",
             __LINE__, big[1]->b[1], big[1]->b[0]);
   }
   bigint_divide_int(big[1], big[2]->b[0], big[1]);
   if (bigint_compare(big[1], big[2]) != 0) {
      pass = 0;
      printf("%d Error: %x%08x <> %x%08x\n",
             __LINE__, big[1]->b[1], big[1]->b[0], big[2]->b[1], big[2]->b[0]);
   }
   
   /* Test carrying bigint_mult_int */
   big[3]->b[0] = 0x30000000;
   bigint_mult_int(big[3], 16, big[3]);
   if (big[3]->b[1] != 3 || big[3]->b[0] != 0) {
      pass = 0;
      printf("%d Error: bigint_mult_int 0x3000000 * 16 carry failed.\n", __LINE__);
   }

   /* Test bigint_mult */
   bigint_mult(big[1], big[2], big[3]);
   if (big[3]->b[1] != 0x71c71c70 || big[3]->b[0] != 0xe38e38e4) {
      pass = 0;
      printf("%d Error: bigint_mult 0x71c71c70e38e38e4 <> %x%08x\n",
             __LINE__, big[3]->b[1], big[3]->b[0]);
   }
   bigint_mult(big[3], big[3], big[1]); /* big[1] = 0xAAAAAAAA ^ 4 */
   if (big[1]->elements != 4 || big[1]->b[3] != 0x329161f8 ||
       big[1]->b[2] != 0xe38e38e4 || big[1]->b[1] != 0xbda12f67 ||
       big[1]->b[0] != 0x81948b10) {
      pass = 0;
      printf("%d Error: bigint_mult 0xAAAAAAAA ^ 4 failed.\n", __LINE__);
   }
   bigint_divide_int(big[1], big[2]->b[0], big[1]);
   bigint_divide_int(big[1], big[2]->b[0], big[1]);
   bigint_divide_int(big[1], big[2]->b[0], big[1]);
   if (bigint_compare(big[1], big[2]) != 0) {
      pass = 0;
      printf("%d Error: bigint_divide_int unexpected result %x%08x <> %x%08x\n",
             __LINE__, big[1]->b[1], big[1]->b[0], big[2]->b[1], big[2]->b[0]);
   }

   /* Test bigint_copy element reduction */
   bigint_copy(big[0], big[1]);
   if (big[0]->elements > 1) {
      pass = 0;
      printf("%d Error: bigint_copy failed to ignore leading zero elements\n",
             __LINE__);
   }

   /* Test bigint_add: add three 0xAAAAAAAA and then divide by 3 */
   bigint_add(big[0], big[2], big[2]);
   bigint_add(big[1], big[2], big[2]);
   bigint_divide_int(big[2], 3, big[2]);
   if (bigint_compare(big[1], big[2]) != 0) {
      pass = 0;
      printf("%d Error: %x%08x <> %x%08x\n",
             __LINE__, big[1]->b[1], big[1]->b[0], big[2]->b[1], big[2]->b[0]);
   }

   /* Test bigint_mod */   
   bigint_mod(big[3], big[1], big[0]);
   if (bigint_nonzero(big[0])) {
      pass = 0;
      printf("%d Error: bigint_mod 71c71c70e38e38e4 % aaaaaaaa != 0\n", __LINE__);
   }
   bigint_add_int(big[3], 1, big[3]);
   bigint_mod(big[3], big[1], big[0]);
   if (bigint_compare_int(big[0], 1) != 0) {
      pass = 0;
      printf("%d Error: bigint_mod 71c71c70e38e38e5 % aaaaaaaa != 1\n", __LINE__);
   }

   /* Test bigint_atoi and bigint_itoa */
   bigint_atoi(big[1], "AAAAAAAA", 16);
   if (bigint_compare(big[1], big[2]) != 0) {
      pass = 0;
      printf("%d Error: bigint_atoi unexpected result %x%08x <> %x%08x\n",
             __LINE__, big[1]->b[1], big[1]->b[0], big[2]->b[1], big[2]->b[0]);
   }
   bigint_atoi(big[1], "2863311530", 10);
   if (bigint_compare(big[1], big[2]) != 0) {
      pass = 0;
      printf("%d Error: bigint_atoi unexpected result %x%08x <> %x%08x\n",
             __LINE__, big[1]->b[1], big[1]->b[0], big[2]->b[1], big[2]->b[0]);
   }
   str = bigint_itoa(big[1], 10);
   if (strcmp(str, "2863311530")) {
      pass = 0;
      printf("%d Error: bigint_itoa failed\n", __LINE__);
   }
   free(str);
   /* base 12: 61917364224 5159780352 429981696 35831808 2985984 248832 20736 1728 144 12 1 */
   bigint_atoi(big[0], "B9A838B0987", 12); /* = 732126679159 in base 10 */
   str = bigint_itoa(big[0], 2);
   if (strcmp(str, "1010101001110110001001101001000001110111")) {
      pass = 0;
      printf("%d Error: bigint_itoa failed\n", __LINE__);
   }
   free(str);

   /* Test bigint_mod_pow */
   bigint_set(big[0], 3);
   bigint_set(big[1], 103482922);
   bigint_set(big[2], 310448767);
   bigint_pow_mod(big[0], big[1], big[2], big[3]);
   if (bigint_compare_int(big[3], 177876406) != 0) {
      pass = 0;
      printf("%d Error: bigint_pow_mod failed\n", __LINE__);
   }
   bigint_atoi(big[1], "b6f3cae8cab2b3d77678bded434ff5eb", 16);
   bigint_atoi(big[2], "9931311c9b54b851f0a4641d17afca83", 16);
   bigint_mult(big[1], big[2], big[3]);
   bigint_mult_int(big[3], 6, big[1]);
   bigint_atoi(big[2], "290e12830734709234c7e1c23206cadc81780a294cc813dd68f558b688fa39f87", 16);
   bigint_pow_mod(big[0], big[1], big[2], big[3]);
   if (bigint_compare_int(big[3], 1) != 0) {
      pass = 0;
      printf("%d Error: bigint_pow_mod 3^(n-1) mod n = 1 failed\n", __LINE__);
   }

   /* Give bigint_is_prime a whack... */
   if (bigint_is_prime(big[1], 20)) {
      pass = 0;
      printf("%d Error: bigint_is_prime is returning true for an even number\n", __LINE__);
   }
   if (bigint_is_prime(big[2], 20) == 0) {
      pass = 0;
      printf("%d Error: bigint_is_prime is returning false for a prime number\n", __LINE__);
   }

/*
   bigint_atoi(big[2], "512823a5c117fccfd807d118fad086cd", 16);
   if (bigint_is_prime(big[2]) == 0) {
      pass = 0;
      printf("%d Error: bigint_is_prime is returning false for a prime number\n", __LINE__);
   }
   bigint_atoi(big[2], "5d36f172dd645477b4e2a90c086b4943", 16);
   if (bigint_is_prime(big[2]) == 0) {
      pass = 0;
      printf("%d Error: bigint_is_prime is returning false for a prime number\n", __LINE__);
   }
   bigint_atoi(big[2], "b14dffc39e60ea593cdcf5ef4a190154ad805a0bc04366470a25163b20306beb", 16);
   if (bigint_is_prime(big[2]) == 0) {
      pass = 0;
      printf("%d Error: bigint_is_prime is returning false for a prime number\n", __LINE__);
   }
*/

   /* Test bigint_gcd */
   bigint_set(big[0], 9);
   bigint_set(big[1], 15);
   if (!bigint_gcd(big[0], big[1], big[2]) || bigint_compare_int(big[2], 3) != 0) {
      pass = 0;
      printf("%d Error: bigint_gcd couldn't figure out that gcd(9,15) = 3\n", __LINE__);
   }
   /* Generate 3 proven 46, 47, and 48-bit primes using the 7-value Rabin-Miller */
   for (i=0; i<3; i++) {
      do {
         bigint_random(big[i], 46+i);   /* 46, 47, 48 bit random numbers */
         SET_BIT(big[i]->b,0);          /* Guarantee that big[i] is odd */
         SET_BIT(big[i]->b,45+i);       /* Set high bit */
      } while (bigint_is_prime(big[i], 20) == 0);
   }
   bigint_mult(big[0], big[1], big[3]); /* big[3] = product of two primes */
   bigint_mult(big[0], big[2], big[1]); /* big[1] = product of two primes */

   if (!bigint_gcd(big[1], big[3], big[2]) || bigint_compare(big[0], big[2])) {
      pass = 0;
      printf("%d Error: bigint_gcd failed to find a 46-bit prime common factor\n", __LINE__);
      bigint_dump("  common factor: ", big[0]);
      bigint_dump("  first number:  ", big[1]);
      bigint_dump("  second number: ", big[3]);
   } else {
      printf("1st product of two primes: ");
      bigint_print(big[1], 10);
      printf("\n");
      printf("2nd product of two primes: ");
      bigint_print(big[3], 10);
      printf("\n");
      printf("Common prime factor found: ");
      bigint_print(big[2], 10);
      printf("\n");
   }

   /* Test bigint_divide() by recovering big[1]'s other factor */
   bigint_divide(big[1], big[0], big[3]);
   bigint_divide(big[1], big[3], big[2]);
   if (bigint_compare(big[0], big[2])) {
      pass = 0;
      printf("%d Error: bigint_divide failed to split product as expected\n", __LINE__);
   }
   bigint_mult(big[2], big[3], big[0]);
   if (bigint_compare(big[0], big[1])) {
      pass = 0;
      printf("%d Error: bigint_divide's factors didn't reconstitute product as expected\n", __LINE__);
   }

   /* big[0] = 3 * 5 * 7 * 11 * ... * 997 (all odd primes < 1000) */
   bigint_set(big[0], 3);
   for (i=5;i<1000;i+=2) {
      if (is_prime(i)) {
         bigint_mult_int(big[0],i,big[0]);
      }
   }
   bigint_set(big[1], 17);
   if (!bigint_gcd(big[0], big[1], big[2])) {
      pass = 0;
      printf("%d Error: bigint_gcd didn't find 17\n", __LINE__);
   }

   /* Test bigint_sqrt */
   for (i=0; i<5; i++) {
      bigint_random(big[0], 100);   /* 100-bit random number */
      bigint_mult(big[0], big[0], big[1]);
      bigint_sqrt(big[1], big[2]);
      if (bigint_compare(big[0], big[2])) {
         pass = 0;
         printf("%d Error: bigint_sqrt failed to reverse bigint_mult square\n", __LINE__);
         printf("  ");
         bigint_print(big[0], 10);
         printf("^2 = ");
         bigint_print(big[1], 10);
         printf("\n  sqrt(");
         bigint_print(big[1], 10);
         printf(") = ");
         bigint_print(big[2], 10);
         printf("\n");
      }
   }

   /* Test bigint_random_n() uniformity */
   counts[0] = 0;
   counts[1] = 0;
   counts[2] = 0;
   bigint_random(big[0], 100);   /* 100-bit random number */
   bigint_divide_int(big[0], 3, big[1]);  /* big[1] is 1/3 of big[0] */
   bigint_mult_int(big[1], 2, big[2]);    /* big[2] is 2/3 of big[0] */

   for (i=0; i<10000; i++) {
      bigint_random_n(big[3], big[0]);    /* big[3] = random 0 to big[0]-1 */
      if (bigint_compare(big[3], big[1]) < 0) {
         counts[0]++;
      } else {
         if (bigint_compare(big[3], big[2]) < 0) {
            counts[1]++;
         } else {
            counts[2]++;
         }
      }
   }
   printf("bigint_random_n distribution: %d %d %d\n",
      counts[0], counts[1], counts[2]);

   /* Test bigint_set() */
   for (i=0; i<BIGINTS; i++) {
      bigint_set(big[i], 0);
      if (bigint_compare_int(big[i], 0) != 0) {
         pass = 0;
         printf("%d Error: bigint_set big[%u] to 0 failed\n", __LINE__, i);
      }
   }

   /* All done */
   for (i=0; i<BIGINTS; i++) {
      bigint_free(big[i]);
   }
   if (pass) {
      printf("All tests passed. Yay!\n");
   }
   return 0;
}
