#include "uart.h"
#include <xc.h>

void init_uart() {
    RPINR18bits.U1RXR = 0b1001011; // mapping pin RD11(RPI75) to UART RX
    RPOR0bits.RP64R = 0b000001; // mapping pin RD0(RP64) to UART TX

    U1BRG = 467; // baud rate to 9600 -> 72 000 000 / (16 * 9600) - 1

    U1STAbits.URXISEL = 0; // set to interrupt on char received
    U1MODEbits.UARTEN = 1; // enable UART
    U1STAbits.UTXEN = 1; // enable UART transmission
    
    IFS0bits.U1RXIF = 0; // interrupt flag set to 0
    IEC0bits.U1RXIE = 1; // enabled interrupt on receive
}

void print_to_uart(const char * str) {
    if(!str) {
        return; 
    }
    
    for (int i = 0; str[i] != '\0'; ++i) {
        while(U1STAbits.UTXBF);
        U1TXREG = str[i];
    }
}