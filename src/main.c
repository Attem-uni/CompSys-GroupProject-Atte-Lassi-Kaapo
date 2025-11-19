#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

//Kurssi reposta, ei mitään hajua tarvitaanko
#define DEFAULT_STACK_SIZE 2048
#define CDC_ITF_TX      1

//Nämä on global variables, pitää varmaan lisätä vielä jotaki
QueueHandle_t output_buffer;
//Yllä oleva täältä
//https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/01-xQueueCreate
char morse;


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


void imuTask(void *p){

    float ax, ay, az;

    while(1){
        ICM42670_read_sensor_data (&ax, &ay, &az, NULL, NULL, NULL, NULL);

        if(az > 0.7f){
            morse = '.';
        }
        else if(ax > 0.7f || ax < -0.7f){
            morse = '-';
        }

    }

    vTaskDelay(pdMS_TO_TICKS(1000));
}

void buttonTask(void *p){
    /*
    TBD
    */
}

//Otettu kurssin reposta
static void usbTask(void *arg) {
    (void)arg;
    while (1) {
        tud_task();              // With FreeRTOS wait for events
                                 // Do not add vTaskDelay. 
    }
}

/*
Tähän kait vielä esim. usbOutputTask yms. joka lähettää sen viestin
*/


int main(void) {
    stdio_init_all();
 
    init_hat_sdk();          // put board in a known state (e.g., RGB off) and start default I2C

    init_ICM42670();                        // IMU WHO_AM_I + basic setup
    ICM42670_startAccel(100, 4);            // 100 Hz, ±4 g

    init_button1;
    init_button2;
    output_buffer = xQueueCreate(32, sizeof(char));

    /*
    Täällä ne pitäs sitte luoda ne taskit xTaskCreate
    */

}

