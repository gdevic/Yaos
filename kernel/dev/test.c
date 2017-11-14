#include <stdio.h>
#include <bios.h>


char buffer[512];


main()
{
    struct diskinfo_t di;
    unsigned short status;
    int nRetry = 4;
    int i;

    for( i=0; i<512; i++ )
       printf(" %02X ", buffer[i] );

    do
    {
        di.drive = 0;
        di.head = 0;
        di.track = 1;
        di.sector = 1;
        di.nsectors = 1;
        di.buffer = &buffer;
        status = _bios_disk( _DISK_READ, &di );
        printf( "Status = 0x%4.4X\n", status );
        _bios_disk( _DISK_RESET, &di );
        delay(100);

    } while( (nRetry-->0) && (status!=di.nsectors) );


    for( i=0; i<512; i++ )
       printf(" %02X ", buffer[i] );
}
