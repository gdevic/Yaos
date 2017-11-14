/******************************************************************************
*                                                                             *
*   Module:     LoadOS.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       7/30/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This module contains a 16-bit code that can be run at the DOS
        prompt to load and start Yaos.

-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 7/30/97    1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <stdio.h>                      // Include standard I/O header file
#include <stdlib.h>                     // Include standarl library header
#include <stddef.h>                     // Include standard compiler defines
#include <string.h>                     // Include string functions
#include <ctype.h>                      // Include character types
#include <malloc.h>                     // Include memory allocation code
#include <sys\stat.h>                   // Include file stat functions
#include <i86.h>                        // Include processor-specific defines

#pragma pack(1)                         // Watcom: pack structures

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef unsigned long DWORD;
typedef unsigned short WORD;

#define BUFFER_SIZE     16000           // Size of load buffers - this is
                                        // duplicated in Loader.asm

#define MAX_BUFFERS     20              // 640*1024 / BUFFER_SIZE

#define EXP_SIGNATURE   0x504D          // EXP file signature value

struct TEXPHeader
{
    WORD    Signature;                  // Signature - "MP"
    WORD    SizeRem;                    // Image size mod 512
    WORD    Size;                       // Image size in 512b pages
    WORD    Reloc;                      // Number of relocation entries
    WORD    HeaderSize;                 // Header size in pages (16b)
    WORD    MinPages;                   // Minimum extra pages (4K)
    WORD    MaxPages;                   // Maximum extra pages (4K)
    DWORD   ESP;                        // Initial ESP value
    WORD    Checksum;                   // Word checksum
    DWORD   EIP;                        // Initial EIP value
    WORD    RelocOff;                   // Offset of the first reloc item
    WORD    Overlay;                    // Overlay number
    WORD    Reserved;                   // Reserved word
    WORD    Reserved2;                  // Reserved word

} Head;                                 // Phar Lap Old EXP-file header


struct TStartupRecord
{
    DWORD  pKernel;                     // Linear address of a loaded kernel
    DWORD  EIP;                         // Initial EIP
    DWORD  dwFlags;                     // Flags (verbose:0)

} Startup;                              // Startup record structure


DWORD Buf[ MAX_BUFFERS ] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void cdecl ExecuteOS( struct TStartupRecord * pBootRecord );
extern WORD cdecl GetPageWord();

/******************************************************************************
*                                                                             *
*   void PrintUsage( char * sName )                                           *
*                                                                             *
*******************************************************************************
*
*   This function prints the syntax of the command line for the loader.
*
#   Where:
#       sName is the name of the loader (this program)
#
******************************************************************************/
void PrintUsage( char *sName )
{
    printf("Usage:\n");
    printf("\t%s <Kernel name> [-v][-p]\n\n", sName );
    printf("\t-v  Output debug info\n");
    printf("\t-p  Show page tables only\n");

    exit( 1 );
}


/******************************************************************************
*                                                                             *
*   void main( int argn, char *argv[] )                                       *
*                                                                             *
*******************************************************************************
*
*   This is the main function of a loader.
*
*   Where:
*       argn is the number of command-line arguments
#       argv is an array of pointers to command line arguments
*
*   Returns:
*       void
*
******************************************************************************/
void main( int argn, char *argv[] )
{
    FILE * fp;
    int nChunks;
    DWORD *pChunk;
    int nRet, i;
    char ch1;
    int fVerbose = 0;
    int fPages = 0;


    // Check for the extra arguments

    for( i=1; i<argn; i++ )
    {
        ch1 = *argv[i];
        if( ch1 == '/' || ch1 == '\\' || ch1 == '-' )
            ch1 = *(argv[i]+1);
        else
            continue;

        // Check for the additional options

        if( tolower(ch1) == 'v' )   fVerbose = 1;
        if( tolower(ch1) == 'p' )   fPages = 1;
        if( tolower(ch1) == '?' )   PrintUsage( argv[0] );
    }

    // The option -p will only show page tables with short descriptions

    if( fPages )
    {
        WORD wW0 = 0, wW1;

        for( i=0, wW0 = GetPageWord(); wW0 != -1; i++, wW0 = GetPageWord() )
        {
            wW1 = GetPageWord();

            printf("%04X\t%04X%04X\n", i, wW1, wW0 );
        }

        return;
    }


    // The first argument must be the file path/name of the kernel

    if( argn >= 2 )
    {
        // Open the kernel file

        fp = fopen( argv[1], "rb" );

        if( fp != NULL )
        {
            // Load an EXP-type header

            nRet = fread( &Head, sizeof(struct TEXPHeader) , 1, fp );

            // Check the header signature

            if( (Head.Signature == EXP_SIGNATURE) && (Head.HeaderSize == 2) )
            {
                if( fVerbose )
                {
                    printf("Header image size: %lu\n", (DWORD)(Head.Size-1) * 512 + (DWORD)Head.SizeRem );
                    printf("Minimum number of extra pages: %d (%u Kb)\n", Head.MinPages, Head.MinPages * 4096);
                    printf("Initial EIP: %lu\n", Head.EIP );
                }

                // Calculate necessary number of chunks

                nChunks = (((DWORD)(Head.Size-1) * 512 + (DWORD)Head.SizeRem) / BUFFER_SIZE) + 1;

                // Load a file into a set of buffers each BUFFER_SIZE is size

                for( i=0; i<nChunks; i++ )
                {
                    pChunk = malloc( BUFFER_SIZE );

                    // Check if we got a buffer

                    if( pChunk==NULL )
                    {
                        printf("Cannot allocate a load buffer\n");
                        return;
                    }

                    // Load a chunk of a kernel into a buffer

                    nRet = fread( pChunk, BUFFER_SIZE, 1, fp );

                    // Set up the address of that buffer into the array of addresses

                    Buf[i] = (DWORD) FP_SEG(pChunk) * 16 + FP_OFF(pChunk);
                }

                // Print the addresses of buffers

                if( fVerbose )
                {
                    for( i=0; i<=nChunks; i++ )
                    {
                        printf("\t%04X", Buf[i] >> 16 );
                        printf("%04X\n", Buf[i] );
                    }
                }

                // Set up the startup record to be sent to the assembly loader

                Startup.pKernel = (DWORD)FP_SEG(&Buf) * 16 + (DWORD)FP_OFF(&Buf);
                Startup.EIP     = Head.EIP;
                Startup.dwFlags = fVerbose;

                ExecuteOS( &Startup );

                // Check for return flags

                if( Startup.dwFlags & 0x8000 )
                {
                    printf("YAOS %d.%d is running\n", Startup.EIP>>8, Startup.EIP & 0xFF );
                }

                if( Startup.dwFlags & 0x4000 )
                {
                    printf("Your CPU is already in protected mode.  Please reboot your computer.\n");
                    printf("Do not load any device drivers or programs before running YAOS.\n");
                }
            }
            else
                printf("Invalid EXP file header\n");
        }
        else
            printf("Error: File `%s' cannot be opened\n", argv[1] );
    }
    else
        PrintUsage( argv[0] );
}

