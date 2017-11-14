/*********************************************************************
*                                                                    *
*   Module:     Checksum.c                                           *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       03/14/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

        This module calculates the kernel checksum and stores
        it in the dword at offset 2 from the start of the kernel.

        The checksum implemented is to add a dword every 16 bytes
        starting at the byte 32 from the start of the file to the
        end (skipping the file header).
        
        The code that checks the checksum is in bootsect.asm.
       
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  03/14/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Include Files
**********************************************************************/
#include <stdlib.h>
#include <stdio.h>

/*********************************************************************
*   Main function
**********************************************************************/

int main( int argn, char *argp[] )
{
    FILE *fin, *fout;
    unsigned long checksum = 0;
    long length;
    char *ptr;
    int i;

    if( argn != 3 )
    {
        printf("Checksum writes the checksum dword in the kernel at");
        printf(" the offset 2.\n");
        printf("Usage:  checksum infile outfile\n");

        return 1;
    }

    // Open the input file and the output file
    //
    if( (fin = fopen( argp[1], "rb" ))==NULL )
    {
        printf("Unable to open input file %s\n", argp[1] );

        return 1;
    }

    if( (fout = fopen( argp[2], "wb" ))==NULL )
    {
        printf("Unable to open output file %s\n", argp[2] );

        return 1;
    }


    // Get the length of the kernel file
    //
    fseek( fin, 0L, SEEK_END );
    length = ftell( fin );
    fseek( fin, 0L, SEEK_SET );

    printf("Kernel length: %d bytes\n", length );


    // Allocate enough space to load whole kernel
    //
    if( (ptr = malloc(length) )==NULL )
    {
        printf("Unable to allocate memory!\n");

        return 1;
    }


    // Load the whole kernel file into that space
    //
    if( fread( ptr, length, 1, fin ) != 1 )
    {
        printf("Sorry, fread failed!\n");

        return 1;
    }


    // Calculate the checksum
    //
    for( i=32; i<=length-4; i+=16 )
        checksum += *(unsigned long*)( ptr+i );

    printf("Checksum = %08Xh\n", checksum );


    // Write the checksum dword
    //
    *(unsigned long*)(ptr+2) = checksum;


    // Write the whole kernel into the output file
    //
    if( fwrite( ptr, length, 1, fout ) != 1 )
    {
        printf("Sorry, fwrite failed!\n");

        return 1;
    }


    // Done!
    //
    printf("Done.\n");
    

    return 0;
}
