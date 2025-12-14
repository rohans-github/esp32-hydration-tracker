#ifndef HYDRATION_H
#define HYDRATION_H
#include "config.h"
#include "storage.h"

void hydration_init();
void update_hydration_status();
HydrationState get_hydration_state();
void record_grams_drank(float grams_drank);
float get_pacer();
float get_goal_grams();
void reset();
float get_total_grams();
void set_goal(int goal);
void set_time_length(int seconds);
int get_time_length();
HydrationState set_hydration_state(HydrationState state_param);
#endif
