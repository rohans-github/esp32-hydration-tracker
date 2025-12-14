#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include "config.h"

SystemState get_state();
void set_state(SystemState state);
extern SystemState current_state;

#endif 