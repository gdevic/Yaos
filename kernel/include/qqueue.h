/******************************************************************************
*                                                                             *
*   Module:     QQueue.h                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/4/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This header file implements the queue functionality using the
        arrays.  These queue functions are written as macros and are
        faster than a more complex queue module.  These can only deal with
        a single integers being queued.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/4/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _QQUEUE_H_
#define _QQUEUE_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

/******************************************************************************

    This is a preferred way to use these queue macros:

    QQ(q1,10);                  <- Declare queue structure named `q1'

    if( !QQIsFull(q1) )         <- Check that the queue is not full
        QQEnqueue(q1, val1);    <- Put a value in a queue

    if( !QQIsEmpty(q1) )        <- Check that the queue is not empty
        val = QQDequeue(q1);    <- Get a value from a queue

    val = QQPeek(q1);           <- Peek at the next queue value

    The queue element is a single integer.  You can declare multiple different
    queues in the same file/function.

    QQIsEmpty, QQIsFull and QQDequeue are functions returning a value.
    QQEnqueue is a statement and its return value is undefined.

******************************************************************************/

#define QQ(name,elem)               \
struct                              \
{                                   \
    int size;                       \
    int head;                       \
    int tail;                       \
    int q[elem];                    \
} name = { elem, 0, elem-1 }


#define QQIsEmpty(name)             \
(name.head==((name.tail+1==name.size)? 0 : name.tail+1))


#define QQIsFull(name)              \
(name.head==name.tail)


#define QQEnqueue(name,value)       \
name.q[name.head] = value;          \
name.head = (name.head+1==name.size)? 0 : name.head+1


#define QQDequeue(name)             \
name.q[ name.tail=(name.tail+1==name.size)? 0:name.tail+1 ]


#define QQPeek(name)                \
name.q[ (name.tail+1==name.size)? 0:name.tail+1 ]


#endif //  _QQUEUE_H_
