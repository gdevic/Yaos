volatile char str[100] = "***********";

void f();

int _cstart()
{
        volatile int i;


        str[0] = 'a';
        str[99] = 'z';
        for( i=0; i<100; i++ ) 
        {
            str[i] = i;   
            f();
        }

        return 0;
}

void f()
{
    volatile int i;
    
    i ++;
}
    