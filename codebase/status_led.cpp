#include "status_led.h"

static LEDColor current_color = COLOR_OFF;
Adafruit_NeoPixel rgb(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void status_led_set_color(LEDColor color);
void status_led_set_from_hydration_state(HydrationState state);

void status_led_init() {
  rgb.begin();
  rgb.show();
}

//white LED to show that system is waiting for user input
void status_led_show_waiting() {
  rgb.setPixelColor(0, rgb.Color(120, 120, 120));
  rgb.show();
}

//sets the LED to the passed in color
void status_led_set_color(LEDColor color) {
  rgb.setPixelColor(0, rgb.Color(color.red, color.green, color.blue));
  rgb.show();
}

//passes in the corresponding color to the passed in hydration state
void status_led_set_from_hydration_state(HydrationState state) {
  LEDColor color;
    switch(state) {
        case HYDRATED:
          color = COLOR_GOOD;
          break;
          
        case NEEDS_WATER:
          color = COLOR_OK;
          break;
          
        case CRITICAL:
          color = COLOR_CRITICAL;
          break;
        
        case COMPLETED:
          color = COLOR_COMPLETED;
          break;

        default:
          color = COLOR_OFF;
          break;
      }
  status_led_set_color(color);
}

void status_led_update(HydrationState state) {
  // Get current hydration state and update LED accordingly
  status_led_set_from_hydration_state(state);
}