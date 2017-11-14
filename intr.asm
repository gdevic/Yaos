;************************************************************************/
;*	Copyright (C) 1986-1989 Phar Lap Software, Inc.			*/
;*	Unpublished - rights reserved under the Copyright Laws of the	*/
;*	United States.  Use, duplication, or disclosure by the 		*/
;*	Government is subject to restrictions as set forth in 		*/
;*	subparagraph (c)(1)(ii) of the Rights in Technical Data and 	*/
;*	Computer Software clause at 252.227-7013.			*/
;*	Phar Lap Software, Inc., 60 Aberdeen Ave., Cambridge, MA 02138	*/
;************************************************************************/
comment `********************************************************************

INTR.ASM - This is a C-callable routine used to issue an interrupt.
	  The routine first saves all regs on the stack (except CS,SS & SP)
	  then loads them with the values given in the structure
	  "reg" and then issues the specified interrupt. Upon returning 
	  from the interrupt, all registers are updated in "reg" (including
	  EFLAGS ) and the carry flag is additionally used as the function
	  result ( if carry is set, EAX =1, otherwise EAX = 0).  Next, all 
	  saved registers are restored and control is returned to the caller.


*** C declaration: ***

extern
int	intr(ULONG num, REGS *reg)
    
Calling Arguments:

	num	    number of interrupt to be issued 
	*reg	    registers (except CS,SS &SP) and flags


`****************************************************************************

	include hw386.ah
	include dosx.ah

; REGS structure (as defined by C routine)

REGS	struc

$EAX	dd	?	
$EBX	dd	?
$ECX	dd	?
$EDX	dd	?
$ESI	dd	?
$EDI	dd	?
$EBP	dd	?
$ESP	dd	?
$CS	dw	?
$DS	dw	?
$ES	dw	? 
$FS	dw	?
$GS	dw	?
$SS	dw	?
$EIP	dd	?
$EFLAGS	dd	?

REGS	ends


CGROUP	group	code

code	segment dword 'CODE'
	assume cs:cgroup,ds:cgroup

	public intr
	db	'intr',4		;this is here for C error handler

intr	proc	near

; Stack Frame:

#NUM	equ	(dword ptr 8[ebp])
#REG	equ	(dword ptr 12[ebp])

	push	ebp			;save old EBP 
	mov	ebp,esp			;make EBP a pointer to args

	pushad  			;save general purpose regs
	push	es			;save segment registers
	push	fs
	push	gs

;--- Set up INT instruction ahead

	mov	eax,#NUM		;get interrupt number from stack
	mov	int_num,al		;save operand of INT instruction
	jmp	short flush		;self-modifying code - must flush
					;  pre-fetched instructions
flush:

;--- Get registers from the struc given to us by the caller

	mov	ebx,#REG		;point EBX to register structure

	mov	eax,[ebx].$EAX		;load regs with values 
	mov	ecx,[ebx].$ECX		;  given in structure
	mov	edx,[ebx].$EDX
	mov	esi,[ebx].$ESI
	mov	edi,[ebx].$EDI
	mov	ebp,[ebx].$EBP	
	mov	es,[ebx].$ES
	mov	fs,[ebx].$FS
	mov	gs,[ebx].$GS

	push	ebx			;save pointer to structure
	push	ds			;save segment of structure
	push	eax			;preserve EAX

	mov	eax,[ebx].$EBX		;get new EBX into eax
	mov	ds,[ebx].$DS		;get new DS (addressability
					;  of struc is destroyed by this
					;  instruction)
	mov	ebx,eax			;put new EBX into ebx

	pop	eax			;restore EAX

;--- Issue the interrupt

	db	0cdh			;this is an INT opcode
int_num  db	21h			; followed by the int number

;--- Update the register structure

	push	ds			;save on stack
	push	ebx

	mov	ebx,8[esp]		;recover segment of structure
	mov	ds,bx			;  and put it in DS
	mov	ebx,12[esp]		;recover pointer to structure
	mov	[ebx].$EAX,eax		;update struc's EAX
	pop	[ebx].$EBX		;update struc's EBX
	pop	eax			;pop new DS into eax
	mov	[ebx].$DS,ax		;update struc's DS

;--- Save state of flags (previous instructions have not affected them)

	pushfd				;push EFLAGS on stack
	pop	[ebx].$EFLAGS		;  and pop them into struc's EFLAGS
	add	esp,8			;discard saved DS & EBX

	mov	[ebx].$ECX,ecx		;update all other regs
	mov	[ebx].$EDX,edx
	mov	[ebx].$ESI,esi
	mov	[ebx].$EDI,edi
	mov	[ebx].$EBP,ebp	
	mov	[ebx].$ES,es
	mov	[ebx].$FS,fs
	mov	[ebx].$GS,gs


;--- Restore C environment

	pop	gs			;restore segment registers
	pop	fs
	pop	es
	popad				;pop general registers off of stack

;--- Set up function result

	mov	ecx,#reg		;set up addressability of struc
	mov	eax,FALSE		;assume carry was cleared
	test	[ecx].$EFLAGS,EF_CF	;was carry cleared?
	jz	short i0		;yes,branch
	mov	eax,TRUE		;say carry was set
i0:
	pop	ebp			;fix ebp

	ret				;return to caller

intr	endp
code	ends
	end
