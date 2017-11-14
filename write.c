/****************************************************************************
*                                                                           *
*   Module:     write                                                       *
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
       
*****************************************************************************
*                                                                           *
*   Changes:                                                                *
*                                                                           *
*   DATE     REV   DESCRIPTION OF CHANGES                        AUTHOR     *
* --------   ----  -----------------------------------------   -----------
  01/23/96   1.00  Original                                    Goran Devic
  02/11/96   1.01  Writes boot sector only                     Goran Devic

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
    long    dPos, dSize;
    int     dKey;
    char    *cpBuf;
    int     dRetry;
    int     dStart, dLen, i;
    int     dStartSec;
    char    fname[13];

    if( argn < 2 )
        usage();

    if( argn==2 )
    {
        strncpy( fname, argp[1], strcspn(argp[1],".") );
        strcat( fname, ".bin" );
        printf("Creating %s\n", fname );
    }
    else
    {
        sscanf(argp[2], "%d", &dStartSec );
        printf("File name: %s\nStarting sector: %d\n", argp[1], dStartSec );
    }
    
    fp = fopen( argp[1], "rb" );
    if( fp==NULL )
    {
        printf("Cannot open input file!\n" );
        exit(1);
    }

    dPos = ftell( fp );
    fseek( fp, 0L, SEEK_END );
    dSize = ftell( fp );
    fseek( fp, dPos, SEEK_SET );

    printf("File size: %d bytes\n", dSize );
    
    cpBuf = malloc( dSize );
    if( cpBuf == NULL )
    {
        printf("Unable to allocate %d bytes\n", dSize );
        fclose( fp );
        exit(1);
    }

    fread( cpBuf, dSize, 1, fp );
    fclose(fp);

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
    dLen = (pHeader->pages-1)*512 + pHeader->bytes - dStart;

    printf("Code size: %d bytes\n", dLen );

    for( i=0; i<dLen; i++ )
        printf("%02X  ", *(cpBuf+dStart+i) );

    if( argn==2 )
    {
        FILE * fpout;
        
        printf("Writing the output file...\n");
        fpout = fopen( fname, "wb" );
        
        for( i=0; i<dLen; i++ )
        {
            fputc( *(cpBuf+dStart+i), fpout );
        }
        
        fclose( fpout );
        
        printf("Updating boot sector...\n");
        
    }
    else
    {

        printf("\n\nAbout to write %d sector(s) starting at %d\n", (dLen/512)+1, dStartSec );
        printf("Insert a floppy disc and press Enter...");
        flushall();
        dKey = getch();
        if( dKey!=13 )
        {
            printf("Aborting!\n");
            free(cpBuf);
            exit(1);
        }

        dRetry = 4;

        do
        {
            // Reset floppy
            r.h.ah = 0x00;          // Reset
            r.h.dl = 0x00;          // Floppy
            int86x( 0x13, &r, &r, &s );

            delay(10);

            // Write sector
            r.h.ah = 0x03;          // Write Disk Sector
            r.h.al = (dLen/512)+1;  // Number of sectors to write
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
            printf("Writing failed - retried 4 times!\n");
    }
    
    free(cpBuf);

    return( 0 );
}


void usage()
{
    printf("Usage: write <Filename.exe> <Sector number>\n");
    printf("     | write <Filename.exe to *.bin+set boot sector>\n");
    exit(0);
}
