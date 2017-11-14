Notes:
======

* Use BIOS in V86 mode for all BIOS-supported basic functionality:
    = Low-level disk handling
    = Keyboard
    = Display

* Kernel:
    = Written completely in "C"
    = Runs in protected mode
    = Allows multiple 32 bit tasks:
        - Specially compiled - flat models
    
* File system:
    = FAT12 compliant    
    
    
SO FAR:
=======

GDT 0   0000 - Empty descriptor
GDT 1   0008 - Kernel code selector
GDT 2   0010 - Kernel data selector

IDT