#ifndef BIGINT_INCLUDE
#define BIGINT_INCLUDE 1

/* Platform dependent stuff */
#ifdef WIN32
typedef unsigned long int_type;        /* 32 bits */
typedef unsigned __int64 int2x_type;   /* 64 bits */
#else
typedef unsigned int int_type;         /* 32 bits */
typedef unsigned long int2x_type;      /* 64 bits */
#endif

/* SHIFTBITS = how many right shifts would make sizeof(int_type) = 1 */
#define SHIFTBITS 5
#define INTBITS   (sizeof(int_type)*8)
#define MAXINT    ((int_type)(-1))

/* Macros to set, clear, or test bit n of int_type array b */
#define SET_BIT(b,n) b[(n)>>SHIFTBITS]|=(1<<((n)&(INTBITS-1)))
#define CLR_BIT(b,n) b[(n)>>SHIFTBITS]&=~(1<<((n)&(INTBITS-1)))
#define IS_BIT(b,n)  (b[(n)>>SHIFTBITS]&(1<<((n)&(INTBITS-1))))

typedef struct {
   int_type * b;
   unsigned int elements;
} bigint_t;

/****************************************************************************
 * bigint_init() : Allocate a new bigint and set it to a small integer value
 ***************************************************************************/
bigint_t * bigint_init(unsigned int bits, int_type val);

/****************************************************************************
 * bigint_free()
 ***************************************************************************/
void bigint_free(bigint_t * var);

/****************************************************************************
 * bigint_dump() : printf bigint as hex in 32-bit blocks
 ***************************************************************************/
void bigint_dump(char *name, bigint_t * var);

/****************************************************************************
 * bigint_extend() : Extend var by some number of elements
 ***************************************************************************/
void bigint_extend(bigint_t * var, int elements);

/****************************************************************************
 * bigint_set() : Set a bigint to a small integer value
 ***************************************************************************/
void bigint_set(bigint_t * var, int_type val);

/****************************************************************************
 * bigint_random() : Set bigint to a random value (high bit not guaranteed)
 ***************************************************************************/
void bigint_random(bigint_t * var, unsigned int bits);

/****************************************************************************
 * bigint_random_n() : Set result to value from 0 to N-1 with uniform dist.
 ***************************************************************************/
void bigint_random_n(bigint_t * result, bigint_t * n);

/****************************************************************************
 * bigint_copy() : dest = source
 ***************************************************************************/
void bigint_copy(bigint_t * dest, bigint_t * source);

/****************************************************************************
 * bigint_add() : result = parm1 + parm2
 * result can be the same as either source
 ***************************************************************************/
void bigint_add(bigint_t * parm1, bigint_t * parm2, bigint_t * result);
 
/****************************************************************************
 * bigint_add_int() : result = parm1 + parm2
 * result can be the same as parm1
 ***************************************************************************/
void bigint_add_int(bigint_t * parm1, unsigned int parm2, bigint_t * result);

/****************************************************************************
 * bigint_subtract() : parm1 = parm1 - parm2
 * Negative numbers are undefined; borrows from beyond MSB are dropped.
 ***************************************************************************/
void bigint_subtract(bigint_t * parm1, bigint_t * parm2);

/****************************************************************************
 * bigint_subtract_int() : parm1 = parm1 - parm2
 * Negative numbers are undefined; borrows to MSB 0 are dropped.
 ***************************************************************************/
void bigint_subtract_int(bigint_t * parm1, unsigned int parm2);

/****************************************************************************
 * bigint_mult() : result = parm1 * parm2
 * Result must be distinct from parm1 or parm2.
 ***************************************************************************/
void bigint_mult(bigint_t * parm1, bigint_t * parm2, bigint_t * result);

/****************************************************************************
 * bigint_mult_int() : result = parm1 * parm2
 * Result can be the same as parm1.
 ***************************************************************************/
void bigint_mult_int(bigint_t * parm1, unsigned int parm2, bigint_t * result);

/****************************************************************************
 * bigint_divide() : result = parm1 / parm2
 * Result must be distinct from parm1 or parm2
 ***************************************************************************/
void bigint_divide(bigint_t * parm1, bigint_t * parm2, bigint_t * result);
	
/****************************************************************************
 * bigint_divide_int() : result = parm1 / parm2
 * Result can be the same as parm1.
***************************************************************************/
void bigint_divide_int(bigint_t * parm1, unsigned int parm2, bigint_t * result);

/****************************************************************************
 * bigint_nonzero() : Return 1 if bigint is nonzero
 ***************************************************************************/
int bigint_nonzero(bigint_t * var);

/****************************************************************************
 * bigint_compare()
 * Return -1 if parm1 < parm2, 1 if parm1 > parm2, and 0 if parm1 == parm2
 ***************************************************************************/
int bigint_compare(bigint_t * parm1, bigint_t * parm2);

/****************************************************************************
 * bigint_compare_int()
 * Return -1 if parm1 < parm2, 1 if parm1 > parm2, and 0 if parm1 == parm2
 ***************************************************************************/
int bigint_compare_int(bigint_t * parm1, unsigned int parm2);

/****************************************************************************
 * bigint_shift() : parm1 = parm1 << -shift  or  parm1 = parm1 >> shift
 * Negative values shift parm1 left, positive values shift parm1 right
 * You can do big shifts; ie: -128
 ***************************************************************************/
void bigint_shift(bigint_t * parm1, int shift);

/****************************************************************************
 * bigint_mod() : result = parm1 % parm2
 ***************************************************************************/
void bigint_mod(bigint_t * parm1, bigint_t * parm2, bigint_t * result);

/****************************************************************************
 * bigint_pow() : result = base ^ exp
 * Be careful with large exponents; this function could consume all memory.
 ***************************************************************************/
void bigint_pow(bigint_t * base, bigint_t * exp, bigint_t * result);

/****************************************************************************
 * bigint_pow_mod() : result = base ^ exp % mod
 * This is the one to use; it handles large exponents without consuming all.
 ***************************************************************************/
void bigint_pow_mod(bigint_t * base, bigint_t * exp, bigint_t * mod,
                    bigint_t * result);

/****************************************************************************
 * bigint_rabin_miller_int() : Performs one Rabin-Miller / Miller-Rabin test
 ***************************************************************************/
int bigint_rabin_miller_int(int a, bigint_t * n);

/****************************************************************************
 * bigint_rabin_miller() : Performs one Rabin-Miller / Miller-Rabin test
 ***************************************************************************/
int bigint_rabin_miller(bigint_t * a, bigint_t * n);

/****************************************************************************
 * bigint_is_prime() : Performs multiple Rabin-Miller / Miller-Rabin tests
 * See code for how to use the certainty parm to do more or less testing.
 * certainty=3 will produce proven 32-bit primes. certainty=7 will produce
 * proven 48-bit primes. certainty=57 will perform tests with the first
 * 7 primes then perform rabin-miller tests using 50 random numbers less
 * than n.  A 2Ghz laptop should be able to produce a 1024-bit prime in
 * a few minutes using bigint_random and this function.
 ***************************************************************************/
int bigint_is_prime(bigint_t * n, int certainty);

/****************************************************************************
 * bigint_atoi() : Convert strings to bigints using any base from 2 to 16
 ***************************************************************************/
void bigint_atoi(bigint_t *result, char *str, unsigned int base);

/****************************************************************************
 * bigint_itoa() : Convert bigints to strings using any base from 2 to 16
 * Example: str = bigint_itoa(n, 10);
 * NOTE: calls malloc. Caller is responsible for freeing returned string.
 ***************************************************************************/
char * bigint_itoa(bigint_t * n, unsigned int base);

/****************************************************************************
 * bigint_print()
 * Output a bigint as an ASCII string
 ***************************************************************************/
void bigint_print(bigint_t * var, unsigned int base);

/****************************************************************************
 * bigint_gcd() : Find greatest common divisor
 * Returns 0 if no GCD other than 1 found. Otherwise, 1 with result = GCD.
 ***************************************************************************/
int bigint_gcd(bigint_t * parm1, bigint_t * parm2, bigint_t *result);

/****************************************************************************
 * bigint_bits()
 * Counts the ones
 ***************************************************************************/
unsigned int bigint_bits(bigint_t * parm1);

/****************************************************************************
 * bigint_width()
 * Returns the "width" of a number as a count of significant bits
 ***************************************************************************/
unsigned int bigint_width(bigint_t * parm1);

/****************************************************************************
 * bigint_sqrt() : result = sqrt(parm1)
 ***************************************************************************/
void bigint_sqrt(bigint_t *parm1, bigint_t *result);

#endif
