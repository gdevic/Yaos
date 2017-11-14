lib=c:\hc386\hce.lib
incl=..\includes

.asm.obj:
	386ASM $* -I $(incl)\ -NOLIST 

.c.obj:
	hc386 $* -profile $(incl)\pl386.pro 

load.obj: $*.c $(incl)\$*.h

intr.obj: $*.asm

far_call.obj: $*.asm

