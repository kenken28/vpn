/*****************************************************************************
 * Perform a mock Diffie-Hellman key exchange using a big prime
 ****************************************************************************/
#include <stdio.h>
#ifdef WIN32
#include "skein.h"
#include <windows.h>
#endif
#include "bigint.h"

#if 1
#define G 5
#define P "E53DF5FC3F650D066875837012A4E7BEA863C65CB592D9C36942CF69CBC6DD4F"\
          "D804E19CCF2696C9BEBCF18742FA5FB091CBDE1782E8291009464913ECE19745"\
          "7800EA6E43B0E2A64615D182B6DE150479C58D1C7C702D47EA3031B379CA13A2"\
          "048C964E1D1E8D4CD3815D0895BF31E53271D4607E16461B77FB26100915D679"\
          "9060203EDEBFEA9495A5A8E7CED68FC9DB2D47CE7992461BA78174608AD0BBE3"\
          "F5E63EC6C960564430CBD2E6E587D08EE12F94B5B99DFFB12C6727A25E800DAC"\
          "6CD8DE77A5BBC93B36E444B070888CB5ADD991870466968A6E9A23C2EE0A1D67"\
          "1C9B601081A44AA6A58D4DC76686EF15FCE1C9AEB4033395A9B24BE1AA1929BB"
#else
#define G 11
#define P "B4638E23F160E61CA0CE0C3EFF9BEA7EDD7174CB217DB9EDEA83E3C65815C6ED"\
          "5CF7068309B88C0930C97C12E33621E60A420554AE0166FA2E2FD46710EB9874"\
          "FE8365379F77206C4E8DAA114753534A2EC81F37EC3C5994024F31C857FBF9C1"\
          "413476670235DD59AF7EEFA5BF225C94875F3D84066BEBE43EE92A72685FDD39"\
          "A2DDFE16EFE69554005F15F6B09647673FC471E359A24FB0320CEE0EB6948221"\
          "E94D1D72E0C4E4E73F99121D113269540275173A256C6AA23A35AF8CDD6E7BB6"\
          "703500515455FDDB897B475F890E90E48D003428D96298AEFCA2A62A66E492C2"\
          "7FB0F04E03CB303D57FBC2EC24F02EB99F7F8D2409D5C11ED3D957B484851F69"\
          "AE00EAA1CCD816F17F38D773C137E77F62C6B0FF9C5A8AE22AE6B818C9F82D47"\
          "8DF17BBC0FC6755E3A9DB69CAA344BD8F4A4341F6AA541A8043A4EB7F21A5927"\
          "5419BDA5E1D38C3778AF49569F2609BCB7E6FC82C5B6AB7AD087ABB6F7A208A5"\
          "CF44C45E2DDE192FEF430A6D4B0AB437A984AF6FC160E6A2BC712524FE7EDC0D"\
          "E4AC72EF8812498791546B60FB458D5F99474024BD1A10E7BBEF5C6003CB8A73"\
          "E0535B4E0F6ABD4A8A9F424DDD5716840EB224EC5EBAF41D67DEAD6DDBCD8923"\
          "C4CBE03C9A85DFE96E240497961229601573757B9F6B1054855DDEA262198C68"\
          "9D6C1F4B4B9A764B33E7F7089CF46C632E27ECF3DDBE46B94FEBA7741943D483"
#endif

int main(int argc, char *argv[]) {
 bigint_t *g, *p, *x, *y, *X, *Y, *a, *b;
#ifdef WIN32
unsigned long ticks = GetTickCount();
#endif
   g = bigint_init(32, G); p = bigint_init(32, 0); x = bigint_init(32, 0);
   y = bigint_init(32, 0); X = bigint_init(32, 0); Y = bigint_init(32, 0);
   a = bigint_init(32, 0); b = bigint_init(32, 0); bigint_atoi(p, P, 16);

   bigint_random(x, bigint_width(p) / 4);
   printf("Alice makes up secret number x ...%08x%08x\n", x->b[1], x->b[0]);
   bigint_pow_mod(g, x, p, X);
   printf("Alice computes X = g^x mod p = ...%08x%08x\n", X->b[1], X->b[0]);

   bigint_random(y, bigint_width(p) / 4);
   printf("Bob makes up secret number y   ...%08x%08x\n", y->b[1], y->b[0]);
   bigint_pow_mod(g, y, p, Y);
   printf("Bob computes   Y = g^y mod p = ...%08x%08x\n", Y->b[1], Y->b[0]);

   bigint_pow_mod(Y, x, p, a);
   printf("Alice computes a = Y^x mod p = ...%08x%08x\n", a->b[1], a->b[0]);
   bigint_pow_mod(X, y, p, b);
   printf("Bob computes   b = X^y mod p = ...%08x%08x\n", b->b[1], b->b[0]);

   if (bigint_compare(a, b) == 0) {
      printf("Alice and Bob now share a %d-bit secret.\n", bigint_width(a));
   } else {
      printf("Now what we have here is a failure to communicate.\n");
   }
   bigint_free(g); bigint_free(p); bigint_free(x); bigint_free(y);
   bigint_free(X); bigint_free(Y); bigint_free(a); bigint_free(b);
#ifdef WIN32
printf("Runtime: %lu ticks\n", GetTickCount() - ticks);
#endif
   return 1;
}
