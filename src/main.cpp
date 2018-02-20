/*
---------------------------------------------
Controlling WS2812B LED strip, Christmas 2017

Author: Peter Javorsky, MSc.

Date: 13.12.2017
---------------------------------------------
*/

#include <Arduino.h>
#include <FastLED.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

//needed for library
#include <DNSServer.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <WiFiManager.h>
#include <ArduinoOTA.h>

#define EFFECT_DURATION_MS 60 * 1000 // 1min

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
#define UPDATES_PER_SECOND 50

// No. of effects
const int EFFECTS_COUNT = 2;

// effects variables
uint8_t max_bright = 255;
uint8_t thisdelay = 40;
uint8_t thishue = 0;
int8_t thisrot = 1;
uint8_t deltahue = 1;
bool thisdir = 0;
bool isChangemeSet = false;

WiFiManager wifiManager;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void rainbow_march() {                                        // The fill_rainbow call doesn't support brightness levels. You would need to change the max_bright value.
  if (thisdir == 0) thishue += thisrot; else thishue -= thisrot;  // I could use signed math, but 'thisdir' works with other routines.
  fill_rainbow(leds, NUM_LEDS, thishue, deltahue);            // I don't change deltahue on the fly as it's too fast near the end of the strip.
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;

    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;

}

void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;

    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

void taskLedStrip(void *parameter)
{
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

  while (1) {
    int i = 0;
    uint8_t secondHand;
    static uint8_t lastSecond = 99;

    effect = (millis() / EFFECT_DURATION_MS) % EFFECTS_COUNT;

    switch (effect)
    {
      case 0:
        ChangePalettePeriodically();

        static uint8_t startIndex = 0;
        startIndex++; /* motion speed */

        FillLEDsFromPaletteColors(startIndex);

        FastLED.show();
        FastLED.delay(1000 / UPDATES_PER_SECOND);
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
}

void setupWifi() {
    wifiManager.autoConnect("LedStripAP");

    ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void handleWifi() {
    ArduinoOTA.handle();
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

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    //FastLED.setMaxRefreshRate(MAX_FPS);
    //FastLED.setBrightness(max_bright);
    //set_max_power_in_volts_and_milliamps(5, 500);

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
