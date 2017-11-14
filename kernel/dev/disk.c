#include <stdio.h>
#include <bios.h>
#include <i86.h>

union REGS regs;
struct SREGS sregs;

static struct rminfo {
    long EDI;
    long ESI;
    long EBP;
    long reserved_by_system;
    long EBX;
    long EDX;
    long ECX;
    long EAX;
    short flags;
    short ES,DS,FS,GS,IP,CS,SP,SS;
} RMI;


int DosAlloc( int size )
{
    union REGS regs;
    short sel, seg;

    regs.w.ax = 0x100;
    regs.w.bx = size >> 4;
    int386( 0x31, &regs, &regs );
    sel = regs.w.dx;
    seg = regs.w.ax;

    printf("Base selector: %04Xh\n", sel );
    printf("Real mode segment: %04X\n", seg );

    return( seg << 4 );
}


void main()
  {
    struct diskinfo_t di;
    unsigned short status;
    void * pBuf;
    int i;

    pBuf = DosAlloc( 512 );
    if( pBuf!=NULL )
    {
        for( i=0; i<512; i++ )
        {
            printf("%02X", *(char *)((char *)pBuf + i) );
        }

        for( i=0; i<512; i++ )
        {
            *(char *)((char *)pBuf + i)  = 0;
        }

#if 1
        memset(&RMI,0,sizeof(RMI));
        RMI.EAX=0x00000201; /* call service 39h (create directory) ah=0x39 */
        RMI.EBX = 0;
        RMI.ECX = 1;
        RMI.ES  = (int)pBuf >> 4;
        RMI.EDX = 0;
        RMI.DS  = 0;     /* put DOS segment:offset of str in DS:DX */

        /* Use DMPI call 300h to issue the DOS interrupt */
        regs.w.ax = 0x0300;
        regs.h.bl = 0x13;
        regs.h.bh = 0;
        regs.w.cx = 0;
        sregs.es = FP_SEG(&RMI);
        regs.x.edi = FP_OFF(&RMI);
        int386x( 0x31, &regs, &regs, &sregs );
#else
        di.drive = di.head = di.track = 0;
        di.sector = 2;
        di.nsectors = 1;
        di.buffer = pBuf;
        status = _bios_disk( _DISK_READ, &di );
        printf( "Status = 0x%4.4X\n", status );
#endif
        for( i=0; i<512; i++ )
        {
            printf("%02X", *(char *)((char *)pBuf + i) );
        }
    }
    else
        printf("Error alloc\n");
}

