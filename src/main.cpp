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
#define DATA_PIN 12

// builtin LED
#define LED_BUILTIN 5

// LEDs array
CRGB leds[NUM_LEDS];

// which effect to display
unsigned char effect = 1;

// maximum frames per second
const uint16_t MAX_FPS = 60;

// No. of effects
const int EFFECTS_COUNT = 1;

// effects variables
uint8_t max_bright = 128;
uint8_t thisdelay = 40;
uint8_t thishue = 0;
int8_t thisrot = 1;
uint8_t deltahue = 1;
bool thisdir = 0;
bool isChangemeSet = false;

void rainbow_march() {                                        // The fill_rainbow call doesn't support brightness levels. You would need to change the max_bright value.
  if (thisdir == 0) thishue += thisrot; else thishue-= thisrot;  // I could use signed math, but 'thisdir' works with other routines.
  fill_rainbow(leds, NUM_LEDS, thishue, deltahue);            // I don't change deltahue on the fly as it's too fast near the end of the strip.
}

void taskLedStrip(void *parameter)
{
  while (1) {
    int speedDivider = 100;
    int i = 0;
    uint8_t secondHand;
    static uint8_t lastSecond = 99;

      switch (effect)
      {
          case 0:
              speedDivider = 50;
              i = (xTaskGetTickCount() * portTICK_PERIOD_MS / speedDivider) % NUM_LEDS;
              leds[i] = CRGB::White;
              if (i > 0) {
                 leds[i - 1] = CRGB::Black;
              }
              FastLED.show();
              FastLED.delay(100);
          break;

          case 1:
            secondHand = ((xTaskGetTickCount() * portTICK_PERIOD_MS) / 1000) % 60;                // Change '60' to a different value to change length of the loop.

            if (lastSecond != secondHand) {                             // Debounce to make sure we're not repeating an assignment.
              lastSecond = secondHand;
              switch(secondHand) {
                case  0: thisrot=1; deltahue=5; break;
                case  5: thisdir=-1; deltahue=10; break;
                case 10: thisrot=5; break;
                case 15: thisrot=5; thisdir=-1; deltahue=20; break;
                case 20: deltahue=30; break;
                case 25: deltahue=2; thisrot=5; break;
                case 30: break;
              }
            }

            rainbow_march();
            FastLED.show();
            FastLED.delay(thisdelay);
          break;

          default:
          break;
      }
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
  // 	delay(2000);

    Serial.begin(115200);

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxRefreshRate(MAX_FPS);
    FastLED.setBrightness(max_bright);
    set_max_power_in_volts_and_milliamps(5, 500);

    pinMode(LED_BUILTIN, OUTPUT);

    xTaskCreate(
                    &taskLedStrip,     /* Task function. */
                    "TaskLedStrip",   /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */

    xTaskCreate(
                    &taskWifi,         /* Task function. */
                    "TaskWifi",       /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
}

void loop() {
    delay(100);
}
