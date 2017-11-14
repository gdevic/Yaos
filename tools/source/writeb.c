/****************************************************************************
*                                                                           *
*   Module:     writeb                                                      *
*                                                                           *
*   Revision:   1.00                                                        *
*                                                                           *
*   Date:       01/23/96                                                    *
*                                                                           *
*   Author:     Goran Devic                                                 *
*                                                                           *
*****************************************************************************
*                                                                           *
*   Module Description:                                                     *

        This program will write row data to the floppy disk
        It is hardcoded to write 512 bytes to the sector 1 of the disk 0

        The source must be the valid .EXE file.

        Compile with:  wcl writeb.c

*****************************************************************************
*                                                                           *
*   Changes:                                                                *
*                                                                           *
*   DATE     REV   DESCRIPTION OF CHANGES                        AUTHOR     *
* --------   ----  -----------------------------------------   -----------
  01/23/96   1.00  Original                                    Goran Devic
  02/11/96   1.01  Writes boot sector only                     Goran Devic
  02/18/96   1.02  Cleanup                                     Goran Devic

* --------   ----  -----------------------------------------   -----------  *
*****************************************************************************
*   Important defines/undefines                                             *
*****************************************************************************/

/****************************************************************************
*   Include Files                                                           *
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <i86.h>

/****************************************************************************
*   Global Functions                                                        *
*****************************************************************************/

/****************************************************************************
*   External Functions                                                      *
*****************************************************************************/

/****************************************************************************
*   Local Variables and defines                                             *
*****************************************************************************/
#define RETRY     4     // Number of retries to write sector

typedef short int WORD;

typedef struct TEXEHeader
{
    char    M, Z;       // "MZ" signature
    WORD    bytes;      // number of bytes in last 512-byte page
    WORD    pages;      // number of 512-byte pages
    WORD    reloc;      // number of relocation entries
    WORD    header;     // header size in paragraphs
    WORD    minmem;
    WORD    maxmem;
    WORD    init_ss;
    WORD    init_sp;
    WORD    checksum;
    WORD    init_ip;
    WORD    init_cs;
    WORD    offset;
    WORD    overlay;

} EXEHeader;

/****************************************************************************
*   Local Functions                                                         *
*****************************************************************************/
int main( int argn, char *argp[] )
{
    EXEHeader *pHeader;

    union   REGS r;
    struct  SREGS s;
    FILE    *fp;
    long    dSize;
    int     dKey, i;
    char    *cpBuf;
    int     dRetry;
    int     dStart, dLen = 512; // Hard coded for 1 sector write
    int     dStartSec = 1;      // Hard coded for sector 1

    if( argn != 2 )
        usage();

    // Read the .exe file
    fp = fopen( argp[1], "rb" );
    if( fp==NULL )
    {
        printf("Cannot open input file: %s\n", argp[1] );
        exit(1);
    }

    // Get the size of the EXE file
    fseek( fp, 0L, SEEK_END );
    dSize = ftell( fp );
    fseek( fp, 0L, SEEK_SET );

    printf("File size: %d bytes\n", dSize );

    // Allocate the buffer to hold data
    cpBuf = malloc( dSize );
    if( cpBuf == NULL )
    {
        printf("Unable to allocate %d bytes\n", dSize );
        exit(1);
    }

    // Read .EXE file
    fread( cpBuf, dSize, 1, fp );
    fclose(fp);

    // Extract the information from the .EXE header
    pHeader = (EXEHeader *)cpBuf;

    if( pHeader->M != 'M' || pHeader->Z != 'Z' )
    {
        printf("File is not in the proper EXE format!\n");
        exit(1);
    }

    printf("%4X - Total size mod 512\n", pHeader->bytes );
    printf("%4X - Total size div 512\n", pHeader->pages );
    printf("%4X - Number of relocation entries\n", pHeader->reloc );
    printf("%4X - Header size in paragraphs\n", pHeader->header );
    printf("%4X - Minimum paragraphs to allocate\n", pHeader->minmem );
    printf("%4X - Maximum paragraphs to allocate\n", pHeader->maxmem );
    printf("%4X - Initial SS relative to start of executable\n", pHeader->init_ss );
    printf("%4X - Initial SP\n", pHeader->init_sp );
    printf("%4X - Checksum\n", pHeader->checksum );
    printf("%4X - Initial IP\n", pHeader->init_ip );
    printf("%4X - Initial CS\n", pHeader->init_cs );
    printf("%4X - Offset within header of relocation table\n", pHeader->offset );
    printf("%4X - Overlay number or 0 for main program\n", pHeader->overlay );

    dStart = pHeader->header*16 + pHeader->reloc*4;

    printf("Writing %d bytes:\n", dLen );

    for( i=0; i<dLen; i++ )
        printf("%02X  ", *(cpBuf+dStart+i) );

    printf("\n\nAbout to write %d sector(s) starting at sector %d\n\n",
            dLen/512, dStartSec );
    printf("Insert a floppy disc and press Enter...\n");
    flushall();

    // Get a key and check for Enter - any other key will escape
    dKey = getch();
    if( dKey!=13 )
    {
        printf("Aborting.\n");
        free(cpBuf);
        exit(1);
    }

    // Retry several times for disk sector write
    dRetry = RETRY;

    do
    {
        r.h.ah = 0x00;          // Reset
        r.h.dl = 0x00;          // Floppy
        int86x( 0x13, &r, &r, &s );

        delay(10);

        // Write sector
        r.h.ah = 0x03;          // Write Disk Sector
        r.h.al = dLen/512;      // Number of sectors to write
        r.h.ch = 0x00;          //  cyl 0
        r.h.cl = dStartSec;     //  starting sector
        r.h.dh = 0x00;          //  head 0
        r.h.dl = 0x00;          //  drive 0
        r.w.bx = FP_OFF( cpBuf+dStart );
        s.es = FP_SEG( cpBuf+dStart );
        int86x( 0x13, &r, &r, &s );

        delay(10);

        } while( (dRetry--) && (r.h.ah!=0x00) );

    if( dRetry==0 )
        printf("Writing failed after %d retries!\n", RETRY );
    else
        printf("Done.\n");

    free(cpBuf);

    return( 0 );
}


void usage()
{
    printf("\nWrites boot sector on the floppy disk from an EXE file\n");
    printf("Usage: write <Filename.exe>\n\n");

    exit(0);
}
