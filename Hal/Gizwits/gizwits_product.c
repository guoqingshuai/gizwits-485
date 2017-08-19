#include <stdio.h>
#include <string.h>
#include "Hal_Usart/hal_uart.h"
#include "gizwits_protocol.h"


/**
* @brief 向环形缓冲区写入数据
* @param [in] buf        : buf地址
* @param [in] len        : 字节长度
* @return   正确 : 返回写入的数据长度
            失败 : -1
*/
int32_t gizPutData(uint8_t *buf, uint32_t len)
{
    int32_t count = 0;

    if(NULL == buf)
    {
        GIZWITS_LOG("ERROR: gizPutData buf is empty \n");
        return -1;
    }

    count = rbWrite(&pRb, buf, len);
    if(count != len)
    {
        GIZWITS_LOG("ERROR: Failed to rbWrite \n");
        return -1;
    }

    return count;
}

static int32_t rbWrite(rb_t *rb, const void *data, size_t count)
{
    int tailAvailSz = 0;

    if(NULL == rb)
    {
        GIZWITS_LOG("ERROR: rb is empty \n");
        return -1;
    }

    if(NULL == data)
    {
        GIZWITS_LOG("ERROR: data is empty \n");
        return -1;
    }

    if (count >= rbCanWrite(rb))
    {
        GIZWITS_LOG("ERROR: no memory %d \n", rbCanWrite(rb));
        return -1;
    }

    if (rb->rbHead <= rb->rbTail)
    {
        tailAvailSz = rbCapacity(rb) - (rb->rbTail - rb->rbBuff);
        if (count <= tailAvailSz)
        {
            memcpy(rb->rbTail, data, count);
            rb->rbTail += count;
            if (rb->rbTail == rb->rbBuff+rbCapacity(rb))
            {
                rb->rbTail = rb->rbBuff;
            }
            return count;
        }
        else
        {
            memcpy(rb->rbTail, data, tailAvailSz);
            rb->rbTail = rb->rbBuff;

            return tailAvailSz + rbWrite(rb, (char*)data+tailAvailSz, count-tailAvailSz);
        }
    }
    else
    {
        memcpy(rb->rbTail, data, count);
        rb->rbTail += count;
        return count;
    }
}

void UART_IRQ_FUN(void)
{
    uint8_t value = 0;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
        value = USART_ReceiveData(USART2);

        gizPutData(&value, 1);
    }
}
void uartInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    UART_GPIO_Cmd(UART_GPIO_CLK, ENABLE);
    UART_CLK_Cmd(UART_CLK, ENABLE);
    UART_AFIO_Cmd(UART_AFIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = UART_TxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(UART_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = UART_RxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART_GPIO_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = UART_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART, &USART_InitStructure);

    USART_ITConfig(UART,USART_IT_RXNE,ENABLE);
    USART_Cmd(UART, ENABLE);
    USART_ClearFlag(UART, USART_FLAG_TC); /*清空发送完成标志,Transmission Complete flag */

    /*使能串口中断,并设置优先级*/
    NVIC_InitStructure.NVIC_IRQChannel = UART_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
