/********************************************************************
 * Aaron Logue 2013 www.cryogenius.com
 * Cryptographically secure pseudorandom number generator for Windows
 * based on the V1.3 Skein hash from
 * http://www.skein-hash.info/sites/default/files/NIST_CD_102610.zip
 *  Get these files from its Optimized_32bit directory:
 *    brg_endian.h brg_types.h skein_block.c skein.c skein.h
 *    skein_iv.h skein_port.h
 *******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "skein.h"
#include "skrand.h"

/*************************************************************
 * skrand_init()
 * Open a pseudorandom stream and optionally seed it with the
 * contents of the specified file. For the initial use, you
 * can create that file in a text editor with any gibberish.
 * F*&#&#$HH*#$**g_=]jsa@|d$JlsndaJGEJW8u333fhhw^jsd()o
 * would make a fine starting state. Additional calls to
 * Skein_512_Update using any other source of hard-to-predict
 * stuffcould be added here, such as system log files or the
 * entire contents of the cookie cache for desktops with users
 * that browse to a lot of different sites, etc.
 ************************************************************/
skrand_t * skrand_init(char *filename) {
 skrand_t * skrand;
   skrand = (skrand_t *)malloc(sizeof(skrand_t));
   if (skrand) {
      skrand->ctx = (Skein_512_Ctxt_t *)malloc(sizeof(Skein_512_Ctxt_t));
      Skein_512_Init(skrand->ctx, 512);
      skrand->remain = 0;
      if (skrand->ctx && filename && *filename) {
       FILE *fp;
         fp = fopen(filename, "rb");
         if (fp) {
            fread(skrand->buf, 1, 64, fp);
            Skein_512_Update(skrand->ctx, skrand->buf, 64);
            fclose(fp);
         }
      }
   }
   return skrand;
}

/*************************************************************
 * skrand_free()
 * Cycles the generator once and writes 64 bytes of binary
 * gibberish to an optional file for seeding the next use.
 ************************************************************/
void skrand_free(skrand_t *skrand, char *filename) {
   if (skrand) {
      if (skrand->ctx) {
         if (filename && *filename) {
          FILE *fp;
            Skein_512_Update(skrand->ctx, skrand->buf, 64);
            Skein_512_Final(skrand->ctx, skrand->buf);
            fp = fopen(filename, "wb");
            if (fp) {
               fwrite(skrand->buf, 1, 64, fp);
               fclose(fp);
            }
         }
         free(skrand->ctx);
      }
      free(skrand);
   }
}

/*************************************************************
 * skrand_read()
 * This works much like Linux's /dev/urandom
 * Each Skein hash block is 64 bytes in length. 16 bytes are
 * fed back into the hash state to perturb it for the next
 * block for each 48 bytes read by the caller.
 * Use skrand_read() if you want values from 0 to MAXINT.
 * Use skrand_int() if you want values from 0 to N-1.
 ************************************************************/
void skrand_read(skrand_t *skrand, char *var, int len) {
   if (skrand && skrand->ctx) {
      while (len) {
         if (skrand->remain == 0) {
            Skein_512_Final(skrand->ctx, skrand->buf);
            Skein_512_Update(skrand->ctx, &skrand->buf[48], 16);
            skrand->remain = 48;
         }
         skrand->remain--;
         *var++ = skrand->buf[skrand->remain];
         len--;
      }
   }
}

/*************************************************************
 * skrand_stir()
 * Call this with any entropy that you can get (anything from
 * the physical world such as timings between mouse clicks or
 * keystrokes or how long the human looked at a web page) to
 * perturb the hash's state with. Anything that varies and is
 * later hard to figure out what it might've been is good.
 ************************************************************/
void skrand_stir(skrand_t *skrand, char *var, int len) {
   if (skrand && skrand->ctx) {
      Skein_512_Update(skrand->ctx, var, len);
   }
}

/*************************************************************
 * skrand_int()
 * Return an evenly distributed unsigned int from 0 to N-1
 * For small values of N, this would probably suffice:
 *
 *  unsigned int result;
 *   skrand_read(skrand, (char *)&result, sizeof(unsigned int));
 *   return result % n;
 *   
 * However, consider that the range of values returned by the
 * RNG is from 0 to 0xFFFFFFFF. Think of what would happen if
 * N was 3/4s of the way to 0xFFFFFFFF. 3/4s of the time, reading
 * a random number would give us an evenly distributed value,
 * but 1/4 of the time, we would get a value that was only in
 * the bottom 1/3 of the possible range from 0 to N-1.
 *
 * One solution is to compute the cutoff point where modulo
 * remainders would start giving us biased results for a given N
 * and just try again if we get a random value that's greater than
 * the cutoff.  To minimize the number of tries (as in, so it
 * pretty much never happens even for large N) we can read twice
 * the bits we need so the RNG gives us from 0 to 0xFFFFFFFFFFFFFFFF
 * while N is only from 0 to 0xFFFFFFFF. The cutoff is max RNG value
 * minus max RNG mod N minus 1. For example, if N is 100 and the
 * max RNG valus is 0xFFFFFFFFFFFFFFFF, then we'd subtract 0x10
 * to get 0xFFFFFFFFFFFFFFEF and try again if the RNG ever returned
 * a value greater than that. This may look like it's splitting
 * hairs, but the bias does become measurable and possibly
 * exploitable for larger N.
 *
 * Here's an illustration of how this makes the modulo groups
 * all the same size if an RNG returned 4 bits 0-F or 5 bits 0-1F
 * and N=3 or N=7:
 *
 * RNG  N         cutoff  all modulo groups the same
 * 0F % 3 = 0, 0F-0-1=0E  012 345 678 9AB CDE
 * 1F % 3 = 1, 1F-1-1=1D  012 345 678 9AB CDE F01 234 567 89A BCD
 * 0F % 7 = 1, 0F-1-1=0D  0123456 789ABCD
 * 1F % 7 = 3, 1F-3-1=1B  0123456 789ABCD EF01234 56789AB
 *
 * TLDR: To roll a d6, do roll = skrand_int(skrand, 6) + 1; 
 ************************************************************/
unsigned int skrand_int(skrand_t *skrand, unsigned int n) {
#ifdef WIN32
 unsigned __int64 result, cutoff;
#else
 unsigned long long result, cutoff;
#endif
   if (n < 2) {
      return 0;
   }
   /* cutoff = 0xFFFFFFFFFFFFFFFF - (0xFFFFFFFFFFFFFFFF % n) - 1 */
#ifdef WIN32
   cutoff = (unsigned __int64)-1 - ((unsigned __int64)-1 % n) - 1;
#else
   cutoff = (unsigned long long)-1 - ((unsigned long long)-1 % n) - 1;
#endif
   do {
      skrand_read(skrand, (char *)&result, sizeof(result));
   } while (result > cutoff);
   return (unsigned int)(result % n);
}
