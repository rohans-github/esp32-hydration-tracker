#include "speaker.h"

static unsigned long last_alert_time = 0;
static bool alert_enabled = false;

void speaker_init() {
  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(SPEAKER_PIN, LOW);
  last_alert_time = 0;
}

void speaker_alert() {  
    // Generate tone using PWM
    ledcAttach(SPEAKER_PIN, ALERT_FREQUENCY, 8);  // pin, frequency, 8-bit resolution
    ledcWrite(SPEAKER_PIN, 128);  // 50% duty cycle
    
    delay(ALERT_DURATION);
    
    // Stop tone
    ledcWrite(SPEAKER_PIN, 0);
    ledcDetach(SPEAKER_PIN);
    
    last_alert_time = millis();
}

void speaker_stop() {
  ledcWrite(SPEAKER_PIN, 0);
  ledcDetach(SPEAKER_PIN);
  digitalWrite(SPEAKER_PIN, LOW);
}