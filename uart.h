/* 
 * File:   uart.h
 * Author: mattia
 *
 * Created on April 4, 2025, 2:42 PM
 */

#ifndef UART_H
#define	UART_H

#include <xc.h>

#define INPUT_BUFF_LEN 50   //TODO: SET THE CORRECT SIZE
#define OUTPUT_BUFF_LEN 50  //TODO: SET THE CORRECT SIZE

extern int int_ret; 

struct circular_buffer {
    char *buff;
    int read;
    int write;
};

extern struct circular_buffer UART_output_buff; 
extern struct circular_buffer UART_input_buff; 

void init_uart();
void print_to_buff(const char * str);

#endif	/* UART_H */


