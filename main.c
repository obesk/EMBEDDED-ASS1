/*
 * File:   main.c
 * Author: obe
 *
 * Created on March 27, 2025, 9:09 PM
 */


#include "timer.h"
#include "uart.h"
#include "spi.h"

#include "xc.h"
#include "string.h"

#define CLOCK_LD_TOGGLE 50
#define CLOCK_ACQUIRE_MAG 4

void algorithm() {
    tmr_wait_ms(TIMER2, 7);
}

void activate_magnetometer() {
    //selecting the magnetometer
    CS_ACC = 1;
    CS_GYR = 1;
    
    CS_MAG = 0;
    spi_write(0x4B);
    spi_write(0x01); // changing the magnetometer to sleep state
    CS_MAG = 1;

    tmr_wait_ms(TIMER1, 3); //waiting for the magnetometer to go into sleep state
    
    CS_MAG = 0;
    spi_write(0x4C);
    spi_write(0x00); // changing the magnetometer to active state  
    CS_MAG = 1;

    tmr_wait_ms(TIMER1, 3); //waiting for the magnetometer to go into active state
}

int read_mag_x_axis() {
    //TODO: decide what to do with this overflow
    if(SPI1STATbits.SPIROV){
        SPI1STATbits.SPIROV = 0;
    }
    
    CS_MAG = 0;
    spi_write(0x42 | 0x80);
    const int mag_x_axis = (spi_write(0x00) & 0x00F8) | (spi_write(0x00) << 8);
    CS_MAG = 1;

    return mag_x_axis >> 3;
}

int main(void) {
    init_uart();
    init_spi(); //TODO: choose the right values for the SPI CLOCK

    char input_buff[INPUT_BUFF_LEN];
    char output_buff[OUTPUT_BUFF_LEN];
    UART_input_buff.buff = input_buff;
    UART_output_buff.buff = output_buff;

    TRISA = TRISG = ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;

    activate_magnetometer();

    char output_str [50]; //TODO: size correctly

    int LD2_toggle_counter = 0;
    int acquire_mag_counter = 0;

    tmr_setup_period(TIMER1, 10);

    while (1) {
        algorithm();
 
        if (++LD2_toggle_counter >= CLOCK_LD_TOGGLE) {
            LD2_toggle_counter = 0;
            LATGbits.LATG9 = !LATGbits.LATG9;
        }

        if (++acquire_mag_counter >= CLOCK_LD_TOGGLE) {
            acquire_mag_counter = 0;
            const int x_axis = read_mag_x_axis();

            sprintf(output_str, "$MAG,%d,y,z*", x_axis);
            print_to_buff(output_str);
        }

        tmr_wait_period(TIMER1);
    }
       
    return 0;
}

void __attribute__((__interrupt__)) _U1TXInterrupt(void){
    IFS0bits.U1TXIF = 0; // clear TX interrupt flag
    if(UART_output_buff.read == UART_output_buff.write){
        int_ret = 1;
    }

    while(!U1STAbits.UTXBF && UART_output_buff.read != UART_output_buff.write){
        U1TXREG = UART_output_buff.buff[UART_output_buff.read];
        UART_output_buff.read = (UART_output_buff.read + 1) % OUTPUT_BUFF_LEN;
    }
}

       // if(SPI1STATbits.SPIROV){
        //     SPI1STATbits.SPIROV = 0;
        //     sprintf(output_buff, "SPIROV");
        //     print_to_uart(output_buff);
        // } else {
        //     CS_MAG = 0;
        //     spi_write(0x42 | 0x80);
        //     unsigned int mag_x_axis = (spi_write(0x00) & 0x00F8) | (spi_write(0x00) << 8);
        //     CS_MAG = 1;

        //     sprintf(output_buff, "$MAGX=%d*", mag_x_axis);
        //     print_to_uart(output_buff);
        //     tmr_wait_ms(TIMER1, 100);
        // }