#ifndef _GIZWITS_PROTOCOL_H
#define _GIZWITS_PROTOCOL_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gizwits_product.h"

#define typedef_t __packed typedef
	/** ????????? */
typedef_t struct {
    size_t rbCapacity;
    uint8_t  *rbHead;
    uint8_t  *rbTail;
    uint8_t  *rbBuff;
}rb_t;
	
rb_t pRb;   
static int32_t rbCanWrite(rb_t *rb)
{
    if(NULL == rb)
    {
        GIZWITS_LOG("ERROR: input rb is NULL\n");
        return -1;
    }

    return rbCapacity(rb) - rbCanRead(rb);
}

void uartInit(void);
static int32_t rbWrite(rb_t *rb, const void *data, size_t count);

#endif
