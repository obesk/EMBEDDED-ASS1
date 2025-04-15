/* 
 * File:   uart.h
 * Author: mattia
 *
 * Created on April 4, 2025, 2:42 PM
 */

#ifndef UART_H
#define	UART_H

#include <xc.h>

#define BUFF_LEN 10

struct circular_buffer {
    char buff[BUFF_LEN];
    int read;
    int write;
    int new_data;
};

void init_uart();
void print_to_uart(const char * str);

#endif	/* UART_H */


