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

// Code by Kaapo
// ======================

char Morsecode[128]; // Defining the morse code string
uint8_t Morseindexcount = 0; // Defining the index number for the morse code string

// ======================

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
        //checks the acceleration on the z-axis
        if(az > 0.8f){
            morse = '.';
        }
        //checks the acceleration on the x-axis or y-axis
        else if(ax > 0.8f || ax < -0.8f){
            morse = '-';
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

}

// Code by Kaapo
// ======================

void buttonTask(void *arg){
    (void)arg;

    bool LastButtonstate1 = false; // Define the last button state for button 1
    bool LastButtonstate2 = false; // Define the last button state for button 2

    // Creating for loop to loop infinitely to read button presses
    for(;;){
        bool Button1 = (gpio_get(SW1_PIN) == 0); // Reading button state of Button1 where 0 means button is pressed and 1 means not pressed
        bool Button2 = (gpio_get(SW2_PIN) == 0); // Reading button state of Button2 where 0 means button is pressed and 1 means not pressed


        // If statement to add dot or dash to string Morsecode
        if (Button1 && !LastButtonstate1) {
            Morsecode[Morseindexcount++] = morse; // If button 1 is pressed it adds dot or dash to the string Morsecode and updates the Morseindexcount
            Morsecode[Morseindexcount] = '\0'; // Adds \0 to the end of the string
        }
        LastButtonstate1 = Button1; // Sets the Lastbuttonstate to the state of button 1

        if (Button2 && !LastButtonstate2) {
            Morsecode[Morseindexcount++] = ' '; // If button 2 is pressed it adds a space to the string Morsecode and updates the Morseindexcount
            Morsecode[Morseindexcount] = '\0'; // Adds \0 to the end of the string

            int length = Morseindexcount;

            // if statement made to print the morsecode if the last 3 characters are a space (button 2 pressed 3 times in a row)
            if (length >= 3 &&
                Morsecode[length-1] == ' ' && // Checking the last character is a space
                Morsecode[length-2] == ' ' && // Checking if the second to last character is a space
                Morsecode[length-3] == ' '){  // Checking if the third to last character is a space
                
                printf("%s\n", Morsecode);

                // Clearing Morseindexcount and Morsecode after the string has been printed
                Morseindexcount = 0; 
                Morsecode[0] = '\0';
                }
        } 
        LastButtonstate2 = Button2; // Sets the Lastbuttonstate to the state of button 2
    }
}

// ======================

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

    init_button1();
    init_button2();
    output_buffer = xQueueCreate(32, sizeof(char));

    /*
    Täällä ne pitäs sitte luoda ne taskit xTaskCreate
    */
    xTaskCreate(
        imuTask,
        "IMU TASK",
        1024
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        buttonTask,
        "Button Task",
        2048,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        usbTask,
        "USB Task",
        1024,
        NULL,
        2,                       
        NULL
    );

    vTaskStartScheduler();

}

