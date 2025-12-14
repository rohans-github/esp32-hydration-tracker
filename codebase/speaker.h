#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>
#include "config.h"


// Function prototypes
void speaker_init();
void speaker_alert();
void speaker_stop();
bool speaker_should_alert();
void speaker_task_handler();

#endif // SPEAKER_H