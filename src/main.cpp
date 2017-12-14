/*
---------------------------------------------
Controlling WS2812B LED strip, Christmas 2017

Author: Peter Javorsky, MSc.

Date: 13.12.2017
---------------------------------------------
*/

#include <Arduino.h>
#include <FastLED.h>

// How many leds are in the strip?
#define NUM_LEDS 50

// Data pin that led data will be written out over
#define DATA_PIN 3

// LEDs array
CRGB leds[NUM_LEDS];

// which effect to display
unsigned char effect = 0;

// maximum frames per second
const uint16_t MAX_FPS = 60;

// No. of effects
const int EFFECTS_COUNT = 1;

void taskLedStrip(void *parameter)
{
    switch (effect)
    {
        case 0:
            // Move a single white led 
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = CRGB::White;
                FastLED.show();
                vTaskDelay(100 / portTICK_RATE_MS);
                //FastLED.delay(100);
                leds[i] = CRGB::Black;
            }

        break;

        default:
        break;
    }
    
    //vTaskDelete(NULL); 
}

void setupWifi() {

}

void handleWifi() {
    
}
 
void taskWifi(void *parameter)
{
    setupWifi();

    while (1) {
        handleWifi();
    }
}

void setup() {
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
   	delay(2000);

    Serial.begin(115200);

    FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
    FastLED.setMaxRefreshRate(MAX_FPS);

    xTaskCreate(
                    taskLedStrip,     /* Task function. */
                    "TaskLedStrip",   /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
    xTaskCreate(
                    taskWifi,         /* Task function. */
                    "TaskWifi",       /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
}

void loop() {
    delay(100);
}
