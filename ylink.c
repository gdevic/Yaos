#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef long int DWORD;
typedef short int WORD;
typedef unsigned char BYTE;

typedef struct
{
   BYTE type;
   WORD len;
   
} Trecord;

typedef struct
{
   BYTE p : 1;
   BYTE b : 1;
   BYTE c : 3;
   BYTE a : 3;
   
   WORD len;
   BYTE segnamex;
   BYTE classnamex;
   BYTE overlaynamex;
   
} Tsegdef16; 

typedef struct
{
   BYTE p : 1;
   BYTE b : 1;
   BYTE c : 3;
   BYTE a : 3;
   
   DWORD len;
   BYTE segnamex;
   BYTE classnamex;
   BYTE overlaynamex;
   
} Tsegdef32; 

typedef struct
{
   BYTE x : 1;
   BYTE   : 5;
   BYTE start : 1;
   BYTE main  : 1;
   
   BYTE enddata;
   BYTE startseg;
   WORD offset;
   
} Tmodend16;

typedef struct
{
   WORD offset : 10;
   WORD location : 4;
   WORD mode : 1;
   WORD type : 1;
   
   
} Tfixup16;  

typedef struct
{
   BYTE segx;
   WORD offset;
   
} Tledata16;  


FILE *fp;

int main( int argn, char *argp[] )
{
   Trecord record;
   Tsegdef16 *segdef16;
   Tsegdef32 *segdef32;
   Tledata16 *ledata16;
   Tmodend16 *modend16;
   char *ptr, *p;
   char tmps[255];
   int i;

   if( argn!=2 )
   {
        printf("Usage: ylink file.obj\n");
        exit(0);
   }
   
   fp = fopen( argp[1], "rb" );
   if( fp==NULL )
   {
        printf("Unable to open file %s\n", argp[1] );
        exit(1);
   }
   
   while( 1 )
   {
      fread( &record, sizeof(Trecord), 1, fp );
      
      if( feof(fp) )
        break;
      
      printf("Record type: %2xh, size: %d bytes\n", record.type, record.len );
      p = ptr = malloc( record.len );
      if( ptr==NULL )
      {
           printf("Cannot allocate for buffer\n");
           exit(1);
      }
      
      fread( ptr, record.len, 1, fp );
      
      switch( record.type )
      {
        case 0x80:   // THEADR - Translator Header Record
        
             ptr[ ptr[0]+1 ] = (char)0;
             printf(" Name: %s\n", &ptr[1] );
        
             break;
             
        case 0x88:   // COMMENT - Comment Record
        
             printf(" Comment\n");
        
             break;
             

        case 0x8A:   // MODEND16 - Module End Record 16 bit
             
             modend16 = (Tmodend16*)p;
        
             printf(" Module end record.\n");
             if( modend16->main )
                     printf(" The main program module.");
             if( modend16->start )
                     printf(" Start address at segment %d at offset %d\n",
                             modend16->startseg, modend16->offset );
        
             break;


        case 0x96:   // LNAMES - List of Names Record
        
             printf(" List of names:\n");
         
             i = 1;    
             while( record.len > 1 )
             {
                 memcpy( tmps, &p[1], (int)p[0] );
                 tmps[ p[0] ] = 0;
                 printf(" %d: `%s'\n", i, tmps );
                 
                 i++;
                 record.len -= p[0]+1;
                 p += p[0]+1;
             }
        
             break;

             
        case 0x98:   // SEGDEF16 - Segment Definition Record 16 bit
        
             segdef16 = (Tsegdef16*)p;
        
             printf(" Segment %d, class: %d, length: %d bytes, USE%d\n ", 
                     segdef16->segnamex, segdef16->classnamex, segdef16->len, segdef16->p? 32:16 );
                     
             switch( segdef16->a )
             {
                case 0:  printf("Absolute segment NOT SUPPORTED!!!\n"); 
                     break;
                     
                case 1:  printf("Relocatable, byte aligned\n"); 
                     break;

                case 2:  printf("Relocatable, word aligned\n"); 
                     break;

                case 3:  printf("Relocatable, paragraph aligned\n"); 
                     break;

                case 4:  printf("Relocatable, page aligned\n"); 
                     break;

                case 5:  printf("Relocatable, dword aligned\n"); 
                     break;
                     
                default: printf("Not supported!!!\n");
             }

             switch( segdef16->c )
             {
                case 0:  printf(" Private - do not combine\n");
                     break;
                     
                case 2:
                case 4:
                case 7:  printf(" Public - combine by appending\n");
                     break;
                     
                case 6:  printf(" Common - combine by overlay NOT SUPPORTED!!!\n");
                     break;
                     
                default: printf("Not supported!!!\n");
             }

             if( segdef16->b )
                     printf(" Big segment exactly 64K long\n");
             
             break;

             
        case 0x99:   // SEGDEF32 - Segment Definition Record 32 bit
        
             segdef32 = (Tsegdef32*)p;
        
             printf(" Segment %d, class: %d, length: %d bytes, USE%d\n ", 
                     segdef32->segnamex, segdef32->classnamex, segdef32->len, segdef32->p? 32:16 );
                     
             switch( segdef32->a )
             {
                case 0:  printf("Absolute segment NOT SUPPORTED!!!\n"); 
                     break;
                     
                case 1:  printf("Relocatable, byte aligned\n"); 
                     break;

                case 2:  printf("Relocatable, word aligned\n"); 
                     break;

                case 3:  printf("Relocatable, paragraph aligned\n"); 
                     break;

                case 4:  printf("Relocatable, page aligned\n"); 
                     break;

                case 5:  printf("Relocatable, dword aligned\n"); 
                     break;
                     
                default: printf("Not supported!!!\n");
             }

             switch( segdef32->c )
             {
                case 0:  printf(" Private - do not combine\n");
                     break;
                     
                case 2:
                case 4:
                case 7:  printf(" Public - combine by appending\n");
                     break;
                     
                case 6:  printf(" Common - combine by overlay NOT SUPPORTED!!!\n");
                     break;
                     
                default: printf("Not supported!!!\n");
             }

             if( segdef32->b )
                     printf(" Big segment exactly 4GB long\n");
             
             break;


        case 0x9C:   // FIXUP16 - Fixup Record 16 bit
        
        
             break;
             
             
        case 0xA0:   // LEDATA16 - Logical Enumerated Data Record 16 bit

             ledata16 = (Tledata16*)p;
             
             printf(" Logical Enumerated Data of a Segment %d, starting at offset %d\n",
                     ledata16->segx, ledata16->offset );
        
             while( record.len > 4 )
             {
                     printf("%02X  ", *(p+3) );
                     p++;
                     record.len--;
             }
             
             printf("\n");
        
             break;
      }
      
      free( ptr );

   };
   
   
   fclose(fp);
   
   return( 0 );
}