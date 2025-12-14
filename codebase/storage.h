#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include "config.h"

void storage_init();
void storage_add_entry(float grams_drank, float goal, float duration);
void storage_load_entries(Entry entries[MAX_ENTRIES]);
void storage_reset_entries();

#endif // STORAGE_H