#include <stdio.h>
#include <string.h>
#include <stdint.h>

unsigned int unk_A7AF048 = 1;
unsigned int unk_A7AF044 = 1;
int amDongleDebugLevel = 1;

#define BYTE1(x) x
#define LOBYTE(x) ((x) & 0xFF)
#define HIBYTE(x) (((x)>>8) & 0xFF)



unsigned int am44 = 0;

int amDongleContext = 1;

signed int __cdecl amDongleSend(uint8_t data) {

unsigned int step = 1;

//ABCDEFGH
// Step 1/2: High nibble (ABCD)
//           1A: A 1B: B
//           2A: C 2B: D
// Step 3/4: Low nibble (EFGH)
//           3A: E 3B: F
//           4A: G 4B: H

  printf("\n\nSending 0x%02X ('%c')\n",a1,a1);
  uint32_t v4; // eax@4
  signed int res; // ecx@9
  char v8; // [sp+8h] [bp-10h]@4

  if ( !amDongleContext ) {
    return -5;
  }
  signed int crumbIndex = 0;
  signed int bitOffset = 6;
  while ( 1 ) {

    // Check if the dongle does something at the moment, abort if it is busy
/*
    if ( amDongleBit_Wait0(waitShort, 1) < 0 ) {
      return -3;
    }
*/
//      __indword(*(_WORD *)&amLibContext[8] + 44);
//      __outbyte(0x80u, v4);
//      __outdword(*(_WORD *)&amLibContext[8] + 44, v4);
//      __outbyte(0x80u, v4);
    am44 &= ~8;
    printf("DWORD (am44) out: 0x%08X\n",am44);

    // Put data and pull the clock low

/*
    if ( amDongleBit_SetData_and_Strobe(((signed int)data >> bitOffset) & 3, &v8) < 0 ) {
      return -1;
    }
*/
    printf("Step %i, Setting A: %i, B: %i, strobe 0\n",step++,(data >> bitOffset) & 1,(a1 >> (bitOffset+1)) & 1);

    // Make sure that the dongle is not busy yet

/*
    if ( amDongleBit_Wait0(timeoutShort, 1) < 0 ) {
      return -4;
    }
*/

    // Pull clock high

/*
    if ( amDongleBit_ResetStrobe((unsigned __int32 *)&v8) < 0 ) { 
      return -1;
    }
*/
    printf("Resetting strobe to 1\n");

    // Now wait until
  
/*
    int timeout;
    if (crumbIndex == 3) {
      timeout = timeoutLong;
    } else {
      timeout = timeoutShort;
    }

    res = amDongleBit_Wait1(timeout, 0);
*/

    if ( res < 0 ) { return -2; }
//      v7 = __indword(*(_WORD *)&amLibContext[8] + 44);
//      __outbyte(0x80u, v7);
//      __outdword(*(_WORD *)&amLibContext[8] + 44, v7);
//      __outbyte(0x80u, v7);
    am44 |= 8;
    printf("DWORD (am44) out: 0x%08X\n",am44);
    crumbIndex++;
    bitOffset -= 2;
    if ( crumbIndex > 3 ) { return res; }
  }
}

signed int __cdecl amDongleSendEx(int a1, int a2, char *a3) {
  unsigned int v3; // esi@1
  int bytesLeft;// edi@1 
  char i; // zf@3 
  signed int err; // ebx@4 
  unsigned char v7; // edx@8 
  unsigned char v8; // eax@8 
  unsigned char v9; // edx@8 
  unsigned char v10; // eax@8 
  v3 = 0; 
  bytesLeft = a1; 
  if ( !unk_A7AF048 || unk_A7AF044 ) { a2 = a1; }
  for ( i = a1 == 0; ; i = bytesLeft == 0 ) {
    if ( i ) { return 0; }
    if ( !a2 ) { break; }
    --a2;
    err = amDongleSend(*a3);
    if ( err < 0 ) { goto LABEL_10; }
  LABEL_5:
    ++a3;
    --bytesLeft;
  }
  v7 = v3;
  v7 = (uint8_t)*a3 >> 4;
  v8 = v7;
  v8 = 2 * (((uint8_t)*a3 >> 4) & 1) | 4 * (((uint8_t)*a3 >> 4) & 2) | 8 * (v7 & 4) | 16 * (v7 & 8) | 0x55;
  v9 = v8;
  v9 = *a3 & 0xF;
  v10 = v9;
  v10 = (v9 >> 7) & 2 | (v9 >> 6) & 8 | (v9 >> 5) & 0x20 | (v9 >> 4) & 0x80 | 0x55;
  v3 = v10;
  err = amDongleSend(v9);
  if ( err >= 0 ) {
    err = amDongleSend(BYTE1(v3));
    if ( err >= 0 ) {
      goto LABEL_5;
    }
  }
LABEL_10:
  if ( amDongleDebugLevel > 0 ) {
    fprintf(stderr, "amDongleSendEx: send failed.(err=%d, %d byte(s) left, abort)\n", err, bytesLeft);
  }
  return err;
} 

int main(int argc, char* argv[]) {
  char* s = "\xF0\x0F\xA5\xC0\xFF"; //argv[1];
  int l = strlen(s);
  amDongleSendEx(l,l,s);
  return 0;  
}
