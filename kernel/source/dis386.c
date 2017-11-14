/*********************************************************************
*                                                                    *
*   Module:     dis386.c                                             *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       03/02/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

        This module contains the code for the Intel 386 disasembler.

**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  03/02/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************/

#define _TEST_       0

/*********************************************************************
*   Include Files
**********************************************************************/
#if _TEST_

typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;
#include <stdlib.h>
#include <stdio.h>
#include "..\include\dis386.h"

#else

#include "types.h"

#include "dis386.h"
#include "dis386p.h"

#include "debugger.h"

#endif

/*********************************************************************
*   Global Functions
**********************************************************************/
BYTE Dis386( TDis386 *pDis );


/*********************************************************************
*   Local Variables and defines
**********************************************************************/
#define NEXTBYTE  *bpTarget++; bInstrLen++
#define NEXTWORD  *( WORD*)bpTarget; bpTarget+=2; bInstrLen+=2
#define NEXTDWORD *(DWORD*)bpTarget; bpTarget+=4; bInstrLen+=4


/*********************************************************************
*   Local Functions
**********************************************************************/

/*********************************************************************
*
*   BYTE Dis386( TDis386 *pDis )
*
*   Where:
*
*       pDis is the pointer to the structure defined in DIS386.H that
*       contains all the fields relevant to the communication to this
*       function.  They are:
*
*       (BYTE*) bpTarget   Pointer to the address to disassemble
*       (BYTE)  bDataSize  Size of the data segment:
*                                0 for 16 bit data
*                                1 for 32 bit data
*       (BYTE)  bAddressSize Size of the address segment:
*                                0 for 16 bit address
*                                1 for 32 bit address
*       (BYTE*) szDisasm   Pointer to the space in the memory where the
*                          disassembled instruction will be stored
*                          as an ASCIIZ string. 80 characters in lenght
*                          will be enough.
*
*   Returns:
*
*       Disassembled instruction is stored as an ASCIIZ string pointed
*       by szDisasm pointer in pDis structure.
*
*       Also, the following fields in that structure are set:
*
*       (BYTE) bAsciiLen   Lenght of the resulting string in bytes
*       (BYTE) bInstrLen   Total lenght of the instruction in bytes
*       (BYTE) fIllegalOp  Set to 1 if the instruction being disassembled
*                          was either illegal or not supported by the
*                          disassembler.
*
*       The function itself returns the bInstrLen value.
*
*
**********************************************************************/
BYTE Dis386( TDis386 *pDis )
{
    BYTE     bPos;               // Printing position in the output string
    BYTE     bOpcode;            // Current opcode that is being disassembled
    DWORD    dwCode;             // Current opcode' code from the table
    BYTE     bInstrLen;          // Current instruction lenght in bytes
    BYTE     bTable;             // Table number that should be look into
    BYTE     bSegOverride;       // Segment override status, zero for default segment
    BYTE     bDataSize;          // Default data size, 16/32 bit (0/1)
    BYTE     bAdSize;            // Default address size, 16/32 bit (0/1)
    BYTE     bByte;              // Temporary byte storage
    WORD     wWord;              // Temporary word storage
    DWORD    dwDword;            // Temporary dword storage
    BYTE     bW;                 // Width bit for the register selection
    BYTE     fPrintPtr;          // Prints pointer message just once flag
    char     *sPtr;              // Message selection pointer

    BYTE     *bpTarget;          // Pointer to the target code to be disassembled
    BYTE     bOp;                // Operation field of the code
    BYTE     bDest;              // Destination selector of the code
    BYTE     bSrc;               // Source selector of the code
    BYTE     bThird;             // Third selector for 3-argument codes

    BYTE     bMod;               // Mod field of the instruction
    BYTE     bReg;               // Register field of the instruction
    BYTE     bRm;                // R/M field of the instruction

    BYTE     bSib;               // S-I-B byte for the instruction
    BYTE     bSs;                // SS field of the s-i-b byte
    BYTE     bIndex;             // Index field of the s-i-b byte
    BYTE     bBase;              // Base field of the s-i-b byte


    //
    // Initialization of the local variables
    //
    bPos = 0;          // Printing position in the output string
    fPrintPtr = 1;     // Print pointer message just once
    bInstrLen = 0;     // Set the instruction len
    bTable = 0;        // Set the first lookup table (1-byte opcodes)
    bSegOverride = 0;  // Set the segment override to 'default'
    bDataSize = pDis->bDataSize;    // Set the default data and
    bAdSize = pDis->bAddressSize;   // address sizes
    bpTarget = pDis->bpTarget;      // Set the starting address
    pDis->fIllegalOp = 0;           // Assume legal opcode
    bW = 1;            // Default long encoding

    
    while( bInstrLen < 16 )
    {
        //
        // Fetch the opcode from the target address
        //
        bOpcode = NEXTBYTE;


        //
        // Get the code of the current instruction from the table and
        // separate the fields of name (code), destination and source
        // modes
        //
        if( bTable )
             dwCode = Op2[ bOpcode >> 4 ][ bOpcode & 0xF ];
        else
             dwCode = Op1[ bOpcode >> 4 ][ bOpcode & 0xF ];

        bOp    = dwCode & 0xFF;
        bDest  = (dwCode >> 16) & 0xFF;
        bSrc   = (dwCode >> 8) & 0xFF;
        bThird = dwCode >> 24;

        //
        // Fetch the modR/M byte only if needed (argument-wise)
        //
        if( ((bDest>0) && (bDest<_Ob)) || ((bSrc>0) && (bSrc<_Ob)) ||
            ( bOp >= _GRP1 && bOp <= _GRP9 ) )
        {
             // Get the next byte (modR/M bit field)
             bOpcode = NEXTBYTE;

             // Parse that byte and get mod, reg and rm fields
             bMod = bOpcode >> 6;
             bReg = (bOpcode >> 3) & 7;
             bRm  = bOpcode & 7;
        }

        //
        // Look the special case opcodes
        //
        switch( bOp )
        {
             case _2BESC: // 2 byte escape code
                 bTable = 2;
             continue;

             case _ESC:   // ESCape to coprocessor instruction
                          // ** not yet supported **
                 bPos += sprintf( pDis->szDisasm+bPos, "ESC ???");
                 pDis->fIllegalOp = 1;
                 goto DisEnd;

             case _S_ES:  // Segments override
             case _S_CS:  //
             case _S_SS:  // Set the new segment code that will be 
             case _S_DS:  // greather than zero, since zero means
             case _S_FS:  // no segment override
             case _S_GS:
                 bSegOverride = bOp - _S_ES + 1;
             continue;

             case _OPSIZ: // Operand size override - toggle
                 bDataSize ^= 1;
             continue;

             case _ADSIZ: // Address size override - toggle
                 bAdSize ^= 1;
             continue;

             case _GRP1:  // Additional groups of instructions that 
             case _GRP2:  // need another byte to fetch
             case _GRP3:
             case _GRP4:
             case _GRP5:
             case _GRP6:
             case _GRP7:
             case _GRP8:
             case _GRP9:

                 // Get the code from the group table
                 dwCode = Groups[ bOp - _GRP1 ][ bReg ];
                 bOp    = dwCode & 0xFF;

                 // Merge the arguments
                 bDest  |= (dwCode >> 16) & 0xFF;
                 bSrc   |= (dwCode >> 8) & 0xFF;
                 bThird |= dwCode >> 24;

                 // Everything is ok if the opcode is valid
                 if( bOp != _NDEF )
             break;

             case _NDEF : // Not defined or illegal opcode

                 bPos += sprintf( pDis->szDisasm+bPos, "???");
                 pDis->fIllegalOp = 1;
                 goto DisEnd;

        }

        //
        // Print the mnemonic only, if the third argument is _NM, use the
        // next opcode and nullify the third argument (for instructions 
        // that change name depending on the data width)
        //
        if( bDataSize && (bThird==_NM) ) 
            bOp++;
        
        // Clean the third argument only if it is a name changing flag
        if( bThird==_NM )
            bThird = 0;
        
        bPos += sprintf( pDis->szDisasm+bPos, "%-6s ", sNames[ bOp - 1 ] );

        //
        // Do the argument processing, up to three times
        //
        while( bDest )
        {
            //
            // Evaluate the argument in bDest
            //
            switch( bDest )
            {
                 case _Eb : // modR/M used - bW = 0
                     bW = 0;
                     goto _E;
                     
                 case _Ev : // modR/M used - bW = 1
                     bW = 1;
                     goto _E;
                 
                 case _Ew : // always word size
                     bDataSize = 0;
                     bW = 1;
                     goto _E;
                 
                 case _Ms :
                 case _Mp :
                 case _Ep : // Always pointer
                     if( bDataSize==0 )
                         sPtr = sDwordPtr;
                     else
                         sPtr = sFwordPtr;
                     goto _E1;  // skip the intro

                 case _M  :

                 _E:
                     // Do registers first so that the rest may be done together
                     if( bMod == 3 )
                     {
                          // Registers depending on the w field and data size
                          bPos+=sprintf(pDis->szDisasm+bPos, "%s", sRegs1[bDataSize][bW][bRm] );

                          break;
                     }

                     //
                     // Print the pointer message
                     //
                     if( bW==0 )
                         sPtr = sBytePtr;
                     else
                         if( bDataSize==0 )
                             sPtr = sWordPtr;
                         else
                             sPtr = sDwordPtr;

                 _E1:

                     if( fPrintPtr )
                     {
                         bPos += sprintf( pDis->szDisasm+bPos, "%s", sPtr );
                         fPrintPtr = 0;
                     }

                 case _Ma : // Used by bound instruction, skip the pointer info
                 

                     //
                     // Print the segment if it is overriden
                     //
                     if( bSegOverride )
                         bPos += sprintf( pDis->szDisasm+bPos,"%s:", sSeg[ bSegOverride-1 ] );


                     //
                     // Special case when sib byte is present in 32 address encoding
                     //
                     if( (bRm==4) && (bAdSize) )
                     {
                          //
                          // Get the s-i-b byte and parse it
                          //
                          bSib = NEXTBYTE;

                          bSs = bSib >> 6;
                          bIndex = (bSib >> 3) & 7;
                          bBase = bSib & 7;

                          // Special case for base=5, 32 bit offset instead
                          if( bBase==5 )
                          {
                              dwDword = NEXTDWORD;
                              bPos += sprintf( pDis->szDisasm+bPos,"[%x", dwDword );
                          }
                          else
                              bPos += sprintf( pDis->szDisasm+bPos,"[%s", sGenReg16_32[ 1 ][ bBase ] );

                          // Scaled index, no index if bIndex is 4
                          if( bIndex != 4 )
                              bPos += sprintf( pDis->szDisasm+bPos,"+%s%s", sScale[ bSs ], sGenReg16_32[ 1 ][ bIndex ] );

                          // Offset 8 bit or 32 bit
                          if( bMod == 1 )
                          {
                              bByte = NEXTBYTE;
                              if( (signed char)bByte < 0 )
                                     bPos += sprintf( pDis->szDisasm+bPos,"-%xH", 0-(signed char)bByte );
                              else
                                     bPos += sprintf( pDis->szDisasm+bPos,"+%xH", bByte );
                          }

                          if( bMod == 2 )
                          {
                              dwDword = NEXTDWORD;
                              bPos += sprintf( pDis->szDisasm+bPos,"+0%xH", dwDword );
                          }

                          // Wrap up the instruction
                          bPos += sprintf( pDis->szDisasm+bPos,"]" );


                          break;
                     }

                     //
                     // 16 or 32 address bit cases with mod zero, one or two
                     //
                     // Special cases when r/m is 5 and mod is 0, immediate d16 or d32
                     if( bMod==0 && ((bRm==6 && !bAdSize) || (bRm==5 && bAdSize)) )
                     {
                          if( bAdSize==0 )
                          {
                              wWord = NEXTWORD;
                              bPos += sprintf( pDis->szDisasm+bPos,"0%xH", wWord );
                          }
                          else
                          {
                              dwDword = NEXTDWORD;
                              bPos += sprintf( pDis->szDisasm+bPos,"0%xH", dwDword );
                          }

                          break;
                     }

                     // Print the start of the line
                     bPos += sprintf( pDis->szDisasm+bPos,"[%s", sAdr1[ bAdSize ][ bRm ] );

                     // Offset (8 or 16) or (8 or 32) bit - 16, 32 bits are unsigned
                     if( bMod==1 )
                     {
                          bByte = NEXTBYTE;
                          if( (signed char)bByte < 0 )
                                 bPos += sprintf( pDis->szDisasm+bPos,"-%xH", 0-(signed char)bByte );
                          else
                                 bPos += sprintf( pDis->szDisasm+bPos,"+%xH", bByte );
                     }

                     if( bMod==2 )
                     {
                          if( bAdSize==0 )
                          {
                              wWord = NEXTWORD;
                              bPos += sprintf( pDis->szDisasm+bPos,"+%xH", wWord );
                          }
                          else
                          {
                              dwDword = NEXTDWORD;
                              bPos += sprintf( pDis->szDisasm+bPos,"+%xH", dwDword );
                          }
                     }

                     // Wrap up the instruction
                     bPos += sprintf( pDis->szDisasm+bPos,"]" );

                 break;

                 case _Gb : // general, byte register
                     bPos += sprintf( pDis->szDisasm+bPos, "%s", sRegs1[0][0][ bReg ] );
                 break;

                 case _Gv : // general, (d)word register
                     bPos += sprintf( pDis->szDisasm+bPos, "%s", sGenReg16_32[ bDataSize ][ bReg ] );
                 break;

                 case _Yb : // ES:(E)DI pointer
                 case _Yv :
                     bPos += sprintf( pDis->szDisasm+bPos, "%s:%s",
                        bSegOverride? sSeg[ bSegOverride-1 ] : "es", sYptr[ bAdSize ] );
                 break;

                 case _Xb : // DS:(E)SI pointer
                 case _Xv :
                     bPos += sprintf( pDis->szDisasm+bPos, "%s:%s",
                        bSegOverride? sSeg[ bSegOverride-1 ] : "ds", sXptr[ bAdSize ] );
                 break;

                 case _Rd : // register double word
                     bPos += sprintf( pDis->szDisasm+bPos, "%s", sGenReg16_32[ 1 ][ bMod ] );
                 break;

                 case _Rw : // register word
                     bPos += sprintf( pDis->szDisasm+bPos, "%s", sGenReg16_32[ 0 ][ bMod ] );
                 break;

                 case _Sw : // segment register
                     bPos += sprintf( pDis->szDisasm+bPos, "%s", sSeg[ bReg ] );
                 break;

                 case _Cd : // control register
                     bPos += sprintf( pDis->szDisasm+bPos, "%s", sControl[ bReg ] );
                 break;

                 case _Dd : // debug register
                     bPos += sprintf( pDis->szDisasm+bPos, "%s", sDebug[ bReg ] );
                 break;

                 case _Td : // test register
                     bPos += sprintf( pDis->szDisasm+bPos, "%s", sTest[ bReg ] );
                 break;


                 case _Jb : // immediate byte, relative offset
                     bByte = NEXTBYTE;
                     bPos += sprintf( pDis->szDisasm+bPos,"short %+d  (%04X)",
                        (int)bByte, bpTarget + (int)bByte );
                 break;

                 case _Jv : // immediate word or dword, relative offset
                     if( bDataSize==0 )
                     {
                          wWord = NEXTWORD;
                          bPos += sprintf( pDis->szDisasm+bPos, "%+d  (%04X)",
                             (short)wWord, bpTarget + (short)wWord );
                     }
                     else
                     {
                          dwDword = NEXTDWORD;
                          bPos += sprintf( pDis->szDisasm+bPos, "%+d  (%04X)",
                             (long)dwDword, bpTarget + (long)dwDword );
                     }
                 break;

                 case _Ob : // immediate byte
                 case _Ib :
                 case _imm8:
                     bByte = NEXTBYTE;
                     bPos += sprintf( pDis->szDisasm+bPos,"0%xH", bByte );
                 break;

                 case _Ov : // immediate word or dword
                 case _Iv :
                 case _immw:
                     if( bDataSize==0 )
                     {
                          wWord = NEXTWORD;
                          bPos += sprintf( pDis->szDisasm+bPos, "0%04xH", wWord );
                     }
                     else
                     {
                          dwDword = NEXTDWORD;
                          bPos += sprintf( pDis->szDisasm+bPos, "0%08xH", dwDword );
                     }
                 break;

                 case _Iw : // Immediate word
                     wWord = NEXTWORD;
                     bPos += sprintf( pDis->szDisasm+bPos, "0%04xH", wWord );
                 break;

                 case _Ap : // 32 bit or 48 bit pointer, possible segment override
                     if( bDataSize==0 )
                     {
                          dwDword = NEXTDWORD;
                          bPos += sprintf( pDis->szDisasm+bPos, "%s%08x",
                              bSegOverride? sSeg[ bSegOverride-1 ] : "", dwDword );
                     }
                     else
                     {
                          dwDword = NEXTDWORD;
                          wWord = NEXTWORD;
                          bPos += sprintf( pDis->szDisasm+bPos, "%s%04x:08%x",
                              bSegOverride? sSeg[ bSegOverride-1 ] : "", wWord, dwDword );
                     }
                 break;

                 case _1 : // numerical 1
                     bPos += sprintf( pDis->szDisasm+bPos,"1" );
                 break;

                 case _3 : // numerical 3
                     bPos += sprintf( pDis->szDisasm+bPos,"3" );
                 break;

                 // Hard coded registers
                 case _DX: case _AL: case _AH: case _BL: case _BH: case _CL: case _CH:
                 case _DL: case _DH: case _CS: case _DS: case _ES: case _SS: case _FS:
                 case _GS:
                     bPos += sprintf( pDis->szDisasm+bPos,"%s", sRegs2[ bDest - _DX ] );
                 break;

                 case _eAX: case _eBX: case _eCX: case _eDX:
                 case _eSP: case _eBP: case _eSI: case _eDI:
                     bPos += sprintf( pDis->szDisasm+bPos, "%s", sGenReg16_32[ bDataSize ][ bDest - _eAX ]);
                 break;
            };


            // Loop over the next argument if source is specified.  We can loop up
            // to three times
            if( bSrc )
                bPos += sprintf( pDis->szDisasm+bPos, ", " );

            // Shift down the argument list: 0 -> Third -> bSrc -> bDest
            bDest  = bSrc;
            bSrc   = bThird;
            bThird = 0;
        }


        // We are done with the instruction
        goto DisEnd;
    }
    //
    // This code can be executed only if instruction lenght was more than
    // 16 bytes - therefore invalid.
    //
    pDis->fIllegalOp = 1;

DisEnd:

    // In the case of illegal opcode, write known string as an output
    if( pDis->fIllegalOp )
        bPos = sprintf( pDis->szDisasm, "?" );

    // Set the returning values and return with the bInstrLen field
    pDis->bAsciiLen = bPos;
    pDis->bInstrLen = bInstrLen;

    return bInstrLen;
}

/*************************************************************************
 *  Testing the stuff
 *************************************************************************/

#if _TEST_

BYTE stream[16];

#pragma aux testa =  \
"mov ax, bx" \
parm caller;

test()
{
    testa();
}


int main()
{
    TDis386 Dis;
    char szDisasm[ 80 ];
    int i, j, nLen = 0;
    BYTE bOverflow;

    Dis.szDisasm = szDisasm;
    Dis.bDataSize = DIS_32_BIT;
    Dis.bAddressSize = DIS_32_BIT;
    Dis.bpTarget = (BYTE*)&stream;

    stream[0] = 0x0f;
    stream[1] = 0xa1;
    stream[2] = 0x0f;
    stream[3] = 0xa9;
    stream[4] = 0x00;
    stream[5] = 0x00;
    stream[6] = 0x00;
    stream[7] = 0x00;
    stream[8] = 0xC;
    stream[9] = 0xD;

    do
    {
        Dis386( &Dis );
        printf("%08X ", (DWORD)Dis.bpTarget );

        j = (Dis.bInstrLen <= 5)?
          5-Dis.bInstrLen : 0;
        if( Dis.bInstrLen > 5 )
            Dis.bInstrLen = 5,
            bOverflow = 1;
        else
            bOverflow = 0;

        for( i=0; i<Dis.bInstrLen; i++ )
             printf(" %02X", *(Dis.bpTarget + i) );
        for( i=0; i<j; i++ )
             printf("   ");

        printf("%c %s\n",
          bOverflow? '+':' ',
          Dis.szDisasm );

        Dis.bpTarget += Dis.bInstrLen;

    }while( getch() != 27 );

    return 0;
}

#endif
