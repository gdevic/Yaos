echo. > kernel.txt
echo                         ========>> kernel.txt
echo                          KERNEL >> kernel.txt
echo                         ========>> kernel.txt
info -f ..\kernel\source\*.* -l- >> kernel.txt

echo. >> kernel.txt
echo                     ================>> kernel.txt
echo                      DEVICE DRIVERS >> kernel.txt
echo                     ================>> kernel.txt
info -f ..\kernel\source\dev\*.* -l- >> kernel.txt

echo. >> kernel.txt
echo                     ===============>> kernel.txt
echo                      INCLUDE FILES >> kernel.txt
echo                     ===============>> kernel.txt
info -f ..\kernel\include\*.* -l- >> kernel.txt

echo. >> kernel.txt
echo                  =====================>> kernel.txt
echo                   INTEGRATED DEBUGGER >> kernel.txt
echo                  =====================>> kernel.txt
info -f ..\kernel\source\debugger\*.* -l- >> kernel.txt

