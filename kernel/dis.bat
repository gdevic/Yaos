@echo off
if "%1"=="" goto Help
if exist Source\%1.c goto Dis_C


:Dis_ASM
wdisasm -l -s=Source\%1.asm -b Objects\%1.obj
goto End


:Dis_C

wdisasm -l -s=Source\%1.c -b Objects\%1.obj
goto End


:Help

echo Disassembles object file whose source is specified.
echo.
echo Usage: dis [source_file]
echo.
echo    Ex: dis kernel

:End

l %1.lst
