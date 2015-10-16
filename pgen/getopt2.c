/*
 * POSIX getopt for Windows
 *
 * AT&T Public License
 *
 * Code given out at the 1985 UNIFORUM conference in Dallas.  
 */

#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif
#include "getopt2.h"
// #include "winstring.h"

int	optind2 = 1;
char	*optarg2;

char getopt2(int argc, char **argv, char *opts) {
 static int sp2 = 1;
 char c;
 char *cp;

   if (sp2 == 1) {
      if (optind2 >= argc || argv[optind2][0] == '\0') {
         return -1;
      }
      if (argv[optind2][0] != '-') {
         optarg2 = argv[optind2];
         optind2++;
         return '*';
      }
      if (strcmp(argv[optind2], "--") == 0) {
         optind2++;
         return -1;
      }
   }
   c = argv[optind2][sp2];
   if (c == ':' || (cp = strchr(opts, c)) == NULL) {
      fprintf(stderr, "undefined option: %c\n", c);
      if (argv[optind2][++sp2] == '\0') {
         optind2++;
         sp2 = 1;
      }
	   return '?';
   }
   if (*++cp == ':') {
      if (argv[optind2][sp2+1] != '\0') {
         optarg2 = &argv[optind2++][sp2+1];
      } else {
         if (++optind2 >= argc) {
            fprintf(stderr, "option requires parameter: %c\n", c);
            sp2 = 1;
            return '?';
         } else {
            optarg2 = argv[optind2++];
         }
      }
      sp2 = 1;
   } else {
      if (argv[optind2][++sp2] == '\0') {
         sp2 = 1;
         optind2++;
      }
      optarg2 = NULL;
   }
   return c;
}
