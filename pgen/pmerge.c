/*****************************************************************************
 * Aaron Logue 2013
 * merge_primes - Load primes from two files, discard dupes, and output.
 * This program can also hunt down all of the relevant primes.proof entries
 * for a given prime as well as display counts and display a prime in either
 * decimal or hex.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "crc.h"
#include "avlc.h"
#include "bigint.h"
#include "getopt2.h"
#ifdef WIN32
#include "winstring.h"
#endif

#define LIST_PRIMES 1

typedef struct {
   unsigned int bits;
   unsigned int crc;
   unsigned int base;
   char skey[16];
   char *sptr;
} prime_t;

typedef struct {
   FILE *fp;    /* Open output file */
   int outbase; /* base to translate primes to */
   bigint_t *p; /* bigint, so we don't have to bigint_init every write */
} prime_walk_t;

typedef struct {
   int bits;
   int count;   /* How many primes of this bit width we have found */
} stats_t;

typedef struct proof_tt {
   bigint_t *p;
   bigint_t *q;
   bigint_t *r;
} proof_t;

/*****************************************************************************
 * load_primes()
 * Load all primes from input file into primes tree
 ****************************************************************************/
void load_primes(avlc_t *primes, avlc_t *stats, char *infile, int *inbase,
                 unsigned int bits) {
 char *buf, *sptr, *dptr;
 bigint_t *candidate;
 prime_t *prime;
 stats_t *stat;
 FILE *fp;

   fp = fopen(infile, "r");
   if (!fp) {
      printf("%s not found.\n", infile);
      return;
   }
   candidate = bigint_init(32, 0);
   buf = (char *)malloc(50000);
   if (buf) {
      while (1) {
         if (fgets(buf, 9999, fp)) {
            prime = malloc(sizeof(prime_t));
            if (prime) {
               dptr = prime->sptr = malloc(strlen(buf) + 1);
               if (dptr) {
                  sptr = buf;
                  while (*sptr) {
                     *sptr = toupper(*sptr);
                     if (strchr("0123456789ABCDEF", *sptr)) {
                        *dptr++ = *sptr;
                        if (*sptr >= 'A') {
                           *inbase = 16;
                        }
                     }
                     sptr++;
                  }
                  *dptr = 0;
               }
               bigint_atoi(candidate, prime->sptr, *inbase);
               prime->bits = bigint_width(candidate);
               prime->crc  = generate_crc(prime->sptr, strlen(prime->sptr));
               prime->base = *inbase;
               sprintf(prime->skey, "%05d%08x", prime->bits, prime->crc);
               
               if ((!bits || prime->bits == bits) &&
                     !avlc_get(primes, prime->skey)) {
                  avlc_add(primes, prime->skey, prime);

                  /* Update count of primes by bit width */
                  sprintf(buf, "%05d", prime->bits);
                  stat = (stats_t *)avlc_get(stats, buf);
                  if (stat) {
                     stat->count++;
                  } else {
                     stat = (stats_t *)malloc(sizeof(stats_t));
                     if (stat) {
                        stat->bits  = prime->bits;
                        stat->count = 1;
                        avlc_add(stats, buf, stat);
                     }
                  }
               } else {
                  free(prime->sptr);
                  free(prime);
               }
            }
         } else {
            break;
         }
      }
      free(buf);
   }
   bigint_free(candidate);
   fclose(fp);
}

/*****************************************************************************
 * print_prime() - callback function called by avlc_walk_parm()
 ****************************************************************************/
int print_prime(char * key, void * value, void * parm) {
 prime_t *prime;
 prime_walk_t *walk;
 char *sptr;

   prime = (prime_t *)value;
   walk  = (prime_walk_t *)parm;

   if (prime && walk) {
      bigint_atoi(walk->p, prime->sptr, prime->base);
      sptr = bigint_itoa(walk->p, walk->outbase);
      if (sptr) {
         if (strlen(sptr) > 8) {
            sptr[8] = 0;
         }
         printf("%d\t%s...\n", bigint_width(walk->p), sptr);
         free(sptr);
      }
   }
   return 0; /* keep walking */
}

/*****************************************************************************
 * write_prime()
 ****************************************************************************/
void write_prime(FILE * fp, bigint_t *p, int outbase) {
 char *sptr;

   if (fp && p && outbase) {
      sptr = bigint_itoa(p, outbase);
      fwrite(sptr, 1, strlen(sptr), fp);
      free(sptr);
      fwrite("\n", 1, 1, fp);
   }
   return;
}

/*****************************************************************************
 * unload_prime() - callback function called by avlc_walk_parm()
 ****************************************************************************/
int unload_prime(char * key, void * value, void * parm) {
 prime_t *prime;
 prime_walk_t *walk;

   prime = (prime_t *)value;
   walk  = (prime_walk_t *)parm;

   if (prime && walk) {
      bigint_atoi(walk->p, prime->sptr, prime->base);
      write_prime(walk->fp, walk->p, walk->outbase);
   }
   return 0; /* keep walking */
}

/*****************************************************************************
 * free_prime() - callback function called by avlc_walk()
 ****************************************************************************/
int free_prime(char * key, void * value) {
 prime_t *prime;
   prime = (prime_t *)value;
   if (prime) {
      if (prime->sptr) {
        free(prime->sptr);
      }
      free(prime);
   }
   return 0;
}

/*****************************************************************************
 * print_stat() - callback function called by avlc_walk()
 ****************************************************************************/
int print_stat(char * key, void * value) {
 stats_t * stat;
   stat = (stats_t *)value;
   if (stat) {
      printf("%d\t%d\n", stat->bits, stat->count);
   }
   return 0;
}

/*****************************************************************************
 * free_stat() - callback function called by avlc_walk()
 ****************************************************************************/
int free_stat(char * key, void * value) {
   if (value) {
      free(value);
   }
   return 0;
}

/*****************************************************************************
 * Returns pointer to first token in primes.proof record
 ****************************************************************************/
char * first_token(char * tok) {
   while (*tok && (*tok == ',' || *tok == ' ' || *tok == '\t' ||
                   *tok == '\n' || *tok == '\r')) {
      tok++;
   }
   return(tok);
}

/*****************************************************************************
 * Returns pointer to next token in primes.proof record
 ****************************************************************************/
char * next_token(char * tok) {
   while (*tok && *tok != ',' && *tok != ' ' && *tok != '\t' &&
                  *tok != '\n' && *tok != '\r') {
      tok++;
   }
   return(first_token(tok));
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
 * find_proof()
 * Hunt for string pointed at by sptr in file at beginning of line.
 * If found, return a structure containing (p,q,r) where p = 2qr+1
 * Note: WIN32 version is using strncasecmp in avlc.c
 * Caller must free returned proof_t and bigints
 ****************************************************************************/
proof_t * find_proof(char *n, char *fproofs, int *inbase) {
 FILE *fp;
 proof_t *p;
 char *sptr, *dptr, *buf, *rc;
 char c;

   p = NULL;
   buf = (char *)malloc(50000);
   if (!buf) {
      return NULL;
   }
   fp = fopen(fproofs, "r");
   if (!fp) {
      printf("File %s not found.\n", fproofs);
      free(buf);
      return NULL;
   }
   do {
      rc = fgets(buf, 49999, fp);
      if (rc) {
         sptr = first_token(buf);
         if (!strncasecmp(n, sptr, strlen(n))) {

            if (*inbase == 10) {
               /* Test for hex in the row. If any, switch input base to hex */
               dptr = sptr;
               while (*dptr) {
                  if (strchr("ABCDEFabcdef", *dptr)) {
                     *inbase = 16;
                     break;
                  }
                  dptr++;
               }
            }

            p = (proof_t *)malloc(sizeof(proof_t));
            if (p) {
               p->p = bigint_init(32, 0);
               p->q = bigint_init(32, 0);
               p->r = bigint_init(32, 0);
               dptr = sptr;
               while (*dptr && *dptr != ',') {
                  dptr++;
               }
               c = *dptr;
               *dptr = 0;
               bigint_atoi(p->p, sptr, *inbase);
               *dptr = c;
               sptr = next_token(sptr); /* should be 2 */
               sptr = next_token(sptr);
               dptr = sptr;
               while (*dptr && *dptr != ',') {
                  dptr++;
               }
               c = *dptr;
               *dptr = 0;
               bigint_atoi(p->q, sptr, *inbase);
               *dptr = c;
               sptr = next_token(sptr);
               dptr = sptr;
               while (*dptr && *dptr != ',') {
                  dptr++;
               }
               c = *dptr;
               *dptr = 0;
               bigint_atoi(p->r, sptr, *inbase);
               rc = 0;
            }
         }
      }
   } while (rc);
   fclose(fp);
   free(buf);
   return p;
}

/*****************************************************************************
 * list_proofs()
 * Use find_proof() to find p=2qr+1 rows, then recurse self to get q and r.
 ****************************************************************************/
int list_proofs(char *n, char *fproofs, char *outfile,
                int *inbase, int outbase, int top) {
 proof_t *proof;
 bigint_t *bn;
 char *p, *q, *r;

   proof = find_proof(n, fproofs, inbase);
   if (!proof && !top && *inbase == 10) {
      /* try same number in hex */
      bn = bigint_init(32, 0);
      bigint_atoi(bn, n, 10);                  /* Convert from base 10 */
      p = bigint_itoa(bn, 16);             /* to base 16           */
      *inbase = 16;
      proof = find_proof(p, fproofs, inbase);
      if (!proof) {
         *inbase = 10;
      }
      free(p);
      bigint_free(bn);
   }
   if (proof) {
      p = bigint_itoa(proof->p, outbase);
      q = bigint_itoa(proof->q, outbase);
      r = bigint_itoa(proof->r, outbase);
      if (strlen(p) > 8) {
         p[8] = 0;
      }
      if (strlen(q) > 8) {
         q[8] = 0;
      }
      if (strlen(r) > 8) {
         r[8] = 0;
      }
      printf("Found %d-bit p %s... %d-bit q %s... %d-bit r %s...\n",
         bigint_width(proof->p), p, bigint_width(proof->q), q,
         bigint_width(proof->r), r);
      free(p);
      free(q);
      free(r);

      write_proof(outfile, proof->p, proof->q, proof->r, outbase);

      q = bigint_itoa(proof->q, *inbase);
      list_proofs(q, fproofs, outfile, inbase, outbase, 0);
      free(q);
      r = bigint_itoa(proof->r, *inbase);
      list_proofs(r, fproofs, outfile, inbase, outbase, 0);
      free(r);
   }
   return 1;
}

/*****************************************************************************
 * find_prime
 * Returns best guess of base of found string
 ****************************************************************************/
prime_t * find_prime(char *n, char *fprimes) {
 FILE *fp;
 prime_t *prime;
 int inbase = 10;
 char *sptr, *dptr, *buf, *rc;

   prime = NULL;
   buf = (char *)malloc(50000);
   if (!buf) {
      return NULL;
   }
   fp = fopen(fprimes, "r");
   if (!fp) {
      printf("File %s not found.\n", fprimes);
      free(buf);
      return NULL;
   }
   do {
      rc = fgets(buf, 49999, fp);
      if (rc) {
         sptr = first_token(buf);
         if (!strncasecmp(n, sptr, strlen(n))) {
            /* Test for hex in the row. If any, switch input base to hex */
            dptr = sptr;
            while (*dptr) {
               if (strchr("ABCDEFabcdef", *dptr)) {
                  inbase = 16;
                  break;
               }
               dptr++;
            }
            prime = (prime_t *)malloc(sizeof(prime_t));
            if (prime) {
               prime->base = inbase;
               prime->sptr = (char *)malloc(strlen(buf) + 1);
               if (prime->sptr) {
                  strcpy(prime->sptr, buf);
               }
            }
            rc = 0;
         }
      }
   } while (rc);
   fclose(fp);
   free(buf);
   return prime;
}

/*****************************************************************************
 * display_prime
 ****************************************************************************/
int display_prime(char *n, char *fprimes, int outbase, int width) {
 prime_t *prime;
 bigint_t * p;
 char *sptr;
 int i;

   prime = find_prime(n, fprimes);
   if (prime && prime->sptr) {
      p = bigint_init(32, 0);
      if (p) {
         bigint_atoi(p, prime->sptr, prime->base);
         sptr = bigint_itoa(p, outbase);
         printf("%d-bits wide, %d characters, base %d:\n",
            bigint_width(p), strlen(sptr), outbase);
         if (!width) {
            printf("%s\n", sptr);
         } else {
            i = 0;
            while (*sptr) {
               printf("%c", *sptr++);
               i++;
               if (i == width) {
                  i = 0;
                  printf("\n");
               }
            }
            if (i) {
               printf("\n");
            }
         }
         bigint_free(p);
      }
   }
   return 1;
}


/*****************************************************************************
 * show_usage()
 ****************************************************************************/
void show_usage(char *me) {
   printf("%s <infile> [<infile2>] [-w <outfile>] [-l] [-b <bits>] [-n <base>]\n", me);
   printf("or\t-p <prime> <proofsfile> [-w <outfile>] [-n <base>]\n");
   printf("or\t-d <prime> <infile> [-o <base>] [-s <width>]\n");
   printf("\n");
   printf("\t-w\tSpecify optional output file (omit to display stats)\n");
   printf("\t-b\tIf specified, loads only primes of this width\n");
   printf("\t-o\tIf specified, outputs primes using this number base\n");
   printf("\t-p\tExtract proof records for prime beginning with this\n");
   printf("\t-l\tList first 8 digits of primes in file\n");
   printf("\t-d\tDisplay full prime that begins with this\n");
   printf("\t-s\tBreak lines of full prime display at this width\n");

   printf("\n");
   printf("Examples:\n");
   printf("%s primes\t\t- Count unique primes in file 'primes'\n", me);
   printf("%s primes primes2\t\t- Count unique primes in both files\n", me);
   printf("%s primes prime2 -w prime3\t- Merge two files to 'primes3'\n", me);
   printf("%s -p 735922 proofs.380\t- Extract 735922...'s proof\n", me);
   printf("%s -d 7359 primes -o 16\t- Display 7359... as hex\n", me);
   return;
}

/*****************************************************************************
 * main()
 ****************************************************************************/
int main(int argc, char *argv[]) {
 unsigned int bits, outbase, base1, base2, flags, width;
 char c;
 char *infile1, *infile2, *outfile, *proof, *display;
 avlc_t *primes, *stats;
 prime_walk_t prime;

   infile1  = NULL;
   infile2  = NULL;
   outfile  = NULL;
   base1    = 10;
   base2    = 10;
   outbase  = 10;
   bits     = 0;
   proof    = NULL;
   display  = NULL;
   flags    = 0;
   width    = 0;

   while ((c = getopt2(argc,argv,"ld:w:p:b:o:s:")) > 0) {
      switch(c) {
       case 'l':
         flags |= LIST_PRIMES;
         break;
       case 'p':
         proof = optarg2;
         break;
       case 'd':
         display = optarg2;
         break;
       case '*':
         if (!infile1) {
            infile1 = optarg2;
         } else {
            if (!infile2) {
               infile2 = optarg2;
            }
         }
         break;
       case 'w':
         outfile = optarg2;
         break;
       case 'b':
         bits = atoi(optarg2);
         if (bits < 1) {
            bits = 1;
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
       case 's':
         width = atoi(optarg2);
         break;
       case '?':
       default:
         show_usage(argv[0]);
         return 0;
      }
   }

   if (!infile1 || !*infile1) {
      show_usage(argv[0]);
      return 0;
   }
   if (proof && *proof) {
      /* Append proofs for number in proof string to outfile */
      list_proofs(proof, infile1, outfile, &base1, outbase, 1);
      return 0;
   }
   if (display && *display) {
      display_prime(display, infile1, outbase, width);
      return 0;
   }

   init_crc_table();
   primes = avlc_init(0);
   stats  = avlc_init(0);

   base1 = 10;
   load_primes(primes, stats, infile1, &base1, bits);
   if (infile2 && *infile2) {
      base2 = 10;
      load_primes(primes, stats, infile2, &base2, bits);
   }

   if (stats->count) {
      if (flags & LIST_PRIMES) {
         prime.outbase = outbase;
         prime.p       = bigint_init(32, 0);
         if (prime.p) {
            printf("bits\tfirst 8 digits\n");
            avlc_walk_parm(primes, &print_prime, &prime);
            bigint_free(prime.p);
         }
      } else {
         printf("bits\tprimes\n");
         avlc_walk(stats, &print_stat);
      }
   }

   if (outfile && *outfile && primes->count) {
      /* Set up structure with stuff to pass to callback walker function */
      prime.fp         = fopen(outfile, "a");
      if (prime.fp) {
         prime.outbase = outbase;
         prime.p       = bigint_init(32, 0);
         if (prime.p) {
            avlc_walk_parm(primes, &unload_prime, &prime); 
            bigint_free(prime.p);
         }
         fclose(prime.fp);
      }
   }

   avlc_walk(stats,  &free_stat);
   avlc_free(stats);
   avlc_walk(primes, &free_prime);
   avlc_free(primes);
   return 1;
}
