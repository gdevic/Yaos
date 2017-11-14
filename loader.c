#include <stdio.h>
#include <stdlib.h>

typedef unsigned short int USHORT;
typedef unsigned long int ULONG;


typedef struct
{
	char exe_sign1,exe_sign2; /* [00] Signature word (MP) */
	USHORT exe_szrem;		/* [02] Remainder of the image size
					        when divided by the page
					        size (512 bytes) */
	USHORT exe_size;		/* [04] Size of image in pages */
	USHORT exe_relcnt;		/* [06] Number of relocation entries*/
	USHORT exe_hsize;		/* [08] Header size in paragraphs */
	USHORT exe_minpg;		/* [0A] Minimum number of extra 
					        4K pages to be allocated
					        at the end of a program when
					        it is loaded */
	USHORT exe_maxpg;		/* [0C] Maximum number of extra
					        4K pages to be allocated
					        at the end of a program when
					        it is loaded */
	ULONG	exe_esp;		/* [0E] Initial ESP */
	USHORT exe_chksum;		/* [12] Word checksum of file */
	ULONG	exe_eip;		/* [14] Initial EIP */
	USHORT exe_reloff;		/* [18] Offset of first relocation
					        item */
	USHORT exe_ovlno;		/* [1A] Overlay number */
	USHORT exe_unkw;		/* [1C] Unknown word, wants to be 1 */
} Texp;


main( int argn, char *argp[] )
{
   FILE *fp;
   Texp *cptr;

   if( argn != 2 )
   {
        printf("Usage: loader <file.obj>\n");
        exit(0);
   }
   
   fp = fopen( argp[1], "rb" );
   if( fp==NULL )
   {
        printf("Cant open %s\n", argp[1] );
        exit(1);
   }

   cptr = malloc( sizeof( Texp ) );
   if( cptr==NULL )
   {
        printf("Cant allocate(1)\n");
        exit(1);
   }
   
   fread( cptr, sizeof( Texp ), 1, fp );
   
   printf("Signature: %c%c\n", cptr->exe_sign1, cptr->exe_sign2 );
   printf("Size of the image in pages: %d\n", cptr->exe_size );
   printf("Remainder of the size: %d bytes\n", cptr->exe_szrem );
   printf("Number of reloc entries: %d\n", cptr->exe_relcnt );
   printf("Header size in paragraphs: %d\n", cptr->exe_hsize );
   printf("Min pages to alloc from the end: %d\n", cptr->exe_minpg );
   printf("Max pages to alloc from the end: %d\n", cptr->exe_maxpg );
   printf("Initial ESP: %08X\n", cptr->exe_esp );
   printf("Initial EIP: %08X\n", cptr->exe_eip );
   printf("Offset of first reloc item: %d\n", cptr->exe_reloff );
   printf("Overlay num: %d\n", cptr->exe_ovlno );
   printf("Unknown word: %d\n", cptr->exe_unkw );

   
   free( cptr );
   fclose( fp );
}
