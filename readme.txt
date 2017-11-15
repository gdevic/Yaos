YAOS - Yet Another Operating System
===================================
(c) Copyright by Goran Devic, 1997

Specifications
==============

* Preemptive multitasking
* Shared memory with copy-on-write
* Simultaneous V86-mode DOS sessions with hooks for DPMI support
* Configurable hot-keys to change among process' terminals
* Boot from DOS prompt and exit back into DOS prompt
* Built in device drivers for DOS file system and Terminal
* Integrated debugger:
  - Full debugging support
  - Internal variables
  - Breakpoints
  - Full expression evaluator
  - Command line interface with history
  - Support for Watcom map files to map addresses to names
  - Single step or trace execution
* Accompanied POSIX compliant C-library

* List of UNIX programs executing: Banner, Cal

Getting started
===============
A bootable floppy image "yaos.img" is included. You can use a VMWare VM to run it
or mount it within DosBOX: imgmount y YAOS.IMG -t floppy
or boot using that image:  boot YAOS.IMG
VMWare is a better method since DosBOX will cause Yaos to fault into debugger.

1.  Run START.BAT to start kernel.  It will load `init' process and break in the
    built-in debugger (*1).

2.  Press F5 (debugger assigned key for `go').  You will see the prompt of one
    of the two Virtual Machines (*2).

3.  Press SHIFT-F1 to activate System Virtal Machine in which all PM processes
    are executing (*3)

---
(*1) You can always break into the debugger by pressing the "SCROLL LOCK" key.
     Press F1 for the debugger help.
(*2) SHIFT-F1 is the System Virtual Machine
     SHIFT-F2 is the first user VM
     SHIFT-F3 is the second user VM (only two extra are created by init.c)
(*3) Blinking 'O'/'X' in the top-right corner is the scheduler.  You will see
     a prompt 'HOME>'.  Try these commands:
       `banner HELLO'
       `cal 1997'

SOURCE CODE
===========
Yes, full source code is included: loader, kernel, device drivers, debugger,
Posix-compliant C library and some test and util programs.

TOOLS NEEDED:
* Watcom C 10.6
* Masm 6.11

Other versions of compiler and assembler may work as well - didn't test.
-----

In the Tools directory there is a program `Info' and a batch file 'Collect'
that will collect some information from the source code.

-----

`Banner' and `Cal' work well, there's also a Minix' shell (sh?.c) and an `ls'
command that do not work... they need more library calls that for now only
have empty stubs.

-----
Need to work on:

Kernel:
* Signals
* Directories
* Device drivers
* Other file system support
* Network stack

Applications:
* Shell
* UNIX commands (bin/sbin)
* ...
DOS machines:
* Full DPMI support
* Full virtualization

Known Problems
==============
* Some Pentium Pro AGP machines reboot during the loading of the kernel.


Disclaimer of Warranty
======================
By using this software you accept this disclaimer of warranty: "This software is supplied AS IS.  Goran Devic disclaims all warranties, expressed or implied, including, without limitation, the warranties of merchantability and of fitness for a particular purpose.  Goran Devic assumes no liability for damages, direct or consequential, which may result from the use of this software."

GNU Public Domain Software
==========================
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public Licence as published by the Free Software Foundation; either version 2 of the Licence or any later version.  See the GNU General Public Licence for more details.
