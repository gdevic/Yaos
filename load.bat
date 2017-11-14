echo off
if x%smpchd% == x goto clearscr
goto skipcls
:clearscr
cls
set smpinc=..\includes\
:skipcls
echo *
echo * COMPILING LOADER FILES
echo *

386ASM intr.asm -i %smpinc% -nolist 
if errorlevel 1 goto err

386ASM far_call.asm -i %smpinc% -nolist 
if errorlevel 1 goto err

hc386 load.c -profile %smpinc%pl386.pro
if errorlevel 1 goto err
goto ok

:err
echo *
echo * ERROR OCCURRED DURING COMPILATION OF LOADER FILES
echo *
goto end

:ok
echo *
echo * LOADER FILES COMPILED SUCCESSFULLY
echo *

:end
if x%smpchd% == x goto clearvars
goto dontclear
:clearvars
set smpinc=
:dontclear

