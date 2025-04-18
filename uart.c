#include "uart.h"
#include <xc.h>


struct circular_buffer UART_input_buff = {0};
struct circular_buffer UART_output_buff = {0};

int int_ret = 1; 

void init_uart() {
    RPINR18bits.U1RXR = 0b1001011; // mapping pin RD11(RPI75) to UART RX
    RPOR0bits.RP64R = 0b000001; // mapping pin RD0(RP64) to UART TX

    U1BRG = 467; // baud rate to 9600 -> 72 000 000 / (16 * 9600) - 1

    U1STAbits.URXISEL = 0; // set to interrupt on char received
    U1MODEbits.UARTEN = 1; // enable UART
    U1STAbits.UTXEN = 1; // enable UART transmission
    
    U1STAbits.UTXISEL0 = 0;
    U1STAbits.UTXISEL1 = 1;

    IFS0bits.U1RXIF = 0; // interrupt flag set to 0
    IEC0bits.U1RXIE = 1; // enabled interrupt on receive
    IEC0bits.U1TXIE = 1; // enabled interrupt on transmission

}

//TODO: make function more gneral with buff as input
void print_to_buff(const char * str) {
    if(!str) {
        return; 
    }
    
    for (int i = 0; str[i] != '\0'; ++i) {
        const int new_write_index = (UART_output_buff.write + 1) % INPUT_BUFF_LEN;

        if(new_write_index == UART_output_buff.read) {
            break;
        }

        UART_output_buff.buff[UART_output_buff.write] = str[i];
        UART_output_buff.write = new_write_index;
    }

    if(int_ret){
        IFS0bits.U1TXIF = 1;
    }
}