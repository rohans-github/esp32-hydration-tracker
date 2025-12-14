#include <Arduino.h>
#include "speaker.h"
#include "hydration.h"
#include "status_led.h"
#include "scale.h"
#include "storage.h"
#include "web.h"
#include "state.h"

/*
Updates onboard LED to show various conditions
*/
void taskUpdateStatusLED(void *pv) {
  while(1) {
    //until user enters required input for the session, make LED turn white
    while (get_state() == STATE_WAITING_USER_INPUT) {
      status_led_show_waiting();
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    //updates LED based on hydration state
    status_led_update(get_hydration_state());
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

/*
Speaker that audibly alerts the user when the LED is red and the user needs to drink water
*/
void taskAlertUser(void *pv) {
  while(1) {
    //while waiting for user input don't use the speaker
    while (get_state() == STATE_WAITING_USER_INPUT) {
      speaker_stop();
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    //if hydration state is critical then sound speaker
    if (get_hydration_state() == CRITICAL) {
      speaker_alert();
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/*
Generates HTML page allowing user to view hydration data and interact with system wirelessly through Wifi
*/
void taskHTMLPage(void *pv) {
  while(1) {   
    //if user presses web page button, turn on web page
    if (get_web_request()) { 
      web_enable();
      unsigned long client_connect_time = millis();
      
      while(1) {
        bool no_client = false;
        //if waiting for user input then don't read from hydration.cpp
        if (get_state() == STATE_RUNNING) {
          set_goal_grams(get_goal_grams());
          set_total_grams(get_total_grams());
        }
        
        //if true, then there is a client connected
        if (webserver_handle_client()) {
          client_connect_time = millis();
        }
        //keep web page on until CLIENT_TIMEOUT_SECS seconds pass with no client
        if (millis() - client_connect_time > CLIENT_TIMEOUT_SECS * 1000) {
          break;
        }
        vTaskDelay(5);
      }
      //disable the website
      set_web_request(false);
      web_disable();
    }
    vTaskDelay(10);
  }
}

/*
Detects scale changes and stores meaningful changes in non-volatile memory
*/
void taskReadScale(void *pv) {
  float prev_weight = 0;  //stores last weight detected
  Entry html_page_entries[MAX_ENTRIES]; //holds data from storage to be populated to web
  storage_load_entries(html_page_entries);  //load entries from non-volatile memory to html_page_entries
  set_history(html_page_entries); //sends data to website
  while (1) {
    //while waiting for user input, do not use the scale
    while (get_state() == STATE_WAITING_USER_INPUT) {
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    //read the weight on the scale
    float current_weight = scale_read_weight();
    
    // If there is something on the scale, weight decreased, and difference is meaningful
    if (current_weight > 30.0 && current_weight < prev_weight && prev_weight - current_weight > 30.0) {
      record_grams_drank(prev_weight - current_weight);  //record the difference in weight between previous reading and current reading
    }

    if (current_weight > 30.0)
      prev_weight = current_weight; //save current weight as previous weight to detect drops in weight

    update_hydration_status();

    //if the pacer is 0, then that means the session ended
    if (get_pacer() == 0) {
      //store the sessions total water intake, the goal, and the session length in memory
      storage_add_entry(get_total_grams(), get_goal_grams(), get_time_length());

      //load past sessions into web page
      storage_load_entries(html_page_entries);
      set_history(html_page_entries);
      
      //refresh when session ends so user input overlay appear automatically
      set_refresh_flag(true);
      //user needs to input new information for new session after this session ends
      set_state(STATE_WAITING_USER_INPUT);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  //init
  Serial.begin(115200);
  scale_init();
  status_led_init();
  web_init();
  speaker_init();
  storage_init();
  
  //task creation
  xTaskCreate(taskUpdateStatusLED, "taskUpdateStatusLED", 2048, NULL, 3, NULL);
  xTaskCreate(taskAlertUser, "taskAlertUser", 2048, NULL, 3, NULL);
  xTaskCreate(taskReadScale, "taskReadScale", 4096, NULL, 1, NULL);
  xTaskCreate(taskHTMLPage, "taskHTMLPage", 4096, NULL, 2, NULL);
}

void loop() {
}