rem
rem  This file compiles a version of HELLO.ASM that is suitable to
rem  be loader with the subroutines in file LOAD.C

rem  The line below assembles the file HELLO.ASM

386asm hello -nolist
if errorlevel 1 goto err

rem  The line below links HELLO.OBJ into an .EXP file

386link hello -exe hello.exp -nomap
if errorlevel 1 goto err

rem 
rem FILES COMPILED SUCCESSFULLY
rem

goto end

:err
rem
rem *** ERROR : FILE DID NOT COMPILE PROPERLY
rem
:end

