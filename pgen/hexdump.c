#include <stdio.h>

#if 1
void hexdump(unsigned char *ptr, int len) {
 int i;
 unsigned char *sptr = ptr;
 unsigned char *addr = 0;
   while (len) {
      printf("%08x  ", addr);
      for (i=0; i<16; i++) {
         if (i == 4 || i == 8 || i == 12) {
            printf(" ");
         }
         if (len > 0) {
            printf("%02x", *ptr++);
            addr++;
            len--;
         } else {
            printf("  ");
         }
      }
      printf(" |");
      for (i=0; i<16; i++) {
         if (sptr < ptr) {
            if (*sptr >= 32 && *sptr < 127) {
               printf("%c", *sptr);
            } else {
               printf(".");
            }
            sptr++;
         } else {
            printf(" ");
         }
      }
      printf("|\n");
   }
}
#else
#ifdef HEXDUMP_SIMPLE
void hexdump(unsigned char *ptr, int len) {
 int i;
 unsigned char *addr = 0;
   while (len) {
      printf("%08x  ", addr);
      for (i=0; i<16; i++) {
         if (len > 0) {
            printf("%02x ", *ptr++);
            addr++;
            len--;
         }
      }
      printf("\n");
   }
}
#endif

#define HEXDUMP_STANDALONE
#ifdef HEXDUMP_STANDALONE
unsigned int hexdump(unsigned char *ptr, int len, unsigned int addr) {
 int i;
 unsigned char *sptr = ptr;
   while (len) {
      printf("%08x  ", addr);
      for (i=0; i<16; i++) {
         if (i == 4 || i == 8 || i == 12) {
            printf(" ");
         }
         if (len > 0) {
            printf("%02x", *ptr++);
            addr++;
            len--;
         } else {
            printf("  ");
         }
      }
      printf(" |");
      for (i=0; i<16; i++) {
         if (sptr < ptr) {
            if (*sptr >= 32 && *sptr < 127) {
               printf("%c", *sptr);
            } else {
               printf(".");
            }
            sptr++;
         } else {
            printf(" ");
         }
      }
      printf("|\n");
   }
   return addr;
}

int main(int argc, char *argv[]) {
 FILE *fp;
 unsigned char buf[256];
 unsigned int num, addr = 0;

   if (argc != 2) {
      printf("Usage: %s <filename>\n");
      return 0;
   }
   fp = fopen(argv[1], "rb");
   if (fp == NULL) {
      printf("Could not open %s for input.\n", argv[1]);
      return 0;
   }
   while (num = fread(buf, 1, 256, fp)) {
      addr = hexdump(buf, num, addr);
   }
   fclose(fp);
   return 1;
}
#endif
#endif
