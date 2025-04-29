/* 
 * File:   uart.h
 * Author: mattia
 *
 * Created on April 4, 2025, 2:42 PM
 */

#ifndef UART_H
#define	UART_H

#include <xc.h>

// Since we use a 10 bit UART transmission we use 10 bits for a byte of data. 
// With a 100Hz main we have 9,6 bytes per cycle
#define INPUT_BUFF_LEN 10

// considerations on MAG message:
// for the x and y axis we have 2^13 bytes signed, which is equivalent to range:
// 4095 : -4096  -> 5 bytes max
// for the z axis we have 2^15 bytes signed, which is equivalent to range:
// 16383 : -16384 -> 6 bytes max

// considreations on YAW message:
// the yaw is in degrees so we have a range of 180 : -180 -> 4 bytes

// considerations on ERR from rate message:
// with 10 bytes we can have a max of 2 RATE messages received
// each ERR message is 7 bytes so 14 bytes max

// considering the worst can in which we print all messages together:
// - $MAG,,,* -> 8 bytes
// - x, y axis -> 10 bytes max
// - z axis -> 6 bytes max
// - $YAW,* -> 6 bytes
// - angle -> 4 bytes
// - err message -> 14 bytes
// total: 48 bytes

#define OUTPUT_BUFF_LEN 48

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


