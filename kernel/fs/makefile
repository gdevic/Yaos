###############################################################################
#                                                                             #
#            makefile for YAOS - Yet Another Operating System                 #
#                                                                             #
#                        (c) 1996 Goran Devic                                 #
#                                                                             #
#                 FS - File Subsystem Makefile for YAOS                       #
#                                                                             #
# Use: wmake | wmake clean | wmake all                                        #
#                                                                             #
# Symbolic commands                                                           #
#                                                                             #
#  wmake clean    - cleans all the objects (forces complete                   #
#                   compilation)                                              #
#                                                                             #
#  wmake all      - cleans all the objects                                    #
#                   and then makes all anew                                   #
#                                                                             #
###############################################################################

SRC              = .
INC              = ..\Include\
OBJ              = .

OBJECT_FILES     = ufs.obj,memfs.obj,dosfs.obj,cache.obj,fdd.obj

###############################################################################
#                                                                             #
# Compiler directives                                                         #
#                                                                             #
###############################################################################

# WCC_OPTIONS:
#  ~ Generate 486 optimum code with stack based params passing (-4s)
#  ~ Do not check stack overflow (-s)
#  ~ Warning level 2 (-W2)
#  ~ Include debug data (-d2)
#  ~ Do not optimize (-od)
#  ~ Use Intel fp unit
#

WCC_OPTIONS      = -4s -W2 -d3
#WCC_OPTIONS      = -4s -s -W3 -od -d3+ -fpi87
WLINK_OPTIONS    = option map debug all
WCC_ROUTE        = 

###############################################################################
#                                                                             #
# List of dependencies                                                        #
#                                                                             #
###############################################################################

fs.exe              :   ufs.obj     &
                        memfs.obj   &
                        dosfs.obj   &
                        cache.obj   &
                        fdd.obj

                    echo Name fs.exe > $^*.lnk
                    echo File >> $^*.lnk
                    echo $(OBJECT_FILES) >> $^*.lnk

                    wlink $(WLINK_OPTIONS) @$^*
                    @del $^*.lnk

###############################################################################

ufs.obj             :   ufs.c                   &

                    wcc386 $(WCC_ROUTE) ufs $(WCC_OPTIONS)

memfs.obj           :   memfs.c                 &

                    wcc386 $(WCC_ROUTE) memfs $(WCC_OPTIONS)

dosfs.obj           :   dosfs.c                 &

                    wcc386 $(WCC_ROUTE) dosfs $(WCC_OPTIONS)

cache.obj           :   cache.c                 &

                    wcc386 $(WCC_ROUTE) cache $(WCC_OPTIONS)

fdd.obj             :   fdd.c                   &

                    wcc386 $(WCC_ROUTE) fdd $(WCC_OPTIONS)



###############################################################################
#                                                                             #
# Symbolic commands                                                           #
#                                                                             #
#  wmake clean    - cleans all the objects (forces complete                   #
#                   compilation)                                              #
#                                                                             #
#  wmake all      - cleans all the objects                                    #
#                   and then makes all anew                                   #
#                                                                             #
###############################################################################

clean               :   .SYMBOLIC
                    @del *.obj
                    @del *.lst
                    @del *.exe
                    @del *.err
                    @del *.map

all                 :   .SYMBOLIC
                    del  fs.exe
                    wmake clean
                    wmake

