#include <stdlib.h>

#if 0
/*****************************************************************************
 * strchr for windows
 ****************************************************************************/
char * strchr(char *sptr, char c) {
   while (*sptr) {
      if (*sptr == c) {
         return sptr;
      }
      sptr++;
   }
   return 0;
}

/*****************************************************************************
 * strcmp for windows
 ****************************************************************************/
int strcmp(unsigned char * str, unsigned char * buf) {
   while (*str && *buf) {
      if (*str < *buf) {
         return -1;
      }
      if (*str > *buf) {
         return 1;
      }
      str++;
      buf++;
   }
   if (!*str && *buf) {
      return -1;
   }
   if (*str && !*buf) {
      return 1;
   }
   return 0;
}
#endif

/*****************************************************************************
 * strcasecmp for windows
 ****************************************************************************/
int strcasecmp(unsigned char * str, unsigned char * buf) {
 unsigned char c1, c2;
   while (*str && *buf) {
      c1 = tolower(*str);
      c2 = tolower(*buf);
      if (c1 < c2) {
         return -1;
      }
      if (c1 > c2) {
         return 1;
      }
      str++;
      buf++;
   }
   if (!*str && *buf) {
      return -1;
   }
   if (*str && !*buf) {
      return 1;
   }
   return 0;
}

/*****************************************************************************
 * strncasecmp, since windows is lacking
 ****************************************************************************/
int strncasecmp(unsigned char * str, unsigned char * buf, int n) {
 unsigned char c1, c2;
   while (*str && n) {
      c1 = tolower(*str);
      c2 = tolower(*buf);
      if (c1 < c2) {
         return -1;
      }
      if (c1 > c2) {
         return 1;
      }
      str++;
      buf++;
      n--;
   }
   return 0;
}
