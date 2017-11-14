;******************************************************************************
;
;   Module:     kernel.inc
;
;   Revision:   1.00
;
;   Date:       08/03/96
;
;   Author:     Goran Devic
;
;******************************************************************************
;
;   Module Description:
;
;         This is a kernel include file for assembly-level modules
;
;******************************************************************************
;
;   Changes:
;
;   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
; --------   ----  ---------------------------------------------   -----------
; 08/03/96   1.00  Original                                        Goran Devic
; --------   ----  ---------------------------------------------   -----------
;******************************************************************************

;
; Selectors defines
;
NULLSEL      equ       0000h               ; not used
CODESEL      equ       0008h               ; selector for kernel code
DATASEL      equ       0010h               ; selector for kernel data/stack
GLOBALSEL    equ       0018h               ; global (base 0) selector
TSS_SEL      equ       0020h               ; task segment selector


