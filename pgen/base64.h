#ifndef BASE64_INCLUDE
#define BASE64_INCLUDE 1

#define BASE64_EQTERM   1
#define BASE64_NULLTERM 2
#define BASE64_CRLF     4

#define BASE64          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

void base64_enc(unsigned char * src, int len, unsigned char * dst, int opts);
int  base64_dec(unsigned char * src, unsigned char * dst, int len);

#endif
