/*********************************************************************
*                                                                    *
*   Module:     dis386p.h                                            *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       04/20/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
.-
    Module Description:

        Private definitions for the disassembler.  No other file 
        should include these.
-.
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  04/20/96   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/

/*********************************************************************
*   Local Variables and defines
**********************************************************************/
/*********************************************************************
*
*   Private portion
*
**********************************************************************/

/*********************************************************************
*
*   Defines for the opcode names
*
**********************************************************************/
#define _aaa           0x01
#define _aad           0x02
#define _aam           0x03
#define _aas           0x04
#define _adc           0x05
#define _add           0x06
#define _and           0x07
#define _arpl          0x08
#define _bound         0x09
#define _bsf           0x0a
#define _bsr           0x0b
#define _bt            0x0c
#define _btc           0x0d
#define _btr           0x0e
#define _bts           0x0f
#define _call          0x10
#define _cbw           0x11
#define _cwde          0x12
#define _clc           0x13
#define _cld           0x14
#define _cli           0x15
#define _clts          0x16
#define _cmc           0x17
#define _cmp           0x18
#define _cmps          0x19
#define _cmpsb         0x1a
#define _cmpsw         0x1b
#define _cmpsd         0x1c
#define _cwd           0x1d
#define _cdq           0x1e
#define _daa           0x1f
#define _das           0x20
#define _dec           0x21
#define _div           0x22
#define _enter         0x23
#define _hlt           0x24
#define _idiv          0x25
#define _imul          0x26
#define _in            0x27
#define _inc           0x28
#define _ins           0x29
#define _insb          0x2a
#define _insw          0x2b
#define _insd          0x2c
#define _int           0x2d
#define _into          0x2e
#define _iret          0x2f
#define _iretd         0x30
#define _jo            0x31
#define _jno           0x32
#define _jb            0x33
#define _jnb           0x34
#define _jz            0x35
#define _jnz           0x36
#define _jbe           0x37
#define _jnbe          0x38
#define _js            0x39
#define _jns           0x3a
#define _jp            0x3b
#define _jnp           0x3c
#define _jl            0x3d
#define _jnl           0x3e
#define _jle           0x3f
#define _jnle          0x40
#define _jmp           0x41
#define _lahf          0x42
#define _lar           0x43
#define _lea           0x44
#define _leave         0x45
#define _lgdt          0x46
#define _lidt          0x47
#define _lgs           0x48
#define _lss           0x49
#define _lds           0x4a
#define _les           0x4b
#define _lfs           0x4c
#define _lldt          0x4d
#define _lmsw          0x4e
#define _lock          0x4f
#define _lods          0x50
#define _lodsb         0x51
#define _lodsw         0x52
#define _lodsd         0x53
#define _loop          0x54
#define _loope         0x55
#define _loopz         0x56
#define _loopne        0x57
#define _loopnz        0x58
#define _lsl           0x59
#define _ltr           0x5a
#define _mov           0x5b
#define _movs          0x5c
#define _movsb         0x5d
#define _movsw         0x5e
#define _movsd         0x5f
#define _movsx         0x60
#define _movzx         0x61
#define _mul           0x62
#define _neg           0x63
#define _nop           0x64
#define _not           0x65
#define _or            0x66
#define _out           0x67
#define _outs          0x68
#define _outsb         0x69
#define _outsw         0x6a
#define _outsd         0x6b
#define _pop           0x6c
#define _popa          0x6d
#define _popad         0x6e
#define _popf          0x6f
#define _popfd         0x70
#define _push          0x71
#define _pusha         0x72
#define _pushad        0x73
#define _pushf         0x74
#define _pushfd        0x75
#define _rcl           0x76
#define _rcr           0x77
#define _rol           0x78
#define _ror           0x79
#define _rep           0x7a
#define _repe          0x7b
#define _repz          0x7c
#define _repne         0x7d
#define _repnz         0x7e
#define _ret           0x7f
#define _sahf          0x80
#define _sal           0x81
#define _sar           0x82
#define _shl           0x83
#define _shr           0x84
#define _sbb           0x85
#define _scas          0x86
#define _scasb         0x87
#define _scasw         0x88
#define _scasd         0x89
#define _set           0x8a
#define _sgdt          0x8b
#define _sidt          0x8c
#define _shld          0x8d
#define _shrd          0x8e
#define _sldt          0x8f
#define _smsw          0x90
#define _stc           0x91
#define _std           0x92
#define _sti           0x93
#define _stos          0x94
#define _stosb         0x95
#define _stosw         0x96
#define _stosd         0x97
#define _str           0x98
#define _sub           0x99
#define _test          0x9a
#define _verr          0x9b
#define _verw          0x9c
#define _wait          0x9d
#define _xchg          0x9e
#define _xlat          0x9f
#define _xlatb         0xa0
#define _xor           0xa1
#define _jcxz          0xa2
#define _loada         0xa3
#define _invd          0xa4
#define _wbinv         0xa5
#define _seto          0xa6
#define _setno         0xa7
#define _setb          0xa8
#define _setnb         0xa9
#define _setz          0xaa
#define _setnz         0xab
#define _setbe         0xac
#define _setnbe        0xad
#define _sets          0xae
#define _setns         0xaf
#define _setp          0xb0
#define _setnp         0xb1
#define _setl          0xb2
#define _setnl         0xb3
#define _setle         0xb4
#define _setnle        0xb5

/*********************************************************************
*
*   Opcode names as ASCII zero-terminated strings
*
**********************************************************************/
static char* sNames[] = {
"aaa",              /* 0x01 */
"aad",              /* 0x02 */
"aam",              /* 0x03 */
"aas",              /* 0x04 */
"adc",              /* 0x05 */
"add",              /* 0x06 */
"and",              /* 0x07 */
"arpl",             /* 0x08 */
"bound",            /* 0x09 */
"bsf",              /* 0x0a */
"bsr",              /* 0x0b */
"bt",               /* 0x0c */
"btc",              /* 0x0d */
"btr",              /* 0x0e */
"bts",              /* 0x0f */
"call",             /* 0x10 */
"cbw",              /* 0x11 */
"cwde",             /* 0x12 */
"clc",              /* 0x13 */
"cld",              /* 0x14 */
"cli",              /* 0x15 */
"clts",             /* 0x16 */
"cmc",              /* 0x17 */
"cmp",              /* 0x18 */
"cmps",             /* 0x19 */
"cmpsb",            /* 0x1a */
"cmpsw",            /* 0x1b */
"cmpsd",            /* 0x1c */
"cwd",              /* 0x1d */
"cdq",              /* 0x1e */
"daa",              /* 0x1f */
"das",              /* 0x20 */
"dec",              /* 0x21 */
"div",              /* 0x22 */
"enter",            /* 0x23 */
"hlt",              /* 0x24 */
"idiv",             /* 0x25 */
"imul",             /* 0x26 */
"in",               /* 0x27 */
"inc",              /* 0x28 */
"ins",              /* 0x29 */
"insb",             /* 0x2a */
"insw",             /* 0x2b */
"insd",             /* 0x2c */
"int",              /* 0x2d */
"into",             /* 0x2e */
"iret",             /* 0x2f */
"iretd",            /* 0x30 */
"jo",               /* 0x31 */
"jno",              /* 0x32 */
"jb",               /* 0x33 */
"jnb",              /* 0x34 */
"jz",               /* 0x35 */
"jnz",              /* 0x36 */
"jbe",              /* 0x37 */
"jnbe",             /* 0x38 */
"js",               /* 0x39 */
"jns",              /* 0x3a */
"jp",               /* 0x3b */
"jnp",              /* 0x3c */
"jl",               /* 0x3d */
"jnl",              /* 0x3e */
"jle",              /* 0x3f */
"jnle",             /* 0x40 */
"jmp",              /* 0x41 */
"lahf",             /* 0x42 */
"lar",              /* 0x43 */
"lea",              /* 0x44 */
"leave",            /* 0x45 */
"lgdt",             /* 0x46 */
"lidt",             /* 0x47 */
"lgs",              /* 0x48 */
"lss",              /* 0x49 */
"lds",              /* 0x4a */
"les",              /* 0x4b */
"lfs",              /* 0x4c */
"lldt",             /* 0x4d */
"lmsw",             /* 0x4e */
"lock",             /* 0x4f */
"lods",             /* 0x50 */
"lodsb",            /* 0x51 */
"lodsw",            /* 0x52 */
"lodsd",            /* 0x53 */
"loop",             /* 0x54 */
"loope",            /* 0x55 */
"loopz",            /* 0x56 */
"loopne",           /* 0x57 */
"loopnz",           /* 0x58 */
"lsl",              /* 0x59 */
"ltr",              /* 0x5a */
"mov",              /* 0x5b */
"movs",             /* 0x5c */
"movsb",            /* 0x5d */
"movsw",            /* 0x5e */
"movsd",            /* 0x5f */
"movsx",            /* 0x60 */
"movzx",            /* 0x61 */
"mul",              /* 0x62 */
"neg",              /* 0x63 */
"nop",              /* 0x64 */
"not",              /* 0x65 */
"or",               /* 0x66 */
"out",              /* 0x67 */
"outs",             /* 0x68 */
"outsb",            /* 0x69 */
"outsw",            /* 0x6a */
"outsd",            /* 0x6b */
"pop",              /* 0x6c */
"popa",             /* 0x6d */
"popad",            /* 0x6e */
"popf",             /* 0x6f */
"popfd",            /* 0x70 */
"push",             /* 0x71 */
"pusha",            /* 0x72 */
"pushad",           /* 0x73 */
"pushf",            /* 0x74 */
"pushfd",           /* 0x75 */
"rcl",              /* 0x76 */
"rcr",              /* 0x77 */
"rol",              /* 0x78 */
"ror",              /* 0x79 */
"rep",              /* 0x7a */
"repe",             /* 0x7b */
"repz",             /* 0x7c */
"repne",            /* 0x7d */
"repnz",            /* 0x7e */
"ret",              /* 0x7f */
"sahf",             /* 0x80 */
"sal",              /* 0x81 */
"sar",              /* 0x82 */
"shl",              /* 0x83 */
"shr",              /* 0x84 */
"sbb",              /* 0x85 */
"scas",             /* 0x86 */
"scasb",            /* 0x87 */
"scasw",            /* 0x88 */
"scasd",            /* 0x89 */
"set",              /* 0x8a */
"sgdt",             /* 0x8b */
"sidt",             /* 0x8c */
"shld",             /* 0x8d */
"shrd",             /* 0x8e */
"sldt",             /* 0x8f */
"smsw",             /* 0x90 */
"stc",              /* 0x91 */
"std",              /* 0x92 */
"sti",              /* 0x93 */
"stos",             /* 0x94 */
"stosb",            /* 0x95 */
"stosw",            /* 0x96 */
"stosd",            /* 0x97 */
"str",              /* 0x98 */
"sub",              /* 0x99 */
"test",             /* 0x9a */
"verr",             /* 0x9b */
"verw",             /* 0x9c */
"wait",             /* 0x9d */
"xchg",             /* 0x9e */
"xlat",             /* 0x9f */
"xlatb",            /* 0xa0 */
"xor",              /* 0xa1 */
"jcxz",             /* 0xa2 */
"loadall",          /* 0xa3 */
"invd",             /* 0xa4 */
"wbinvd",           /* 0xa5 */
"seto",             /* 0xa6 */
"setno",            /* 0xa7 */
"setb",             /* 0xa8 */
"setnb",            /* 0xa9 */
"setz",             /* 0xaa */
"setnz",            /* 0xab */
"setbe",            /* 0xac */
"setnbe",           /* 0xad */
"sets",             /* 0xae */
"setns",            /* 0xaf */
"setp",             /* 0xb0 */
"setnp",            /* 0xb1 */
"setl",             /* 0xb2 */
"setnl",            /* 0xb3 */
"setle",            /* 0xb4 */
"setnle"            /* 0xb5 */
};


/*********************************************************************
*
*   Defines to be used in the opcode table as the opcode special
*   code.  They will fit in the opcode field so their value does
*   not overlap with the opcode defines.
*
**********************************************************************/
#define _NDEF          0xC0      // Udefined/reserved opcode
#define _2BESC         0xC1      // 2 byte escape code
#define _ESC           0xC2      // ESC - Escape to coprocessor instruction set
#define _S_ES          0xC3      // Segment ES override
#define _S_CS          0xC4      // Segment CS override
#define _S_SS          0xC5      // Segment SS override
#define _S_DS          0xC6      // Segment DS override
#define _S_FS          0xC7      // Segment FS override
#define _S_GS          0xC8      // Segment GS override
#define _OPSIZ         0xC9      // Operand size override
#define _ADSIZ         0xCA      // Address size override
#define _GRP1          0xCB      // Group 1 extended opcode
#define _GRP2          0xCC      // Group 2 extended opcode
#define _GRP3          0xCD      // Group 3 extended opcode
#define _GRP4          0xCE      // Group 4 extended opcode
#define _GRP5          0xCF      // Group 5 extended opcode
#define _GRP6          0xD0      // Group 6 extended opcode
#define _GRP7          0xD1      // Group 7 extended opcode
#define _GRP8          0xD2      // Group 8 extended opcode
#define _GRP9          0xD3      // Group 9 extended opcode (group 3 w/word)

/*********************************************************************
*
*   Define to use in the place of the third argument to an opcode
*   to define an instruction that changes its name depending on the
*   data width: for 16 bit data use the original name, for 32 bit data
*   use the name that immediately follows it
*   Example: 16 bit = stosw / 32 bit = stosd
*
**********************************************************************/
#define _NM            0xff

/*********************************************************************
*
*   Addressing modes argument definiton for the opcodes in a table
*
**********************************************************************/
#define _Eb            0x01
#define _Ev            0x02
#define _Ew            0x03
#define _Ep            0x04
#define _Gb            0x05
#define _Gv            0x06
#define _M             0x07
#define _Ma            0x08
#define _Mp            0x09
#define _Ms            0x0a
#define _Rd            0x0b
#define _Rw            0x0c
#define _Sw            0x0d
#define _Cd            0x0e
#define _Dd            0x0f
#define _Td            0x10

#define _Ob            0x15 // <- they dont use modR/M additional byte
#define _Ov            0x16 //  ||
#define _Ib            0x17 //  \/
#define _Iv            0x18 //
#define _Iw            0x19 //
#define _Yb            0x1a //
#define _Yv            0x1b //
#define _Xb            0x1c //
#define _Xv            0x1d //
#define _Jb            0x1e //
#define _Jv            0x1f //
#define _Ap            0x20 //
#define _Fv            0x21 //
#define _imm8          0x22 //
#define _immw          0x23 //
#define _1             0x24 //
#define _3             0x25 //
#define _DX            0x26 //
#define _AL            0x27 //
#define _AH            0x28 //
#define _BL            0x29 //
#define _BH            0x2a //
#define _CL            0x2b //
#define _CH            0x2c //
#define _DL            0x2d //
#define _DH            0x2e //
#define _CS            0x2f //
#define _DS            0x30 //
#define _ES            0x31 //
#define _SS            0x32 //
#define _FS            0x33 //
#define _GS            0x34 //
#define _eAX           0x35 //
#define _eCX           0x36 //
#define _eDX           0x37 //
#define _eBX           0x38 //
#define _eSP           0x39 //
#define _eBP           0x3a //
#define _eSI           0x3b //
#define _eDI           0x3c //


#define PK(Opcode,Dest,Src)       ( ((Dest) << 16) | ((Src) << 8) | (Opcode) )
#define P3(Opcode,Dest,Src,Imm)   ( ((Dest) << 16) | ((Src) << 8) | (Opcode) | ((Imm) << 24) )

/*********************************************************************
*
*   Table for the 1 byte opcodes
*
**********************************************************************/
static DWORD Op1[ 16 ][ 16 ] = {
{ PK(_add  ,_Eb ,_Gb  ), PK(_add  ,_Ev ,_Gv  ), PK(_add  ,_Gb ,_Eb  ), PK(_add  ,_Gv ,_Ev  ), PK(_add  ,_AL ,_Ib  ), PK(_add  ,_eAX,_Iv  ), PK(_push ,_ES ,0    ), PK(_pop  ,_ES ,0    ),
  PK(_or   ,_Eb ,_Gb  ), PK(_or   ,_Ev ,_Gv  ), PK(_or   ,_Gb ,_Eb  ), PK(_or   ,_Gv ,_Ev  ), PK(_or   ,_AL ,_Ib  ), PK(_or   ,_eAX,_Iv  ), PK(_push ,_CS ,0    ), PK(_2BESC,0   ,0    )
},
{ PK(_adc  ,_Eb ,_Gb  ), PK(_adc  ,_Ev ,_Gv  ), PK(_adc  ,_Gb ,_Eb  ), PK(_adc  ,_Gv ,_Ev  ), PK(_adc  ,_AL ,_Ib  ), PK(_adc  ,_eAX,_Iv  ), PK(_push ,_SS ,0    ), PK(_pop  ,_SS ,0    ),
  PK(_sbb  ,_Eb ,_Gb  ), PK(_sbb  ,_Ev ,_Gv  ), PK(_sbb  ,_Gb ,_Eb  ), PK(_sbb  ,_Gv ,_Ev  ), PK(_sbb  ,_AL ,_Ib  ), PK(_sbb  ,_eAX,_Iv  ), PK(_push ,_DS ,0    ), PK(_pop  ,_DS ,0    )
},
{ PK(_and  ,_Eb ,_Gb  ), PK(_and  ,_Ev ,_Gv  ), PK(_and  ,_Gb ,_Eb  ), PK(_and  ,_Gv ,_Ev  ), PK(_and  ,_AL ,_Ib  ), PK(_and  ,_eAX,_Iv  ), PK(_S_ES ,0   ,0    ), PK(_daa  ,0   ,0    ),
  PK(_sub  ,_Eb ,_Gb  ), PK(_sub  ,_Ev ,_Gv  ), PK(_sub  ,_Gb ,_Eb  ), PK(_sub  ,_Gv ,_Ev  ), PK(_sub  ,_AL ,_Ib  ), PK(_sub  ,_eAX,_Iv  ), PK(_S_CS ,0   ,0    ), PK(_das  ,0   ,0    )
},
{ PK(_xor  ,_Eb ,_Gb  ), PK(_xor  ,_Ev ,_Gv  ), PK(_xor  ,_Gb ,_Eb  ), PK(_xor  ,_Gv ,_Ev  ), PK(_xor  ,_AL ,_Ib  ), PK(_xor  ,_eAX,_Iv  ), PK(_S_SS ,0   ,0    ), PK(_aaa  ,0   ,0    ),
  PK(_cmp  ,_Eb ,_Gb  ), PK(_cmp  ,_Ev ,_Gv  ), PK(_cmp  ,_Gb ,_Eb  ), PK(_cmp  ,_Gv ,_Ev  ), PK(_cmp  ,_AL ,_Ib  ), PK(_cmp  ,_eAX,_Iv  ), PK(_S_DS ,0   ,0    ), PK(_aas  ,0   ,0    )
},
{ PK(_inc  ,_eAX,0    ), PK(_inc  ,_eCX,0    ), PK(_inc  ,_eDX,0    ), PK(_inc  ,_eBX,0    ), PK(_inc  ,_eSP,0    ), PK(_inc  ,_eBP,0    ), PK(_inc  ,_eSI,0    ), PK(_inc  ,_eDI,0    ),
  PK(_dec  ,_eAX,0    ), PK(_dec  ,_eCX,0    ), PK(_dec  ,_eDX,0    ), PK(_dec  ,_eBX,0    ), PK(_dec  ,_eSP,0    ), PK(_dec  ,_eBP,0    ), PK(_dec  ,_eSI,0    ), PK(_dec  ,_eDI,0    )
},
{ PK(_push ,_eAX,0    ), PK(_push ,_eCX,0    ), PK(_push ,_eDX,0    ), PK(_push ,_eBX,0    ), PK(_push ,_eSP,0    ), PK(_push ,_eBP,0    ), PK(_push ,_eSI,0    ), PK(_push ,_eDI,0    ),
  PK(_pop  ,_eAX,0    ), PK(_pop  ,_eCX,0    ), PK(_pop  ,_eDX,0    ), PK(_pop  ,_eBX,0    ), PK(_pop  ,_eSP,0    ), PK(_pop  ,_eBP,0    ), PK(_pop  ,_eSI,0    ), PK(_pop  ,_eDI,0    )
},
{ P3(_pusha,0   ,0,_NM), P3(_popa ,0   ,0,_NM), PK(_bound,_Gv ,_Ma  ), PK(_arpl ,_Ew ,_Rw  ), PK(_S_FS ,0   ,0    ), PK(_S_GS ,0   ,0    ), PK(_OPSIZ,0   ,0    ), PK(_ADSIZ,0   ,0    ),
  PK(_push ,_Iv ,0    ), P3(_imul,_Gv,_Ev,_Iv), PK(_push ,_Ib ,0    ), P3(_imul,_Gv,_Ev,_Ib), PK(_insb ,_Yb ,_DX  ), P3(_insw,_Yv,_DX,_NM), PK(_outsb,_DX ,_Xb  ), P3(_outsw,_DX,_Xv,_NM)
},
{ PK(_jo   ,_Jb ,0    ), PK(_jno  ,_Jb ,0    ), PK(_jb   ,_Jb ,0    ), PK(_jnb  ,_Jb ,0    ), PK(_jz   ,_Jb ,0    ), PK(_jnz  ,_Jb ,0    ), PK(_jbe  ,_Jb ,0    ), PK(_jnbe ,_Jb ,0    ),
  PK(_js   ,_Jb ,0    ), PK(_jns  ,_Jb ,0    ), PK(_jp   ,_Jb ,0    ), PK(_jnp  ,_Jb ,0    ), PK(_jl   ,_Jb ,0    ), PK(_jnl  ,_Jb ,0    ), PK(_jle  ,_Jb ,0    ), PK(_jnle ,_Jb ,0    )
},
{ PK(_GRP1 ,_Eb ,_Ib  ), PK(_GRP1 ,_Ev ,_Iv  ), PK(_mov  ,_AL ,_imm8), PK(_GRP1 ,_Ev ,_Ib  ), PK(_test ,_Eb ,_Gb  ), PK(_test ,_Ev ,_Gv  ), PK(_xchg ,_Eb ,_Gb  ), PK(_xchg ,_Ev ,_Gv  ),
  PK(_mov  ,_Eb ,_Gb  ), PK(_mov  ,_Ev ,_Gv  ), PK(_mov  ,_Gb ,_Eb  ), PK(_mov  ,_Gv ,_Ev  ), PK(_mov  ,_Ew ,_Sw  ), PK(_lea  ,_Gv ,_M   ), PK(_mov  ,_Sw ,_Ew  ), PK(_pop  ,_Ev ,0    )
},
{ PK(_nop  ,0   ,0    ), PK(_xchg ,_eCX,_eAX ), PK(_xchg ,_eDX,_eAX ), PK(_xchg ,_eBX,_eAX ), PK(_xchg ,_eSP,_eAX ), PK(_xchg ,_eBP,_eAX ), PK(_xchg ,_eSI,_eAX ), PK(_xchg ,_eDI,_eAX ),
  P3(_cbw  ,0   ,0,_NM), P3(_cwd,0 ,0  ,_NM  ), PK(_call ,_Ap ,0    ), PK(_wait ,0   ,0    ), P3(_pushf,_Fv ,0,_NM), P3(_popf ,_Fv ,0,_NM), PK(_sahf ,0   ,0    ), PK(_lahf ,0   ,0    )
},
{ PK(_mov  ,_AL ,_Ob  ), PK(_mov  ,_eAX,_Ov  ), PK(_mov  ,_Ob ,_AL  ), PK(_mov  ,_Ov ,_eAX ), PK(_movsb,_Xb ,_Yb  ), PK(_movsw,_Xv ,_Yv  ), PK(_cmpsb,_Xb ,_Yb  ), PK(_cmpsw,_Xv ,_Yv  ),
  PK(_test ,_AL ,_Ib  ), PK(_test ,_eAX,_Iv  ), PK(_stosb,_Yb ,_Iv  ), P3(_stosw,_Yb,_eAX,_NM),PK(_lodsb,_AL,_Xb  ), P3(_lodsw,_eAX,_Xv,_NM),PK(_scasb,_AL,_Xb  ), P3(_scasw,_eAX,_Xv,_NM)
},
{ PK(_mov  ,_AL ,_imm8), PK(_mov  ,_CL ,_imm8), PK(_mov  ,_DL ,_imm8), PK(_mov  ,_BL ,_imm8), PK(_mov  ,_AH ,_imm8), PK(_mov  ,_CH ,_imm8), PK(_mov  ,_DH ,_imm8), PK(_mov  ,_BH ,_imm8),
  PK(_mov  ,_eAX,_immw), PK(_mov  ,_eCX,_immw), PK(_mov  ,_eDX,_immw), PK(_mov  ,_eBX,_immw), PK(_mov  ,_eSP,_immw), PK(_mov  ,_eBP,_immw), PK(_mov  ,_eSI,_immw), PK(_mov  ,_eDI,_immw)
},
{ PK(_GRP2 ,_Eb ,_Ib  ), PK(_GRP2 ,_Ev ,_Ib  ), PK(_ret  ,_Iw ,0    ), PK(_ret  ,0   ,0    ), PK(_les  ,_Gv ,_Mp  ), PK(_lds  ,_Gv ,_Mp  ), PK(_mov  ,_Eb ,_Ib  ), PK(_mov  ,_Ev ,_Iv  ),
  PK(_enter,_Iw ,_Ib  ), PK(_leave,0   ,0    ), PK(_ret  ,_Iw ,0    ), PK(_ret  ,0   ,0    ), PK(_int  ,_3  ,0    ), PK(_int  ,_Ib ,0    ), PK(_into ,0   ,0    ), P3(_iret ,0   ,0,_NM)
},
{ PK(_GRP2 ,_Eb ,_1   ), PK(_GRP2 ,_Ev ,_1   ), PK(_GRP2 ,_Eb ,_CL  ), PK(_GRP2 ,_Ev ,_CL  ), PK(_aam  ,0   ,0    ), PK(_aad  ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_xlat ,0   ,0    ),
  PK(_ESC  ,0   ,0    ), PK(_ESC  ,0   ,0    ), PK(_ESC  ,0   ,0    ), PK(_ESC  ,0   ,0    ), PK(_ESC  ,0   ,0    ), PK(_ESC  ,0   ,0    ), PK(_ESC  ,0   ,0    ), PK(_ESC  ,0   ,0    )
},
{ PK(_loopne,_Jb,0    ), PK(_loope,_Jb ,0    ), PK(_loop ,_Jb ,0    ), PK(_jcxz ,_Jb ,0    ), PK(_in   ,_AL ,_Ib  ), PK(_in   ,_eAX,_Ib  ), PK(_out  ,_Ib ,_AL  ), PK(_out  ,_Ib ,_eAX ),
  PK(_call ,_Jv ,0    ), PK(_jmp  ,_Jv ,0    ), PK(_jmp  ,_Ap ,0    ), PK(_jmp  ,_Jb ,0    ), PK(_in   ,_AL ,_DX  ), PK(_in   ,_eAX,_DX  ), PK(_out  ,_DX ,_AL  ), PK(_out  ,_DX ,_eAX )
},
{ PK(_lock ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_repne,0   ,0    ), PK(_rep  ,0   ,0    ), PK(_hlt  ,0   ,0    ), PK(_cmc  ,0   ,0    ), PK(_GRP3 ,_Eb ,0    ), PK(_GRP9 ,_Ev ,0    ),
  PK(_clc  ,0   ,0    ), PK(_stc  ,0   ,0    ), PK(_cli  ,0   ,0    ), PK(_sti  ,0   ,0    ), PK(_cld  ,0   ,0    ), PK(_std  ,0   ,0    ), PK(_GRP4 ,0   ,0    ), PK(_GRP5 ,0   ,0    )
}
};


/*********************************************************************
*
*   Table for the 2 byte opcodes
*
**********************************************************************/
static DWORD Op2[ 16 ][ 16 ] = {
{ PK(_GRP6 ,0   ,0    ), PK(_GRP7 ,0   ,0    ), PK(_lar  ,_Gv ,_Ew  ), PK(_lsl  ,_Gv ,_Ew  ), PK(_NDEF ,0   ,0    ), PK(_loada,0   ,0    ), PK(_clts ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_invd ,0   ,0    ), PK(_wbinv,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_mov  ,_Rd ,_Cd  ), PK(_mov  ,_Rd ,_Dd  ), PK(_mov  ,_Cd ,_Rd  ), PK(_mov  ,_Dd ,_Rd  ), PK(_mov  ,_Rd ,_Td  ), PK(_NDEF ,0   ,0    ), PK(_mov  ,_Td ,_Rd  ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_jo   ,_Jv ,0    ), PK(_jno  ,_Jv ,0    ), PK(_jb   ,_Jv ,0    ), PK(_jnb  ,_Jv ,0    ), PK(_jz   ,_Jv ,0    ), PK(_jnz  ,_Jv ,0    ), PK(_jbe  ,_Jv ,0    ), PK(_jnbe ,_Jv ,0    ),
  PK(_js   ,_Jv ,0    ), PK(_jns  ,_Jv ,0    ), PK(_jp   ,_Jv ,0    ), PK(_jnp  ,_Jv ,0    ), PK(_jl   ,_Jv ,0    ), PK(_jnl  ,_Jv ,0    ), PK(_jle  ,_Jv ,0    ), PK(_jnle ,_Jv ,0    )
},
{ PK(_seto ,_Eb ,0    ), PK(_setno,_Eb ,0    ), PK(_setb ,_Eb ,0    ), PK(_setnb,_Eb ,0    ), PK(_setz ,_Eb ,0    ), PK(_setnz,_Eb ,0    ), PK(_setbe,_Eb ,0    ), PK(_setnbe,_Eb,0    ),
  PK(_sets ,_Eb ,0    ), PK(_setns,_Eb ,0    ), PK(_setp ,_Eb ,0    ), PK(_setnp,_Eb ,0    ), PK(_setl ,_Eb ,0    ), PK(_setnl,_Eb ,0    ), PK(_setle,_Eb ,0    ), PK(_setnle,_Eb,0    )
},
{ PK(_push ,_FS ,0    ), PK(_pop  ,_FS ,0    ), PK(_NDEF ,0   ,0    ), PK(_bt   ,_Ev ,_Gv  ), P3(_shld,_Ev,_Gv,_Ib), P3(_shld,_Ev,_Gv,_CL), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_push ,_GS ,0    ), PK(_pop  ,_GS ,0    ), PK(_NDEF ,0   ,0    ), PK(_bts  ,_Ev ,_Gv  ), P3(_shrd,_Ev,_Gv,_Ib), P3(_shrd,_Ev,_Gv,_CL), PK(_NDEF ,0   ,0    ), PK(_imul ,_Gv ,_Ev  )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_lss  ,_Gv ,_Mp  ), PK(_btr  ,_Ev ,_Gv  ), PK(_lfs  ,_Gv ,_Mp  ), PK(_lgs  ,_Gv ,_Mp  ), PK(_movzx,_Gv ,_Eb  ), PK(_movzx,_Gv ,_Ew  ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_GRP8 ,_Ev ,_Ib  ), PK(_btc  ,_Ev ,_Gv  ), PK(_bsf  ,_Gv ,_Ev  ), PK(_bsr  ,_Gv ,_Ev  ), PK(_movsx,_Gv ,_Eb  ), PK(_movsx,_Gv ,_Ew  )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ),
  PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
}
};


/*********************************************************************
*
*   Table for groups codes 1-9
*
**********************************************************************/
static DWORD Groups[ 9 ][ 8 ] = {
{ PK(_add  ,0   ,0    ), PK(_or   ,0   ,0    ), PK(_adc  ,0   ,0    ), PK(_sbb  ,0   ,0    ), PK(_and  ,0   ,0    ), PK(_sub  ,0   ,0    ), PK(_xor  ,0   ,0    ), PK(_cmp  ,0   ,0    )
},
{ PK(_rol  ,0   ,0    ), PK(_ror  ,0   ,0    ), PK(_rcl  ,0   ,0    ), PK(_rcr  ,0   ,0    ), PK(_sal  ,0   ,0    ), PK(_shr  ,0   ,0    ), PK(_shl  ,0   ,0    ), PK(_sar  ,0   ,0    )
},
{ PK(_test ,_Eb ,_Ib  ), PK(_test ,_Eb ,_Ib  ), PK(_not  ,0   ,0    ), PK(_neg  ,0   ,0    ), PK(_mul  ,_Eb ,_AL  ), PK(_imul ,_Eb ,_AL  ), PK(_div  ,_Eb ,_AL  ), PK(_idiv ,_Eb ,_AL  )
},
{ PK(_inc  ,_Eb ,0    ), PK(_dec  ,_Eb ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_inc  ,_Ev ,0    ), PK(_dec  ,_Ev ,0    ), PK(_call ,_Ev ,0    ), PK(_call ,_Ep ,0    ), PK(_jmp  ,_Ev ,0    ), PK(_jmp  ,_Ep ,0    ), PK(_push ,_Ev ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_sldt ,_Ew ,0    ), PK(_str  ,_Ew ,0    ), PK(_lldt ,_Ew ,0    ), PK(_ltr  ,_Ew ,0    ), PK(_verr ,_Ew ,0    ), PK(_verw ,_Ew ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_sgdt ,_Ms ,0    ), PK(_sidt ,_Ms ,0    ), PK(_lgdt ,_Ms ,0    ), PK(_lidt ,_Ms ,0    ), PK(_smsw ,_Ew ,0    ), PK(_NDEF ,0   ,0    ), PK(_lmsw ,_Ew ,0    ), PK(_NDEF ,0   ,0    )
},
{ PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_NDEF ,0   ,0    ), PK(_bt   ,0   ,0    ), PK(_bts  ,0   ,0    ), PK(_btr  ,0   ,0    ), PK(_btc  ,0   ,0    )
},
{ PK(_test ,_Ev ,_Iv  ), PK(_test ,_Ev ,_Iv  ), PK(_not  ,0   ,0    ), PK(_neg  ,0   ,0    ), PK(_mul  ,_Ev ,_eAX ), PK(_imul ,_Ev ,_eAX ), PK(_div  ,_Ev ,_eAX ), PK(_idiv ,_Ev ,_eAX )
},
};


/*********************************************************************
*
*   Generic messages
*
**********************************************************************/
static char *sBytePtr  = "byte ptr ";
static char *sWordPtr  = "word ptr ";
static char *sFwordPtr = "fword ptr ";
static char *sDwordPtr = "dword ptr ";


/*********************************************************************
*
*   Different register combinations
*
**********************************************************************/

static char *sGenReg16_32[ 2 ][ 8 ] = {
{ "ax","cx","dx","bx","sp","bp","si","di" },
{ "eax","ecx","edx","ebx","esp","ebp","esi","edi" }
};

static char *sSeg[ 8 ] = {
"es","cs","ss","ds","fs","gs","?","?"
};

static char *sScale[ 4 ] = {
"",
"2*",
"4*",
"8*"
};

static char *sAdr1[ 2 ][ 8 ] = {
{ "bx+si","bx+di","bp+si","bp+di","si","di","bp","bx" },
{ "eax","ecx","edx","ebx","?","ebp","esi","edi" }
};

static char *sRegs1[ 2 ][ 2 ][ 8 ] = {
{{ "al","cl","dl","bl","ah","ch","dh","bh" },
 { "ax","cx","dx","bx","sp","bp","si","di" } },
{{ "al","cl","dl","bl","ah","ch","dh","bh" },
 { "eax","ecx","edx","ebx","esp","ebp","esi","edi" } }
};

static char *sRegs2[] = {
"dx",
"al",
"ah",
"bl",
"bh",
"cl",
"ch",
"dl",
"dh",
"cs",
"ds",
"es",
"ss",
"fs",
"gs"
};

static char *sControl[ 8 ] = {
"cr0","?","cr2","cr3","?","?","?","?"
};

static char *sDebug[ 8 ] = {
"dr0","dr1","dr2","dr3","?","?","dr6","dr7"
};

static char *sTest[ 8 ] = {
"?","?","?","?","?","?","tr6","tr7"
};

static char *sYptr[ 2 ] = {
"[di]",
"[edi]"
};

static char *sXptr[ 2 ] = {
"[si]",
"[esi]"
};


