rem Compile sample main program
hc386 sample.c -profile ..\includes\pl386.pro
if errorlevel 1 goto err

rem Build loader modules
hc386 load.c -profile ..\includes\pl386.pro
if errorlevel 1 goto err
386ASM intr.asm -i ..\includes\ -nolist 
if errorlevel 1 goto err
386ASM far_call.asm -i ..\includes\ -nolist 
if errorlevel 1 goto err

rem Link the sample program
386link sample load intr far_call -lib c:\hc386\hce.lib -maxdata 4000h -nomap
if errorlevel 1 goto err
goto end

:err
echo *
echo * ERROR OCCURRED DURING BUILD OF SAMPLE FILE
echo *

:end
