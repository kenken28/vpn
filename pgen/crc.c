/****************************************************************************
 * crc.c
 ***************************************************************************/

#define POLYNOMIAL 0x04c11db7L

unsigned long crc_table[256];

/****************************************************************************
 * init_crc_table()
 *
 *  generate the table of CRC remainders for all possible bytes
 ***************************************************************************/
void init_crc_table() {
 int i, j;
 unsigned long crc_accum;

   for ( i = 0;  i < 256;  i++ ) {
      crc_accum = ( (unsigned long) i << 24 );
      for (j = 0; j < 8; j++) {
         if ( crc_accum & 0x80000000L ) {
            crc_accum = ( crc_accum << 1 ) ^ POLYNOMIAL;
         } else {
            crc_accum = ( crc_accum << 1 );
         }
      }
      crc_table[i] = crc_accum;
   }
   return;
}
/****************************************************************************
 * update_crc()
 *
 *   Returns the updated value of the CRC accumulator after
 *   processing each byte in the addressed block of data.
 *   It is assumed that an unsigned long is at least 32 bits wide and
 *   that the predefined type char occupies one 8-bit byte of storage.
 ***************************************************************************/
unsigned long update_crc(unsigned long crc_accum,
         char * data_blk_ptr, int data_blk_size) {
 int i, j;
   for ( j = 0;  j < data_blk_size;  j++ ) {
      i = ( (int) ( crc_accum >> 24) ^ *data_blk_ptr++ ) & 0xff;
      crc_accum = ( crc_accum << 8 ) ^ crc_table[i];
   }
   return crc_accum;
}

/****************************************************************************
 * generate_crc()
 ****************************************************************************/
unsigned long generate_crc(char *buffer, unsigned long buffer_size)
{
 unsigned long crc = 0;

   crc = update_crc(crc, buffer, buffer_size);
   return crc;
}
