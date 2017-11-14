if %1==clean goto clean
        wcl     /fo=..\Objects\ ..\Source\writeb.c
        wcl     /fo=..\Objects\ ..\Source\bootsect.asm /fe=BootSect.
        wcl386  /fo=..\Objects\ ..\Source\checksum.c
        goto end
:clean
        del ..\Objects\*.obj
        del ..\Objects\*.lst
        del *.err
        del *.exe
        del bootsect.
:end
