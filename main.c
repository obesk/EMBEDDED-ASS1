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

int main(void) {
    init_uart();
    init_spi();
    
    char output_buff[10]; //TODO: change size

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

    tmr_wait_ms(TIMER1, 3); //waiting for the magnetometer to go into sleep state

    //READ THE CHIP ID OF THE IMU

    // CS_MAG = 0;
    // unsigned int addr = spi_write(0x40 | 0x80);
    // addr= spi_write(0x00);
    // CS_MAG = 1;

    // sprintf(output_buff, "ADDR: %d", addr);
    // print_to_uart(output_buff);

    while (1) {

        if(SPI1STATbits.SPIROV){

            SPI1STATbits.SPIROV = 0;
            sprintf(output_buff, "SPIROV");
            print_to_uart(output_buff);
        } else {

            CS_MAG = 0;
            spi_write(0x42 | 0x80);
            unsigned int mag_x_axis = (spi_write(0x00) & 0x00F8) | (spi_write(0x00) << 8);
            CS_MAG = 1;

            sprintf(output_buff, "$MAGX=%d*", mag_x_axis);
            print_to_uart(output_buff);
            tmr_wait_ms(TIMER1, 100);
        }
    }
       
    return 0;
}
