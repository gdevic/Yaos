/*
 * stdlib.h
 *
 * YAOS (c) Goran Devic
 *
 */

#ifndef _STDLIB_
#define _STDLIB_


#include "types.h"


#ifndef _SIZE_T_DEF_
#define _SIZE_T_DEF_
typedef unsigned size_t;
#endif

#ifndef _NULL_DEF_
#define _NULL_DEF_
#define NULL           (long)0
#endif


//extern void *kmalloc( size_t );
//extern void  kfree( void * );

/* Supporting functions */
//extern void init_kmem( BYTE*, DWORD );

#endif
