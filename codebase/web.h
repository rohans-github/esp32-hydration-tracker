#ifndef WEB_H
#define WEB_H
#include <WiFi.h>
#include "config.h"

void web_init();
bool webserver_handle_client();
void set_goal_grams(float goal_grams_param);
void set_total_grams(float total_grams_param);
void set_history(Entry entries[]);
bool get_web_request();
void set_web_request(bool input);
void web_enable();
void web_disable();
void set_refresh_flag(bool v);

#endif