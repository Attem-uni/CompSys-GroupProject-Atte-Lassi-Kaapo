#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "tusb.h"

//Every members contribution was equal, so we want to share the points equally.

// Code by Kaapo and Lassi
// ======================

char Morsecode[128]; // Defining the morse code string
uint8_t Morseindexcount = 0; // Defining the index number for the morse code string
QueueHandle_t output_buffer;

// ======================



// Code by Lassi and modifications by Kaapo
static void imuTask(void *p) {
    float ax, ay, az, gx, gy, gz, t;

    if (init_ICM42670() == 0) {
        printf("ICM-42670P initialized successfully!\n");
        if (ICM42670_start_with_default_values() != 0) {
            printf("ICM-42670P could not initialize accelerometer or gyroscope\n");
        }
    }

    while (1) {
        ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t);
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
    float ax, ay, az, gx, gy, gz, t;

    // Creating for loop to loop infinitely to read button presses
    for(;;){
        bool Button1 = (gpio_get(SW1_PIN) == 0); // Reading button state of Button1 where 0 means button is pressed and 1 means not pressed
        bool Button2 = (gpio_get(SW2_PIN) == 0); // Reading button state of Button2 where 0 means button is pressed and 1 means not pressed


        // If statement to add dot or dash to string Morsecode
        if (Button1 && !LastButtonstate1) { // Checks if button1 is pressed
            if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0) { // Reads sensor data
                char morse = (ax >= 0.5) ? '.' : '-'; // Assings either . or - to the morse variable
                printf("Added: %c\n", morse); // Prints to the terminal if . or - has been added

                xQueueSend(output_buffer, &morse, 0);
            }
            btn2Count = 0;
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        // If statement to send space to the output buffer
        if (Button2 && !LastButtonstate2) {
            char space = ' ';
            printf("Added: space\n");
            xQueueSend(output_buffer, &space, 0);   // sends a space to output buffer upon button press
            btn2Count++;                            // press of button 2 adds to counter
            vTaskDelay(pdMS_TO_TICKS(100));          // small 20ms delay to avoid button registering too many times
        

            // if statement made to print the morsecode if the last 3 characters are a space (button 2 pressed 3 times in a row)
            if (btn2Count >= 3){  // Checking if button 2 has been pressed 3 times in a row
                char end = '\n';
                xQueueSend(output_buffer, &end, 0); // send newline to output buffer to indicate end of current string
                btn2Count = 0;                      // reset button 2 count
            } 
        }
        LastButtonstate1 = Button1; // Sets the Lastbuttonstate to the state of button 1
        LastButtonstate2 = Button2; // Sets the Lastbuttonstate to the state of button 2

        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

// ======================
// Added by Lassi
static void usbTask(void *arg) {
    (void)arg;
    while (1) {
        tud_task();              // With FreeRTOS wait for events
                                 // Do not add vTaskDelay. 
    }
}
// usbOutputTask Code by Atte

void usbOutputTask(void *arg) {
    (void)arg;

    while (1) {

        char c;
        xQueueReceive(output_buffer, &c, portMAX_DELAY);    // recieve character from buttontask

        if (c == '\n') {        // ends string if newline detected as char c

            printf("Morsecode is: %s\n", Morsecode);              // prints finished morse string on a new line
            Morseindexcount = 0;                    // reset index count for next message
            Morsecode[0] = '\0';                    // set first next value in string to '\0' to reset it
        }
        else {
            if (Morseindexcount < sizeof(Morsecode) - 1) {  // avoid adding to string if buffer overflow possible

                Morsecode[Morseindexcount++] = c;           // adds c to morsecode string
                Morsecode[Morseindexcount] = '\0';          // sets next value in string preemptively to '\0' to avoid issues
            }
        }
    }
}

// Code by Atte and Lassi
int main(void) {
    stdio_init_all();
 
    init_hat_sdk();          // put board in a known state (e.g., RGB off) and start default I2C

    init_ICM42670();                        // IMU WHO_AM_I + basic setup

    init_button1();
    init_button2();

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

    xTaskCreate(
        usbTask,
        "USB Task",
        1024,
        NULL,
        2,                       
        NULL
    );
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

