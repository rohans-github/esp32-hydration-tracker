#include "hydration.h"
#include "storage.h"
#include <Arduino.h>

HydrationState hydration_state = NEEDS_WATER;
float initial_time; //used to determine elapsed time
float goal; //session water intake goal
float grams_left; //grams left to reach goal
float pacer;  //mainly used to determine hydration state, acts as an "ideal" grams_left
float total_grams;  //total water intake so far during the session
unsigned long time_period_ms; //duration of session in milliseconds

//adds the grams_drank parameter to total water intake
void record_grams_drank(float grams_drank) {
  grams_left = grams_left - grams_drank;
  total_grams += grams_drank;
}

//gets called every time a new session starts, resets variables and locks in new user inputs from website
void reset() {
  initial_time = 0;
  pacer = goal;
  grams_left = goal;
  total_grams = 0;
  hydration_state = NEEDS_WATER;
}

//determines hydration status based on time left and how much the user has drank so far
void update_hydration_status() {
  if (initial_time == 0) {
    initial_time = millis();
  }
  
  float time_now = millis();
  float time_since_start = time_now - initial_time;
  
  // the pacer linearly decreases from the goal, represents an ideal grams_left
  float subtract = goal/time_period_ms * time_since_start;
  pacer = (goal - subtract < 0) ? 0 : goal - subtract;

  if (DEBUG) {
    Serial.print("grams_left=");
    Serial.print(grams_left);
    Serial.print(" pacer=");
    Serial.println(pacer);
  }

  //if met goal, then set state to COMPLETED
  if (grams_left <= 0) {
    hydration_state = COMPLETED;
  }
  // Update hydration state based on deviation from ideal drink pacer
  else {
    if (pacer - grams_left <= -1 * goal / 5) {
      hydration_state = CRITICAL;
    }
    else if (pacer - grams_left <= 0) {
      hydration_state = NEEDS_WATER;
    }
    else {
      hydration_state = HYDRATED;
    }
  }
}

//GETTERS AND SETTERS
HydrationState get_hydration_state() {
  return hydration_state;
}

HydrationState set_hydration_state(HydrationState state_param) {
  hydration_state = state_param;
}

float get_pacer() {
  return pacer;
}

float get_total_grams() {
  return goal - grams_left;
}

float get_goal_grams() {
  return goal;
}

void set_goal(int goal_param) {
  goal = goal_param;
}

void set_time_length(int seconds) {
  time_period_ms = seconds * 1000;
}

int get_time_length() {
  return time_period_ms/1000;
}