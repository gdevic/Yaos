;************************************************************************/
;*	Copyright (C) 1986-1989 Phar Lap Software, Inc.			*/
;*	Unpublished - rights reserved under the Copyright Laws of the	*/
;*	United States.  Use, duplication, or disclosure by the 		*/
;*	Government is subject to restrictions as set forth in 		*/
;*	subparagraph (c)(1)(ii) of the Rights in Technical Data and 	*/
;*	Computer Software clause at 252.227-7013.			*/
;*	Phar Lap Software, Inc., 60 Aberdeen Ave., Cambridge, MA 02138	*/
;************************************************************************/
;
; HELLO.ASM - Hello world program for 386 protected mode
;
; This program is the [in]famous "Hello world" program.  It illustrates
; making MS-DOS system calls from 386 protected mode.
;
; This version of the program is provided to demonstrate how to terminate
; a program so that control will be returned to the loader. If the standard
; Dos function 4ch is used, the loaded program will terminate but control
; will not be returned to the loader, instead the parent program will 
; terminate along with the child. One solution to this problem is to 
; terminate the child with a "far" return if it is a .EXP or a "near" 
; return if it is a .REX.  
;

	assume	cs:_text,ds:_data

_text	segment	para public use32 'code'

	public	_start_

_start_	proc	far	; This will be linked as a .EXP file, so
				;    we will define this as a far procedure
				;    so that the 'ret' instruction below
				;    will generate a far return.

	mov	ah,09h			; Output the message.
	mov	edx,offset hellomsg	;
	int	21h			;

	ret				; far return

_start_	endp

_text	ends

_data	segment	para public use32 'data'

hellomsg db	'Hello world!!!!!!!!',0DH,0AH,'$'
	
_data	ends

_stack	segment byte stack use32 'stack'

	db	8192 dup (?)

_stack	ends

	end _start_

