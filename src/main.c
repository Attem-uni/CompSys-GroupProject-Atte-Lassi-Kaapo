
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"


//Lisätty

char morse = '.';

void init_sw1() {
    // Initialize the button pin as an input with a pull-up resistor
    gpio_init(SW1_PIN);
    gpio_set_dir(SW1_PIN, GPIO_IN);

}

void init_sw2() {
    // Initialize the button pin as an input with a pull-up resistor
    gpio_init(SW2_PIN);
    gpio_set_dir(SW2_PIN, GPIO_IN);
}

void init_button1(){
    return init_sw1();
}

void init_button2(){
    return init_sw2();
}


void imuTask(){

    float ax, ay, az;

    while{
        ICM42670_read_sensor_data (float &ax, float &ay, float &az, NULL, NULL, NULL, NULL);

        if(az > 0.7f){
            morse = '.';
        }
        else if(ax > 0.7f || ax < -0.7f){
            morse = '-';
        }

    }
}

void buttonTask(){

}

int main(void) {
    stdio_init_all();
 
    init_hat_sdk();          // put board in a known state (e.g., RGB off) and start default I2C

    init_ICM42670();                        // IMU WHO_AM_I + basic setup
    ICM42670_startAccel(100, 4);            // 100 Hz, ±4 g

}

