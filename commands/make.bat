@set include=\yaos\lib\h
@set lib=\yaos\lib

@if %1==clean goto clean
del *.exp
del *.obj
rem --------------------------------------------------------------------

#wcc386 -4s -s -W2 -d2 -zp4 -zl Init.c
#wlink file Init.obj lib cstart.lib lib clib.lib form phar op map

wcc386 -4s -s -W2 -d2 -zp4 -zl banner.c
wlink file banner.obj lib cstart.lib lib clib.lib form phar op map

wcc386 -4s -s -W2 -d2 -zp4 -zl cal.c
wlink file cal.obj lib cstart.lib lib clib.lib form phar op map

wcc386 -4s -s -W2 -d2 -zp4 -zl test.c
wlink file test.obj lib cstart.lib lib clib.lib form phar op map

#wcc386 -4s -s -W2 -d2 -zp4 -zl sh1.c
#wcc386 -4s -s -W2 -d2 -zp4 -zl sh2.c
#wcc386 -4s -s -W2 -d2 -zp4 -zl sh3.c
#wcc386 -4s -s -W2 -d2 -zp4 -zl sh4.c
#wcc386 -4s -s -W2 -d2 -zp4 -zl sh5.c
#wcc386 -4s -s -W2 -d2 -zp4 -zl sh6.c
#wlink file sh1.obj sh2.obj sh3.obj sh4.obj sh5.obj sh6.obj lib cstart.lib lib clib.lib form phar op map

wcc386 -4s -s -W2 -d2 -zp4 -zl ls.c
wlink file ls.obj lib cstart.lib lib clib.lib form phar op map

#wcc386 -4s -s -W2 -d2 -zp4 -zl test2.c
#wlink file test2.obj lib cstart.lib lib clib.lib form phar op map

rem --------------------------------------------------------------------
dir *.exp
@goto end

:clean

del *.obj
del *.map
del *.err

:end

