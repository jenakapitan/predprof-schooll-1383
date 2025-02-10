#include "LED.h"

CRGB leds[NUM_LEDS];

void setupLedStrip(int pin) {
    FastLED.addLeds<LED_TYPE, pin, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

void ledRed() {
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
}

void ledGreen() {
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
}

void ledBlue_blink() {
    for (int i = 0; i < 5; i++) {
        fill_solid(leds, NUM_LEDS, CRGB::Blue);
        FastLED.show();
        delay(500);
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        delay(500);
    }
}
