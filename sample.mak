obj=sample.obj load.obj intr.obj far_call.obj 
lib=c:\hc386\hce.lib
incl=..\includes

.c.obj:
	hc386 $* -profile $(incl)\pl386.pro

sample.obj: $*.c 

load.obj: $*.c $(incl)\$*.h intr.asm far_call.asm
	make load.mak

sample.exp: $(obj) $(lib)
	386link $(obj) -exe $* -lib $(lib) -sym -maxdata 4000h -nomap


