#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>
#import <Adafruit_NeoPixel.h>
#include "config.h"
#include "state.h"

// Function prototypes
void status_led_init();
void status_led_update(HydrationState state);
void status_led_show_waiting();

#endif // STATUS_LED_H