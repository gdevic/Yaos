@echo off
cd..
del *.zip
pkzip -p -r yaosrc.zip *.*

@echo Insert a disk in drive A:
pause

xcopy yaosrc.zip a:

cd Tools
call MakeBoot.bat
