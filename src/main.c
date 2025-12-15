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
QueueHandle_t output_buffer, morse_buffer;
//Yllä oleva täältä
//https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/01-xQueueCreate


// Code by Lassi
void imuTask(void *p){

    float ax, ay, az, gx, gy, gz, t;
    char morse = 0;

    if (init_ICM42670() == 0) {
        printf("ICM-42670P initialized successfully!\n");
        if (ICM42670_start_with_default_values() != 0){
            printf("ICM-42670P could not initialize accelerometer or gyroscope");
        }
    }

    while(1)
        {
            if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0){
                if(az >= 0.8f || az <= -0.8f){
                    morse = '.';
                }
                else{
                    morse = '-';
                }
                xQueueSend(morse_buffer, &morse, 0);   
    
            printf("gyro z-axis on %f\n", az);

            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
}

// Code by Kaapo, modifications by Atte
// ======================

void buttonTask(void *arg){
    (void)arg;

    bool LastButtonstate1 = false; // Define the last button state for button 1
    bool LastButtonstate2 = false; // Define the last button state for button 2

    int btn2Count = 0; // counter for consecutive times button 2 has been pressed
    char morseReceived = 0;

    // Creating for loop to loop infinitely to read button presses
    for(;;){
        bool Button1 = (gpio_get(SW1_PIN) == 0); // Reading button state of Button1 where 0 means button is pressed and 1 means not pressed
        bool Button2 = (gpio_get(SW2_PIN) == 0); // Reading button state of Button2 where 0 means button is pressed and 1 means not pressed


        // If statement to add dot or dash to string Morsecode
        if (Button1 && !LastButtonstate1) {
            xQueueReceive(morse_buffer, &morseReceived, portMAX_DELAY);
            xQueueSend(output_buffer, &morseReceived, 0);   // sends morse to output buffer upon button press
            btn2Count = 0;                          // press of button 1 sets count to 0
            vTaskDelay(pdMS_TO_TICKS(100));          // small 20ms delay to avoid button registering too many time
            printf("toimii btn1");
        }

        if (Button2 && !LastButtonstate2) {
            char space = ' ';
            xQueueSend(output_buffer, &space, 0);   // sends a space to output buffer upon button press
            btn2Count++;                            // press of button 2 adds to counter
            vTaskDelay(pdMS_TO_TICKS(100));          // small 20ms delay to avoid button registering too many times
            printf("toimii btn2");

            // if statement made to print the morsecode if the last 3 characters are a space (button 2 pressed 3 times in a row)
            if (btn2Count >= 3){  // Checking if button 2 has been pressed 3 times in a row
                char end = '\n';
                xQueueSend(output_buffer, &end, 0); // send newline to output buffer to indicate end of current string
                btn2Count = 0;                      // reset button 2 count
                printf("toimii btn2 3krt");
            } 
        }
        LastButtonstate1 = Button1; // Sets the Lastbuttonstate to the state of button 1
        LastButtonstate2 = Button2; // Sets the Lastbuttonstate to the state of button 2

        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

// ======================
//Otettu kurssin reposta
//static void usbTask(void *arg) {
//    (void)arg;
//    while (1) {
//        tud_task();              // With FreeRTOS wait for events
                                 // Do not add vTaskDelay. 
//    }
//}
// usbOutputTask Code by Atte

void usbOutputTask(void *arg) {
    (void)arg;

    while (1) {

        char c;
        xQueueReceive(output_buffer, &c, portMAX_DELAY);    // recieve character from buttontask

        if (c == '\n') {        // ends string if newline detected as char c

            printf("%s\n", Morsecode);              // prints finished morse string on a new line
            Morseindexcount = 0;                    // reset index count for next message
            Morsecode[0] = '\0';                    // set first next value in string to '\0' to reset it
            printf("jee");
        }
        else {
            if (Morseindexcount < sizeof(Morsecode) - 1) {  // avoid adding to string if buffer overflow possible

                Morsecode[Morseindexcount++] = c;           // adds c to morsecode string
                Morsecode[Morseindexcount] = '\0';          // sets next value in string preemptively to '\0' to avoid issues
            }
        }
    }
}

int main(void) {
    stdio_init_all();
 
    init_hat_sdk();          // put board in a known state (e.g., RGB off) and start default I2C

    init_ICM42670();                        // IMU WHO_AM_I + basic setup
    //ICM42670_startAccel(100, 4);            // 100 Hz, ±4 g
    //ICM42670_startGyro(100, 250);

    init_button1();
    init_button2();
    morse_buffer = xQueueCreate(1, sizeof(char));
    output_buffer = xQueueCreate(32, sizeof(char));
    

    TaskHandle_t hIMUTask = NULL;

    xTaskCreate(
        imuTask,
        "IMU TASK",
        1024,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        buttonTask,
        "Button Task",
        2048,
        NULL,
        1,
        &hIMUTask
    );

    //xTaskCreate(
    //    usbTask,
    //    "USB Task",
    //    1024,
    //    NULL,
    //    2,                       
    //    NULL
    //);
    xTaskCreate(
        usbOutputTask,
        "USB Output Task",
        1024,
        NULL,
        2,                       
        NULL
    );

    vTaskStartScheduler();

}

