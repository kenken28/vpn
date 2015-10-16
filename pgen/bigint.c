/****************************************************************************
 * Functions for performing arithmetic with large positive integers
 * Aaron Logue 2013
 * Understandability over speed is the goal here.
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#ifdef WIN32
#include "skrand.h"
#endif
#include "bigint.h"

/****************************************************************************
 * bigint_init()
 ***************************************************************************/
bigint_t * bigint_init(unsigned int bits, int_type val) {
 bigint_t * result;
 int elements;
   result = (bigint_t *)malloc(sizeof(bigint_t));
   if (result) {
      elements = bits >> SHIFTBITS;
      if (bits & (INTBITS-1)) {
         elements++;
      }
      result->elements = elements;
      result->b = (int_type *)malloc(elements * sizeof(int_type));
      if (result->b) {
         while (--elements) {
            result->b[elements] = 0;
         }
         result->b[0] = val;
      }
   }
   return result;
}

/****************************************************************************
 * bigint_free()
 ***************************************************************************/
void bigint_free(bigint_t * var) {
   if (var) {
      if (var->b) {
         free(var->b);
      }
      free(var);
   }
   return;
}

/****************************************************************************
 * bigint_extend() - Adds elements to a bigint array.
 * This is called as needed by various bigint_ functions.
 ***************************************************************************/
#define REALLOC_PROBLEMS
void bigint_extend(bigint_t * var, int elements) {
 int_type * newb;
#ifdef REALLOC_PROBLEMS
 int_type * old;

   old = var->b;
   newb = malloc(var->elements * sizeof(int_type) +
                          elements * sizeof(int_type));
   memcpy(newb, old, var->elements * sizeof(int_type));
#else
   newb = realloc(var->b, var->elements * sizeof(int_type) +
                          elements * sizeof(int_type));
#endif
   if (newb) {
      var->b = newb;
      while (elements) {
         var->b[var->elements] = 0;
         var->elements++;
         elements--;
      }
   }
#ifdef REALLOC_PROBLEMS
   free(old);
#endif
   return;
}

/****************************************************************************
 * bigint_dump()
 ***************************************************************************/
void bigint_dump(char *name, bigint_t * var) {
 int i;
   printf("%s ", name);
   for (i=var->elements; i>0; i--) {
      printf("%08x ", var->b[i-1]);
   }
   printf("\n");
   return;
}

/****************************************************************************
 * bigint_set()
 ***************************************************************************/
void bigint_set(bigint_t * var, int_type val) {
 unsigned int i;
   if (var && var->b) {
      var->b[0] = val;
      for (i=1; i<var->elements; i++) {
         var->b[i] = 0;
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
void bigint_random(bigint_t * var, unsigned int bits) {
 unsigned int i, elements;
#ifdef WIN32
 skrand_t *skrand;
#else
 unsigned int fd;
#endif
   elements = bits >> SHIFTBITS;
   if (bits & (INTBITS-1)) {
      elements++;
   }
   if (elements > var->elements) {
      bigint_extend(var, elements - var->elements);
   }
#ifdef WIN32
   skrand = skrand_init("skrand.bin");
   if (skrand) {
      for (i=0; i<elements; i++) {
         skrand_read(skrand, (char *)&var->b[i], sizeof(int_type));
      }
      skrand_free(skrand, "skrand.bin");
   }
#else   
   fd = open("/dev/urandom", O_RDONLY);
   if (fd >= 0) {
      for (i=0; i<elements; i++) {
         read(fd, &var->b[i], sizeof(int_type));
      }
      close(fd);
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
 * Beware: On Windows, this is slow because it's calling bigint_random
 * which is reading and writing skrand.bin. If you need a bunch of these
 * values, write your own bigint_random() and bigint_random_n() functions
 * that take an skrand_t * as one of the parms and make your own call to
 * skrand_init.
 ***************************************************************************/
void bigint_random_n(bigint_t * result, bigint_t * n) {
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
      bigint_random(modn, bits);
   } while (bigint_compare(modn, cutoff) == 1);
   bigint_mod(modn, n, result);

   bigint_free(cutoff);
   bigint_free(modn);
   return;
}

/****************************************************************************
 * bigint_copy()
 ***************************************************************************/
void bigint_copy(bigint_t * dst, bigint_t * src) {
 unsigned int i;

   i = 0;
   while (i < src->elements) {
      if (i < dst->elements) {
         dst->b[i] = src->b[i];
      } else {
         if (src->b[i]) {
            bigint_extend(dst, i - dst->elements + 1);
            dst->b[i] = src->b[i];
         }
      }
      i++;
   }
   while (i < dst->elements) {
      dst->b[i] = 0;
      i++;
   }
   return;
}

/****************************************************************************
 * bigint_add()
 * result can be the same as either source
 * This function handles arbitrary digit expansion
 ***************************************************************************/
void bigint_add(bigint_t * parm1, bigint_t * parm2, bigint_t * result) {
 unsigned int i, max_elements;
 int_type sum, carry, b1, b2;

   max_elements = parm1->elements;
   if (parm2->elements > max_elements) {
      max_elements = parm2->elements;
   }
   carry = 0;
   i = 0;
   do {
      b1 = 0;
      b2 = 0;
      if (i < parm1->elements) {
         b1 = parm1->b[i];
      }
      if (i < parm2->elements) {
         b2 = parm2->b[i];
      }
      sum = b1 + b2 + carry;
      carry = 0;
      if (sum < b1 || sum < b2) {
         carry = 1;
      }
      if (i < result->elements) {
         result->b[i] = sum;
      } else {
         if (sum) {
           bigint_extend(result, i - result->elements + 1);
           result->b[i] = sum;
         }
      }
      i++;
   } while (sum || carry || i < max_elements);
   while (i < result->elements) {
      result->b[i] = 0;
      i++;
   }
   return;
}

/****************************************************************************
 * bigint_add_int()
 * result can be the same as parm1
 * Result is expanded as needed
 ***************************************************************************/
void bigint_add_int(bigint_t * parm1, unsigned int parm2, bigint_t * result) {
 unsigned int i;
 int_type sum, carry;

   sum = parm1->b[0] + parm2;
   carry = 0;
   if (sum < parm1->b[0] || sum < parm2) {
      carry = 1;
   }
   result->b[0] = sum;
   i = 1;
   while (carry || i < parm1->elements) {
      sum = parm1->b[i] + carry;
      carry = 0;
      if (sum < parm1->b[i]) {
         carry = 1;
      }
      if (i < result->elements) {
         result->b[i] = sum;
      } else {
         if (sum) {
           bigint_extend(result, i - result->elements + 1);
           result->b[i] = sum;
         }
      }
      i++;
   }
   while (i < result->elements) {
      result->b[i] = 0;
      i++;
   }
   return;
}

/****************************************************************************
 * bigint_subtract()
 * Negative numbers are undefined; borrows to MSB 0 are dropped.
 * parm1 = parm1 - parm2
 ***************************************************************************/
void bigint_subtract(bigint_t * parm1, bigint_t * parm2) {
 unsigned int i, j, borrow;
 int_type b1, b2;

   borrow = 0;
   i = 0;
   do {
      b1 = 0;
      b2 = 0;
      if (i < parm1->elements) {
         b1 = parm1->b[i];
      }
      if (i < parm2->elements) {
         b2 = parm2->b[i];
      }
      if (b2 > b1) {
         borrow = 1;
      }
      b1 = b1 - b2;
      if (i < parm1->elements) {
         parm1->b[i] = b1;
      }
      i++;
      if (borrow) {
         j = i;
         while (borrow && j < parm1->elements) {
            if (parm1->b[j]) {
               parm1->b[j]--;
               borrow = 0;
            } else {
               parm1->b[j] = MAXINT;
            }
            j++;
         }
      }
   } while (i < parm1->elements && i < parm2->elements);
   return;
}

/****************************************************************************
 * bigint_subtract_int()
 * Negative numbers are undefined; borrows to MSB 0 are dropped.
 * parm1 = parm1 - parm2
 ***************************************************************************/
void bigint_subtract_int(bigint_t * parm1, unsigned int parm2) {
 unsigned int i, borrow;

   borrow = 0;
   if (parm2 > parm1->b[0]) {
      borrow = 1;
   }
   parm1->b[0] -= parm2;
   if (borrow) {
      i = 1;
      while (borrow && i < parm1->elements) {
         if (parm1->b[i]) {
            parm1->b[i]--;
            borrow = 0;
         } else {
            parm1->b[i] = MAXINT;
         }
         i++;
      }
   }
   return;
}

/****************************************************************************
 * bigint_mult()
 * Result must be distinct from parm1 or parm2, but will be extended as needed.
 ***************************************************************************/
void bigint_mult(bigint_t * parm1, bigint_t * parm2, bigint_t * result) {
 unsigned int i, j, k, e1, e2;
 int_type low, high;
 int2x_type product;

   /* Prevent leftmost zero elements from being processed */
   e1 = parm1->elements - 1;
   while (!parm1->b[e1] && e1) {
      e1--;
   }
   e2 = parm2->elements - 1;
   while (!parm2->b[e2] && e2) {
      e2--;
   }
   /* Guarantee that result is large enough to eliminate most in-loop checks */
   k = e1 + e2;
   j = result->elements;
   if (k >= j) {
      bigint_extend(result, k - result->elements + 1);
   }
   /* Zero whatever bigint_extend didn't zero */
   for (i=0; i<j; i++) {
      result->b[i] = 0;
   }
   /* Multiply each significant element from one parm with the other */
   for (i=0; i <= e1; i++) {
      for (j=0; j <= e2; j++) {
         product = (int2x_type)parm1->b[i] * (int2x_type)parm2->b[j];
         high = (int_type)(product >> INTBITS);
         low  = (int_type)(product & MAXINT);
         k = i+j;
         result->b[k] += low;
         if (result->b[k] < low) {
            high++;
         }
         /* Apply the carry */
         while (high) {
            k++;
            if (k >= result->elements) {
               bigint_extend(result, k - result->elements + 1);
            }
            result->b[k] += high;
            if (result->b[k] < high) {
               high = 1;
            } else {
               high = 0;
            }
         }
      }
   }
   return;
}

/****************************************************************************
 * bigint_mult_int() : result = parm2 * parm1
 * Accepts an unsigned int for parm2.
 * Result can be the same as parm1.
 ***************************************************************************/
void bigint_mult_int(bigint_t * parm1, unsigned int parm2, bigint_t * result) {
 unsigned int i;
 int_type low, high;
 int2x_type product;

   high = 0;
   for (i=0; i<parm1->elements; i++) {
      product = (int2x_type)parm1->b[i] * (int2x_type)parm2;
      product = product + high;
      high = (int_type)(product >> INTBITS);
      low  = (int_type)(product & MAXINT);
      if (i < result->elements) {
         result->b[i] = low;
      } else {
         if (low) {
            bigint_extend(result, i - result->elements + 1);
            result->b[i] = low;
         }
      }
   }
   if (i < result->elements) {
      result->b[i] = high;
   } else {
      if (high) {
         bigint_extend(result, i - result->elements + 1);
         result->b[i] = high;
      }
   }
   for (i++; i<result->elements; i++) {
      result->b[i] = 0;
   }
   return;
}

/****************************************************************************
 * bigint_divide()
 ***************************************************************************/
void bigint_divide(bigint_t * parm1, bigint_t * parm2, bigint_t * result) {
 int i, bit;
 bigint_t * remainder;

   bigint_set(result, 0);
   remainder = bigint_init(sizeof(int_type), 0);

   for (i=parm1->elements - 1; i>=0; i--) {
      for (bit=INTBITS-1; bit>=0; bit--) {
         bigint_shift(remainder, -1);
         if (parm1->b[i] & (1 << bit)) {
            remainder->b[0] |= 1;
         }
         bigint_shift(result, -1);
         if (bigint_compare(remainder, parm2) >= 0) {
            result->b[0] |= 1;
            bigint_subtract(remainder, parm2);        
         }
      }
   }
   bigint_free(remainder);
   return;
}

/****************************************************************************
 * bigint_divide_int()
 * Result can be the same as parm1.
 ***************************************************************************/
void bigint_divide_int(bigint_t * parm1, unsigned int parm2, bigint_t * result) {
 int i, max_re;
 int2x_type dividend;

   max_re = -1;
   dividend = 0;
   for (i=parm1->elements - 1; i>=0; i--) {
      dividend = dividend | parm1->b[i];
      if (i < result->elements) {
         result->b[i] = (int_type)(dividend / parm2);
         if (max_re == -1) {
            max_re = i;
         }
      } else {
         if (dividend & MAXINT) {
            bigint_extend(result, i - result->elements + 1);
            result->b[i] = (int_type)(dividend / parm2);
            if (max_re == -1) {
               max_re = i;
            }
         }
      }
      dividend = dividend % parm2;
      dividend = dividend << INTBITS;
   }
   for (i=result->elements - 1; i > max_re; i--) {
      result->b[i] = 0;
   }
   return;
}

/****************************************************************************
 * bigint_nonzero()
 * Return 1 if bigint is nonzero
 ***************************************************************************/
int bigint_nonzero(bigint_t * var) {
 unsigned int i;
   for (i=0; i<var->elements; i++) {
      if (var->b[i]) {
         return 1;
      }
   }
   return 0;
}

/****************************************************************************
 * bigint_compare()
 * Return -1 if parm1 < parm2, 1 if parm1 > parm2, and 0 if parm1 == parm2
 ***************************************************************************/
int bigint_compare(bigint_t * parm1, bigint_t * parm2) {
 unsigned int i;
 int_type b1, b2;

   i = parm1->elements;
   if (parm2->elements > i) {
      i = parm2->elements;
   }
   while (i) {
      i--;
      b1 = 0;
      b2 = 0;
      if (i < parm1->elements) {
         b1 = parm1->b[i];
      }
      if (i < parm2->elements) {
         b2 = parm2->b[i];
      }
      if (b1 < b2) {
         return -1;
      }
      if (b1 > b2) {
         return 1;
      }
   }
   return 0;
}

/****************************************************************************
 * bigint_compare_int()
 * Return -1 if parm1 < parm2, 1 if parm1 > parm2, and 0 if parm1 == parm2
 ***************************************************************************/
int bigint_compare_int(bigint_t * parm1, unsigned int parm2) {
 int i;

   i = parm1->elements - 1;
   while (i > 0) {
      if (parm1->b[i] > 0) {
         return 1;
      }
      i--;
   }
   if (parm1->b[0] < parm2) {
      return -1;
   }
   if (parm1->b[0] > parm2) {
      return 1;
   }
   return 0;
}

/****************************************************************************
 * bigint_shift()
 * Negative values shift parm1, positive values shift parm2
 * You can do big shifts; ie: -128
 ***************************************************************************/
void bigint_shift(bigint_t * parm1, int shift) {
 unsigned int i;
 int j, k, offset_r, offset_l, bits;
 int_type bits_l, bits_r;

   if (shift > 0) {
      offset_r = shift / INTBITS;
      bits = shift % INTBITS;
      offset_l = offset_r + 1;
      i = 0;
      while (i < parm1->elements) {
         k = i + offset_l;
         bits_l = 0;
         if (k < parm1->elements) {
            bits_l = parm1->b[k];
         }
         if (bits) {
            bits_l = bits_l << (INTBITS - bits);
         } else {
            bits_l = 0;
         }
         j = i + offset_r;
         bits_r = 0;
         if (j < parm1->elements) {
            bits_r = parm1->b[j];
         }
         bits_r = bits_r >> bits;
         parm1->b[i] = bits_l | bits_r;
         i++;
      }
   } else if (shift) {
      shift = 0 - shift;
      offset_l = shift / INTBITS;
      bits = shift % INTBITS;
      offset_r = offset_l + 1;
      i = parm1->elements - 1;

      /* Count how many leading zero bits there are */
      k = 0;
      for (j=i; j>=(i - offset_l) && (j >= 0); j--) {
         if (parm1->b[j]) {
            bits_l = parm1->b[j];
            while ((bits_l & (1 << (INTBITS-1))) == 0) {
               k++;
               bits_l = bits_l << 1;
            }
            break;
         } else {
            k += INTBITS; /* all bits are zero */
         }
      }
      k = shift - k; /* How many bits to the parm1 do we really need? */
      if (k > 0) {
         /* Convert bits to elements, rounding up */
         k = k / INTBITS + ((k % INTBITS) ? 1 : 0);
         if (k) {
            bigint_extend(parm1, k);
            i = parm1->elements - 1;
         }
      }

      do {
         k = i - offset_l;
         bits_l = 0;
         if (k >= 0) {
            bits_l = parm1->b[k];
         }
         bits_l = bits_l << bits;

         j = i - offset_r;
         bits_r = 0;
         if (j >= 0) {
            bits_r = parm1->b[j];
         }
         if (bits) {
            bits_r = bits_r >> (INTBITS - bits);
         } else {
            bits_r = 0;
         }
         parm1->b[i] = bits_l | bits_r;
         if (i == 0) {
            break;
         }
         i--;
      } while (1);
   }
   return;
}

/****************************************************************************
 * bigint_mod()
 ***************************************************************************/
void bigint_mod(bigint_t * parm1, bigint_t * parm2, bigint_t * result) {
 unsigned int i, bit;
 bigint_t * k;

   bigint_set(result, 0);
   k = bigint_init(sizeof(int_type), 1);
   for (i=0; i<parm1->elements; i++) {
      for (bit=0; bit<INTBITS; bit++) {
         if (parm1->b[i] & (1 << bit)) {
            bigint_add(result, k, result);
            if (bigint_compare(result, parm2) >= 0) {
               bigint_subtract(result, parm2);
            }
         }
         bigint_shift(k, -1);
         if (bigint_compare(k, parm2) >= 0) {
            bigint_subtract(k, parm2);
         }
      }
   }
   bigint_free(k);
   return;
}

/****************************************************************************
 * bigint_mod_int()
 ***************************************************************************/
unsigned int bigint_mod_int(bigint_t * parm1, unsigned int parm2) {
 unsigned int i, bit;
 int2x_type k, result;

   result = 0;
   k = 1;
   for (i=0; i<parm1->elements; i++) {
      for (bit=0; bit<INTBITS; bit++) {
         if (parm1->b[i] & (1 << bit)) {
            result = result + k;
            if (result >= parm2) {
               result = result - parm2;
            }
         }
         k <<= 1;
         if (k >= parm2) {
            k = k - parm2;
         }
      }
   }
   return (unsigned int)result;
}

/****************************************************************************
 * bigint_pow()
 * Beware calling this with large exponents. Use bigint_pow_mod instead
 * if performing modulo division after raising a number to a power.
 ***************************************************************************/
void bigint_pow(bigint_t * base, bigint_t * exp, bigint_t * result) {
 bigint_t *base_copy, *exp_copy, *temp;

   base_copy = bigint_init(32, 0);
   exp_copy = bigint_init(32, 0);
   temp = bigint_init(32, 0);

   bigint_copy(base_copy, base);
   bigint_copy(exp_copy, exp);

   bigint_set(result, 1);
   while (bigint_nonzero(exp_copy)) {
      if (exp_copy->b[0] & 1) {
         /* result = result * base */
         bigint_mult(result, base_copy, temp);
         bigint_copy(result, temp);
      }
      bigint_shift(exp_copy, 1);
      /* base = base * base */
      bigint_mult(base_copy, base_copy, temp);
      bigint_copy(base_copy, temp);
   }
   bigint_free(temp);
   bigint_free(exp_copy);
   bigint_free(base_copy);
   return;
}

/****************************************************************************
 * bigint_pow_mod()
 ***************************************************************************/
void bigint_pow_mod(bigint_t * base, bigint_t * exp, bigint_t * mod,
                    bigint_t * result) {
 bigint_t *base_copy, *exp_copy, *temp;

   base_copy = bigint_init(32, 0);
   exp_copy = bigint_init(32, 0);
   temp = bigint_init(32, 0);

   bigint_copy(base_copy, base);
   bigint_copy(exp_copy, exp);

   bigint_set(result, 1);
   while (bigint_nonzero(exp_copy)) {
      if (exp_copy->b[0] & 1) {
         /* result = result * base % mod */
         bigint_mult(result, base_copy, temp);
         bigint_mod(temp, mod, result);
      }
      bigint_shift(exp_copy, 1);
      /* base = base * base % mod */
      bigint_mult(base_copy, base_copy, temp);
      bigint_mod(temp, mod, base_copy);
   }
   bigint_free(temp);
   bigint_free(exp_copy);
   bigint_free(base_copy);
   return;
}

/****************************************************************************
 * bigint_rabin_miller_int()
 ***************************************************************************/
int bigint_rabin_miller_int(int a, bigint_t * n) {
 unsigned int i, s;
 int r;
 bigint_t *n_1, *ncon, *base, *temp;

   ncon = bigint_init(32, a);
   base = bigint_init(32, 0);
   temp = bigint_init(32, 0);
   n_1  = bigint_init(32, 0);

   bigint_copy(n_1, n);         /* n_1 = n - 1 */
   bigint_subtract_int(n_1, 1);
   s = 0;
   bigint_copy(temp, n_1);
   while ((temp->b[0] & 1) == 0) {
      bigint_shift(temp, 1);
      s++;
   }
   bigint_pow_mod(ncon, temp, n, base);

   if (bigint_compare_int(base, 1) == 0) {
      bigint_free(n_1);
      bigint_free(temp);
      bigint_free(base);
      bigint_free(ncon);
      return 1;
   }

   bigint_copy(n_1, n);
   bigint_subtract_int(n_1, 1);
   r = bigint_compare(base, n_1);
   bigint_set(ncon, 2);
   for (i=0; i < s-1; i++) {
      if (r == 0) {
         break;
      }
      if ((i&1) == 0) {
         bigint_pow_mod(base, ncon, n, temp);
         r = bigint_compare(temp, n_1);
      } else {
         bigint_pow_mod(temp, ncon, n, base);
         r = bigint_compare(base, n_1);
      }
   }
   bigint_free(n_1);
   bigint_free(temp);
   bigint_free(base);
   bigint_free(ncon);
   return (r == 0);
}

/****************************************************************************
 * bigint_rabin_miller()
 ***************************************************************************/
int bigint_rabin_miller(bigint_t * a, bigint_t * n) {
 unsigned int i, s;
 int r;
 bigint_t *n_1, *ncon, *base, *temp;

   temp = bigint_init(32, 1);
   ncon = bigint_init(32, 0);
   base = bigint_init(32, 0);
   n_1  = bigint_init(32, 0);

   bigint_copy(n_1, n);         /* n_1 = n - 1 */
   bigint_subtract(n_1, temp);
   s = 0;
   bigint_copy(temp, n_1);
   while ((temp->b[0] & 1) == 0) {
      bigint_shift(temp, 1);
      s++;
   }
   bigint_copy(ncon, a);
   bigint_pow_mod(ncon, temp, n, base);

   if (bigint_compare_int(base, 1) == 0) {
      bigint_free(n_1);
      bigint_free(temp);
      bigint_free(base);
      bigint_free(ncon);
      return 1;
   }

   bigint_copy(n_1, n);
   bigint_set(ncon, 1);
   bigint_subtract(n_1, ncon);
   r = bigint_compare(base, n_1);
   bigint_set(ncon, 2);
   for (i=0; i < s-1; i++) {
      if (r == 0) {
         break;
      }
      if ((i&1) == 0) {
         bigint_pow_mod(base, ncon, n, temp);
         r = bigint_compare(temp, n_1);
      } else {
         bigint_pow_mod(temp, ncon, n, base);
         r = bigint_compare(base, n_1);
      }
   }
   bigint_free(n_1);
   bigint_free(temp);
   bigint_free(base);
   bigint_free(ncon);
   return (r == 0);
}

/****************************************************************************
 * bigint_is_prime()
 *
 * This is intended for probabilistic primality testing.
 *
 * See https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test
 * For enough Rabin-Millering to prove 48-bit primes, use certainty = 6
 * certainty = 2 is enough to prove 32-bit primes.
 * if n < 1,373,653, it is enough to test a = 2 and 3;
 * if n < 9,080,191, it is enough to test a = 31 and 73;
 * if n < 4,759,123,141, it is enough to test a = 2, 7, and 61;
 * if n < 1,122,004,669,633, it is enough to test a = 2,13,23, and 1662803;
 * if n < 2,152,302,898,747, it is enough to test a = 2,3,5,7, and 11;
 * if n < 3,474,749,660,383, it is enough to test a = 2,3,5,7,11, and 13;
 * if n < 341,550,071,728,321, it is enough to test a = 2,3,5,7,11,13, and 17.
 * Let's call these first tests certainty 1-7
 ***************************************************************************/
int bigint_is_prime(bigint_t * n, int certainty) {
 int i, r;
 int rm_tests[7][7] = {      /* second index is number of lists */
   {3, 0},
   {31, 73},
   {7, 61, 0},
   {13, 23, 1662803, 0},
   {3, 5, 7, 11, 0},
   {3, 5, 6, 11, 13, 0},
   {3, 5, 7, 11, 13, 17, 0}
 };
 bigint_t * a;               /* random bases to test with Rabin-Miller */
 bigint_t * n_4;

   r = bigint_compare_int(n, 2);
   if (r == 0) {
      return 1;              /* n == 2, so it's prime */
   }
   if ((n->b[0] & 1) == 0) {
      return 0;              /* n is even, so it's not prime */
   }
   if (r == -1) {
      return 0;              /* n is less than 2, so it's not prime */
   }
   if (bigint_rabin_miller_int(2, n) == 0) {
      return 0;
   }
   if (certainty == 0) {
      return 1;              /* shrug shoulders and call it prime */
   }
   r = certainty - 1;
   if (r > 6) {
      r = 6;
   }
   i = 0;
   while (rm_tests[i][r]) {
      if (bigint_compare_int(n, rm_tests[i][r]) == 1 &&
          bigint_rabin_miller_int(rm_tests[i][r], n) == 0) {
// printf("r%d\n", rm_tests[i][r]);
         return 0;
      }
      i++;
   }
   certainty = certainty - r - 1;
   if (!certainty || bigint_width(n) <= 48) {
      return 1;
   }
   a   = bigint_init(32, 0);
   n_4 = bigint_init(32, 0);
   bigint_copy(n_4, n);
   bigint_subtract_int(n_4, 4);
   while (certainty) {
      /* Rabin-Miller test with a random number from 2 to n - 2 */
      bigint_random_n(a, n_4);
      bigint_add_int(a, 2, a);
      if (bigint_rabin_miller(a, n) == 0) {
// printf("c%d\n", certainty);
// tested with large pseudoprimes to be sure we got here
// see rabin-miller primality test: composite numbers which pass it
// F. Arnault 1995
         return 0;
      }
      certainty--;
   }
   bigint_free(a);
   bigint_free(n_4);
   return 1;
}

/****************************************************************************
 * bigint_atoi() - Convert strings to bigints using any base from 2 to 16
 * Example: bigint_atoi(dest, "123873256574552575852243582347234234", 10);
 * or bigint_atoi(dest, "F8798bc89AEd9a98ce980bf80093fde80980cb098a", 16);
 ***************************************************************************/
void bigint_atoi(bigint_t *result, char *str, unsigned int base) {
 unsigned int digit;
 char *dptr, *basechars = "0123456789ABCDEF";

   bigint_set(result, 0);
   while (*str) {
      dptr = strchr(basechars, toupper(*str));
      if (dptr) {
         digit = dptr - basechars;
         if (digit < base) {
            bigint_mult_int(result, base, result);
            bigint_add_int(result, digit, result);
         }
      }
      str++;
   }
}

/****************************************************************************
 * bigint_itoa() - Convert bigints to strings using any base from 2 to 16
 * Example: str = bigint_itoa(n, 10);
 * Caller is responsible for freeing returned string.
 ***************************************************************************/
char * bigint_itoa(bigint_t * n, unsigned int base) {
 int i, j;
 char *result;
 char c;
 bigint_t * num;
 char *basechars = "0123456789ABCDEF";

   if (base < 2 || base > 16) {
      return NULL;
   }
   if (bigint_nonzero(n)) {
      num = bigint_init(32, 0);
      bigint_copy(num, n); /* Skips leading 0s to reduce element count */
      i = num->elements * INTBITS;
      if (base >= 4) {
         i = i >> 1;
      }
      if (base >= 16) {
         i = i >> 1;
      }
      result = (char *)malloc(i + 1);
      if (result) {
         for (i=0; bigint_nonzero(num); i++) {
            result[i] = basechars[bigint_mod_int(num, base)];
            bigint_divide_int(num, base, num);
         }
         result[i] = 0;   /* Null-termiante and reverse the string */
         i--;
         j = 0;
         while (j < i) {
            c = result[i];
            result[i] = result[j];
            result[j] = c;
            j++;
            i--;
         }
      }
      bigint_free(num);
   } else {
      result = (char *)malloc(2);
      if (result) {
         strcpy(result, "0");
      }
   }
   return result;
}

/****************************************************************************
 * bigint_print()
 * Output a bigint as ASCII
 ***************************************************************************/
void bigint_print(bigint_t * var, unsigned int base) {
 char *str;
   str = bigint_itoa(var, base);
   if (str) {
      printf(str);
      free(str);
   }
}

/****************************************************************************
 * bigint_ctz2() - Counts trailing zeroes of the bitwise OR of two bigints.
 * This is called from bigint_gcd() with parm1 or parm2 guaranteed nonzero.
 ***************************************************************************/
int bigint_ctz2(bigint_t * parm1, bigint_t * parm2) {
 unsigned int i, count;
 int_type bits;

   count = 0;
   i = 0;
   while (i < parm1->elements || i < parm2->elements) {
      bits = parm1->b[i] | parm2->b[i];
      if (bits) {
         while ((bits & 1) == 0) {
            bits >>= 1;
            count++;
         }
         break;
      } else {
         count += INTBITS;
      }
      i++;
   }
   return count;
}

/****************************************************************************
 * bigint_shift_ctz() - Shift parm2 until odd
 * This function is called from bigint_gcd()
 ***************************************************************************/
void bigint_shift_ctz(bigint_t * parm1) {
 unsigned int i, count;
 int_type bits;

   count = 0;
   i = 0;
   while (i < parm1->elements) {
      bits = parm1->b[i];
      if (bits) {
         while ((bits & 1) == 0) {
            bits >>= 1;
            count++;
         }
         break;
      } else {
         count += INTBITS;
      }
      i++;
   }
   bigint_shift(parm1, count);
}

/****************************************************************************
 * bigint_gcd() - Find greatest common divisor
 * Also, returns 1 if 1 is GCD, 0 otherwise (with GCD in result)
 ***************************************************************************/
int bigint_gcd(bigint_t * parm1, bigint_t * parm2, bigint_t *result) {
 bigint_t *parm1_copy, *parm2_copy, *x, *y;
 int shift, rc;

   rc = 1;
   if (!bigint_nonzero(parm1)) {
      bigint_copy(result, parm2);
      return 1;
   }
   if (!bigint_nonzero(parm2)) {
      bigint_copy(result, parm1);
      return 1;
   }
   shift = bigint_ctz2(parm1, parm2);
   parm1_copy = bigint_init(32, 0);
   parm2_copy = bigint_init(32, 0);
   bigint_copy(parm1_copy, parm1);
   bigint_copy(parm2_copy, parm2);
   x = parm1_copy;
   y = parm2_copy;
   bigint_shift_ctz(x);
   bigint_shift_ctz(y);
   while (bigint_compare(x,y)) {
      if (bigint_compare(x, y) > 0) {
         x = y;
         if (y == parm1_copy) {
            y = parm2_copy;
         } else {
            y = parm1_copy;
         }
      }
      if (bigint_compare_int(x, 1) == 0) {
         if (!shift) {
            rc = 0;
         }
         break;
      }
      bigint_subtract(y, x);
      bigint_shift_ctz(y);
   }
   bigint_shift(x, 0 - shift);
   bigint_copy(result, x);
   bigint_free(parm1_copy);
   bigint_free(parm2_copy);
   return rc;
}

/*********************************************************************
 * bigint_bits()
 * Counts the number of ones in the bigint
 ********************************************************************/
unsigned int bigint_bits(bigint_t * parm1) {
 unsigned int i, bits, x;

   bits = 0;
   for (i=0; i<parm1->elements; i++) {
      x = (parm1->b[i] & 0x55555555) + ((parm1->b[i] & 0xAAAAAAAA) >> 1);
      x = (x & 0x33333333) + ((x & 0xCCCCCCCC) >> 2);
      x = (x & 0x0F0F0F0F) + ((x & 0xF0F0F0F0) >> 4);
      x = (x & 0x00FF00FF) + ((x & 0xFF00FF00) >> 8);
      bits += (x & 0x0000FFFF) + ((x & 0xFFFF0000) >> 16);
   }
   return bits;
}

/****************************************************************************
 * bigint_width()
 * Returns the "width" of a number as a count of significant bits
 ***************************************************************************/
unsigned int bigint_width(bigint_t * parm1) {
 unsigned int i, bits;
 int max_nonzero;

   bits = 0;
   max_nonzero = -1;
   i = 0;
   while (i < parm1->elements) {
      if (parm1->b[i]) {
         max_nonzero = i;
      }
      i++;
   }
   if (max_nonzero >= 0) {
      bits = (max_nonzero + 1) << SHIFTBITS;
      i = INTBITS - 1;
      while ((parm1->b[max_nonzero] & (1 << i)) == 0) {
         bits--;
         i--;
      }
   }
   return bits;
}

/****************************************************************************
 * bigint_sqrt() : result = sqrt(parm1)
 * Uses the "binary search" method
 ***************************************************************************/
void bigint_sqrt(bigint_t * parm1, bigint_t * result) {
 int width, cmp;
 bigint_t *max, *min, *product, *limit;

   width   = bigint_width(parm1);
   product = bigint_init(width, 0);
   limit   = bigint_init(width, 0);
   max     = bigint_init(width, 0);
   min     = bigint_init(32, 0);

   bigint_copy(max, parm1);
   if (width > 3) {
      bigint_shift(max, (width >> 1) - 1);
      bigint_copy(min, parm1);
      bigint_shift(min, (width >> 1) + 1);
   }

   bigint_add(min, max, result);
   bigint_shift(result, 1);
   bigint_mult(result, result, product);   
   bigint_add_int(min, 1, limit);

   while (bigint_compare(limit, max) < 0 &&
         (cmp = bigint_compare(product, parm1)) != 0) {

      if (cmp < 0) {
         bigint_copy(min, result);
         bigint_add_int(min, 1, limit);
      } else {
         bigint_copy(max, result);
      }
      bigint_add(min, max, result);
      bigint_shift(result, 1);
      bigint_mult(result, result, product);   
   }

   bigint_free(product);
   bigint_free(limit);
   bigint_free(max);
   bigint_free(min);
   return;
}
