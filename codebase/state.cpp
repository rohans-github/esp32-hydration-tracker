#include "state.h"

//holds the state of the system, which can either be waiting for the user input or currently tracking hydration
SystemState current_state = STATE_WAITING_USER_INPUT;

SystemState get_state() {
  return current_state;
}
void set_state(SystemState state_param) {
  current_state = state_param;
}