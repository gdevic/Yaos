/*
 * This example clears the screen on DOS
 */
#include <i86.h>

void main()
  {
        int * ptr = 0xb8000;

        *ptr = 0x8721;
  }

