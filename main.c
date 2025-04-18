/*
 * File:   main.c
 * Author: obe
 *
 * Created on March 27, 2025, 9:09 PM
 */


#include "timer.h"
#include "uart.h"
#include "spi.h"

#include <xc.h>
#include <string.h>
#include <math.h>

#define CLOCK_LD_TOGGLE 50
#define CLOCK_ACQUIRE_MAG 4

#define N_MAG_READINGS 5

struct MagReading {
    int x;
    int y;
    int z;
};

struct MagReadings {
    int w;
    struct MagReading readings[N_MAG_READINGS];
};

enum Axis {
    X_AXIS = 0,
    Y_AXIS,
    Z_AXIS,
};

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

int read_mag_axis(enum Axis axis) {
    //TODO: decide what to do with this overflow
    if(SPI1STATbits.SPIROV){
        SPI1STATbits.SPIROV = 0;
    }
    
    CS_MAG = 0;
    spi_write((0x42 + axis * 2)| 0x80);
    const int axis_value = (spi_write(0x00) & 0x00F8) | (spi_write(0x00) << 8);
    CS_MAG = 1;

    return axis_value >> 3;
}

int main(void) {
    init_uart();
    init_spi(); //TODO: choose the right values for the SPI CLOCK

    struct MagReadings mag_readings = {0};

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

            // TODO: do some readings at the start to ensure that 
            // the average is decent
            mag_readings.readings[mag_readings.w] = (struct MagReading) {
                .x = read_mag_axis(X_AXIS),
                .y = read_mag_axis(Y_AXIS),
                .z = read_mag_axis(Z_AXIS),
            };

            mag_readings.w = (mag_readings.w + 1) % N_MAG_READINGS;

            //TODO: ensure that no overflow can occur
            struct MagReading sum_reading = {0}; 
            for(int i = mag_readings.w ; i != (mag_readings.w + 1) % N_MAG_READINGS; i = (i + 1) % N_MAG_READINGS) {
                sum_reading.x += mag_readings.readings[i].x;
                sum_reading.y += mag_readings.readings[i].y;
                sum_reading.z += mag_readings.readings[i].z;
            }

            const struct MagReading avg_reading = {
                .x = sum_reading.x / N_MAG_READINGS,
                .y = sum_reading.y / N_MAG_READINGS,
                .z = sum_reading.z / N_MAG_READINGS,
            };

            //TODO: print only at specified Hz
            sprintf(output_str, "$MAG,%d,%d,%d*", avg_reading.x, avg_reading.y, avg_reading.z);
            print_to_buff(output_str);

            const int yaw_deg = (int) (180.0 * atan2((float)avg_reading.y, (float)avg_reading.x) / M_PI);
            sprintf(output_str, "$YAW,%d*", yaw_deg); 
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