f()
{
    volatile int i=10;
}


int main()
{
    char *ptr;
    int i;

    ptr = (char*)(0xB8000 - 0x30020);

    do
    {
        for( i=0; i<40*80; i+=2 )
        {
            *(ptr+i+0) = i;
            *(ptr+i+1) = 7;
        }

    } while(1);
    
    f();
    return( 0 );
}

    