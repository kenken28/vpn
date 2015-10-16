/*
   base64.c
   Aaron Logue 2009
*/
#include <stdio.h>
#include "base64.h"

/*
   base64_enc(source, length, destination, options)

   The caller can enable '=' termination with BASE64_EQTERM
   The caller can enable null termination with BASE64_NULLTERM
   Both terminations can be specified with BASE64_EQTERM+BASE64_NULLTERM
   The caller must provide a buffer large enough to receive the output.
   Four bytes of output are generated for every three bytes of input
   plus up to five more bytes of output depending on termination options
   and how many bytes of input were left over.
   The two buffers must be different.
*/
void base64_enc(unsigned char * src, int len, unsigned char * dst, int opts) {
   char base64[] = BASE64;
   int i;
   int n;
   int l = 0;	// line length

   /* Process all complete groups of 3 */
   for (i = 0; i <= (len-3); i = i + 3) {
      *dst++ = base64[((src[i]&0xFC) >> 2)];
      *dst++ = base64[((src[i]&0x03) << 4)   + ((src[i+1]&0xF0) >> 4)];
      *dst++ = base64[((src[i+1]&0x0F) << 2) + ((src[i+2]&0xC0) >> 6)];
      *dst++ = base64[src[i+2]&0x3F];
      if (opts & BASE64_CRLF) {
         l += 4;
         if (l >= 64) {
             *dst++ = '\r';
             *dst++ = '\n';
             l = 0;
         }
      }
   }
   /* Process leftovers */
   n = len - i;
   if (n > 0) {
      *dst++ = base64[(src[i]&0xFC) >> 2];
      l++;
      if (n == 1) {
         *dst++ = base64[(src[i]&0x03) << 4];
         l++;
         if (opts & BASE64_EQTERM) {
            *dst++ = '=';
            l++;
         }
      } else {
         *dst++ = base64[((src[i]&0x03) << 4) + ((src[i+1]&0xF0) >> 4)];
         l++;
         *dst++ = base64[(src[i+1]&0x0F) << 2];
         l++;
      }
   }
   if (opts & BASE64_EQTERM) {
      *dst++ = '=';
      l++;
   }
   if (opts & BASE64_CRLF) {
      if (l > 0) {
         *dst++ = '\r';
         *dst++ = '\n';
	  }
   }
   if (opts & BASE64_NULLTERM) {
      *dst = 0;
   }
   return;
}

/*
   base64_dec(source, destination)

   Three bytes of output are generated for every four bytes of input.
   The source and destination buffers can be the same.
   Decoding stops when a null or a '=' is encountered, or if len is
   not equal to -1, after len input characters have been processed.
   Return value: The number of bytes that were fully produced.
*/
int base64_dec(unsigned char * src, unsigned char * dst, int len) {
   unsigned char unbase64[] = { /* 64s are ignored, 65s kick us out */
      65,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
      52,53,54,55,56,57,58,59,60,61,64,64,64,65,64,64,
      64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
      15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
       0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
      41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
      64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
   };
   unsigned char bits;
   int i = 0;
   int state = 0;
   char * start = dst;

   while ((bits = unbase64[src[i]]) <= 64) {
	  if (len != -1 && i >= len) {
	     break;
	  }
      if (bits < 64) {
         switch (state) {
            case 0: *dst  = (bits & 0x3F) << 2; i++; state++; break;
            case 1: *dst |= (bits & 0x30) >> 4; dst++;
                    *dst  = (bits & 0x0F) << 4; i++; state++; break;
            case 2: *dst |= (bits & 0x3C) >> 2; dst++;
                    *dst  = (bits & 0x03) << 6; i++; state++; break;
            case 3: *dst |= (bits & 0x3F);      dst++; i++; state = 0; break;
         }
      } else {
         i++;
      }
   }
   return (int)((char*)dst - start);
}
