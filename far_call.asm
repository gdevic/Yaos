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

FAR_CALL.ASM - This is a C-callable routine used to issue a far call to a
	  subroutine. This routine first saves all regs on the stack 
	  (except CS,SS & ESP)  then loads them with the values given in 
	  the structure "reg". The next steps are saving the current SS and
	  ESP in local storage, loading SS and ESP with the corresponding
	  values in the register structure and then making the far call. 
	  Upon returning from the subroutine, first the old SS and ESP are 
	  restored then all registers are updated in "reg" (including 
	  EFLAGS ) and the carry flag is additionally used as the function
	  result ( if carry is set, EAX =1, otherwise EAX = 0).  Next, all 
	  saved registers are restored and control is returned to the caller.

	  Note: The EFLAGS register is not loaded with the value passed in
	  	the register structure before making the far_call. It is used
		only to pass back the new flags to the caller.

	  Warning: This code assumes that CS and DS point at the same place.

*** C declaration: ***

extern
int	far_call(REGS *regst)
    
Calling Arguments:

	*regst          = structure containing registers

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
$EFLAGS dd	?

REGS	ends


CGROUP	group	code

code	segment dword 'CODE'
        	assume cs:cgroup,ds:cgroup
        	public far_call,call_inst

        	db	'far_call',8	;this is here for C error handler

far_call	proc	near

; Stack Frame:

#REG	equ	(dword ptr 8[ebp])

;--- Start of code. Save C environment.

	jmp	short skip		;skip pointer

far_rtn	df	?		;indirect address of routine to call

skip:	push	ebp			;save old EBP 
	mov	ebp,esp			;make EBP a pointer to args

	pushad  			;save general purpose regs
	push	es			;save segment registers
	push	fs
	push	gs

;--- Get registers from the struc given to us by the caller

	mov	ebx,#REG		;point EBX to register structure

	mov	ax,[ebx].$CS 		;get selector of destination
	mov	word ptr far_rtn+4,ax	;store in indirect address
	mov	eax,[ebx].$EIP		;get offset of destination
	mov	dword ptr far_rtn,eax	;store in indirect address

	mov	ax,[ebx].$SS		;save new SS where we can
	mov	new_ss,ax		;  access it through CS
	mov	eax,[ebx].$ESP		;save new esp where we can
	mov	new_esp,eax		;  access it through CS
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

	mov	eax,esp			;save ESP to reload after returning
	add	eax,4			;  from far call into save_esp
	mov	save_esp,eax
	mov   save_ss,ss		;save SS too
	mov	eax,[ebx].$EBX		;get new EBX into eax
	mov	ds,[ebx].$DS		;get new DS (addressability
					;  of struc is destroyed by this
					;  instruction)
	mov	ebx,eax			;put new EBX into ebx

	pop	eax			;restore EAX

	push  word ptr cs:new_ss	;push new SS and 
	pop   ss			;  pop it into SS & immediately
	mov	esp,cs:new_esp		;  swap current esp and desired esp

;--- Make far call

call_inst:
	call	cs:far_rtn

;--- Update the register structure

	mov   ss,cs:save_ss		;get saved SS
	mov	esp,cs:save_esp		;get saved ESP

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

	mov	ecx,#reg           	;set up addressability of struc
	mov	eax,FALSE		;assume carry was cleared
	test	[ecx].$EFLAGS,EF_CF	;was carry cleared?
	jz	short i0		;yes,branch
	mov	eax,TRUE		;say carry was set
i0:
	pop	ebp			;fix ebp

	ret				;return to caller

save_ss	dw	?		;save areas for SS and ESP
save_esp	dd	?
new_ss		dw	?		;new SS and ESP
new_esp	dd	?

far_call	endp

code	ends
	end
