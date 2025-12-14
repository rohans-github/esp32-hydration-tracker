#include <WiFi.h>
#include "web.h"
#include "storage.h"
#include "state.h"
#include "hydration.h"

Entry web_entries[MAX_ENTRIES]; //holds past session data from storage
float web_goal_grams; //holds hydration goal for session
float web_total_grams;  //holds total hydration intake for session
bool web_request = false; //flag that is used to turn enable web functionality
bool refresh_flag = false;  //flag to indicate whether website should be refreshed
WiFiServer server(80);
unsigned long last_isr = 0; //ISR Button debouncing

//WiFi configurations
const char* ssid     = "Hydration Tracker";
const char* password = "12345678";

//connected to web button, which will turn web_request to true
void IRAM_ATTR buttonISR() {
    unsigned long now = millis();
    if (now - last_isr > 50) {  // 50ms debounce
        web_request = true;
    }
    last_isr = now;
}

//function prototypes
String webserver_read_request(WiFiClient& client);
void webserver_send_page(WiFiClient &client);
void set_web_pin_state(uint8_t state);

void web_init() {
  pinMode(WEB_STATUS_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(
        digitalPinToInterrupt(BTN_PIN),         
        buttonISR,
        FALLING             
  );
}

//turns on website
void web_enable() {
  if (DEBUG)
    Serial.println("Starting WiFi AP...");
  WiFi.softAP(ssid, password);

  if (DEBUG) {
    Serial.print("AP Running. IP: ");
    Serial.println(WiFi.softAPIP());
  }
    

  server.begin();
  set_web_pin_state(HIGH);
}

//turns off website
void web_disable() {
  if (DEBUG) Serial.println("Stopping server and turning off WiFi.");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  set_web_pin_state(LOW);
}



// Read HTTP Request Header
String webserver_read_request(WiFiClient &client) {
  String request = "";
  unsigned long timeout = millis();

  while (client.connected() && millis() - timeout < 2000) {
    if (client.available()) {
      char c = client.read();
      request += c;

      // End of headers (blank line)
      if (request.endsWith("\r\n\r\n")) {
        break;
      }
    }
  }
  return request;
}

// Handle AJAX data endpoint  (/data)
void webserver_handle_data(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  //when waiting for user input, load in the past session data, and don't display any goal or total water intake readigns
  if (get_state() == STATE_WAITING_USER_INPUT && refresh_flag == false) {
      client.println("{\"web_goal_grams\":\"--\",");
      client.println("\"web_total_grams\":\"--\",");
      client.print("\"history\":[");
      for (int i = 0; i < MAX_ENTRIES; i++) {
        client.print("{\"d\":");
        client.print(web_entries[i].grams_drank);
        client.print(",\"g\":");
        client.print(web_entries[i].goal);
        client.print(",\"t\":");
        client.print(web_entries[i].duration);
        client.print("}");
        if (i < MAX_ENTRIES - 1) client.print(",");
      }
      client.print("],");
      client.print("\"refresh\":false");
      client.println("}");
      return;
  }
  //pass in goal and water intake measurements
  client.print("{\"web_goal_grams\":");
  client.print(web_goal_grams, 1);
  client.print(",\"web_total_grams\":");
  client.print(web_total_grams, 1);

  //send historical session data as an array
  client.print(",\"history\":[");
  for (int i = 0; i < MAX_ENTRIES; i++) {
    client.print("{\"d\":");
    client.print(web_entries[i].grams_drank);
    client.print(",\"g\":");
    client.print(web_entries[i].goal);
    client.print(",\"t\":");
    client.print(web_entries[i].duration);
    client.print("}");
    if (i < MAX_ENTRIES - 1) client.print(",");
  }
  //refresh capability
  client.print("],\"refresh\":");
  client.print(refresh_flag ? "true" : "false");
  refresh_flag = false;
  client.print("}");
}

//HTML page
void webserver_send_page(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html><html>");
  client.println("<head><title>Hydration Monitor</title></head>");
  client.println("<body style='font-family:sans-serif; text-align:center;'>");
  client.println("<h1 style='font-size:48px;'>Hydration Monitor</h1>");
  client.println("<p style='font-size:10px;'>Your Goal This Session Is <span id='web_goal_grams'>--</span> mL</p>");
  client.println("<p style='font-size:10px;'>You Drank <span id='web_total_grams'><b>--</b></span> mL This Session</p>");


  // Overlay for user input
  if (get_state() == STATE_WAITING_USER_INPUT) {
    client.println(R"rawliteral(
    <style>
      /* Fullscreen semi-transparent background */
    #overlay { 
        position: fixed; 
        top: 0; left: 0; 
        width: 100%; height: 100%; 
        background: rgba(0,0,0,0.6); 
        display: flex; 
        justify-content: center; 
        align-items: center; 
        z-index: 1000; 
    }

    /* Centered overlay box */
    #overlay-box { 
        background: white; 
        padding: 60px 50px;       /* bigger padding */
        border-radius: 20px;      /* slightly more rounded */
        text-align: center; 
        min-width: 400px;         /* wider */
        box-shadow: 0 8px 25px rgba(0,0,0,0.3);
    }

    #overlay-box h2 { 
        margin-bottom: 30px; 
        font-size: 36px;           /* bigger heading */
        color: #333; 
    }

    #overlay-box div { 
        margin: 20px 0; 
        font-size: 20px;           /* bigger labels */
        color: #444; 
    }

    #overlay-box input { 
        width: 180px;              /* bigger input */
        padding: 12px; 
        font-size: 20px; 
        margin-left: 15px; 
        border: 1px solid #ccc; 
        border-radius: 6px; 
    }

    #overlay-box button { 
        margin-top: 30px; 
        padding: 15px 35px;        /* bigger button */
        font-size: 20px; 
        background-color: white; 
        color: black; 
        border: 2px solid black; 
        border-radius: 10px; 
        cursor: pointer; 
    }
    </style>

    <div id='overlay'>
  <div id='overlay-box'>
    <h2>Set Session Goal</h2>

    <div style='margin-top:10px; font-size:20px; display:flex; align-items:center; gap:10px; justify-content:center;'>
  <label style='font-size:20px;'>Duration:</label>

  <!-- Hours input -->
  <input 
    type='number' 
    id='inputHours' 
    min='0'
    max='24'
    oninput='checkInputs()'
    style='width:70px; border:none; border-bottom:2px solid black; text-align:center; font-size:20px; outline:none;'>
  <span style='font-size:20px;'>hr</span>

  <!-- Minutes input -->
  <input 
    type='number' 
    id='inputMinutes' 
    min='0'
    max='59'
    oninput='checkInputs()'
    style='width:70px; border:none; border-bottom:2px solid black; text-align:center; font-size:20px; outline:none;'>
  <span style='font-size:20px;'>min</span>

  <!-- Seconds input -->
  <input 
    type='number' 
    id='inputSeconds' 
    min='0'
    max='59'
    oninput='checkInputs()'
    style='width:70px; border:none; border-bottom:2px solid black; text-align:center; font-size:20px; outline:none;'>
  <span style='font-size:20px;'>s</span>
  </div>

  <div style='margin-top:25px; font-size:20px; display:flex; align-items:center; gap:10px; justify-content:center;'>
    <label style='font-size:20px;'>Goal (mL):</label>
    <input 
      type='number' 
      id='inputGoal' 
      min='100'
      oninput='checkInputs()'
      style='width:120px; border:none; border-bottom:2px solid black; text-align:center; font-size:20px; outline:none;'>
  </div>

      <button id='submitBtn' onclick='submitOverlay()' disabled>Enter</button>
    </div>
  </div>

  <script>
  function checkInputs() {
    const h = document.getElementById('inputHours').value;
    const m = document.getElementById('inputMinutes').value;
    const s = document.getElementById('inputSeconds').value;
    const g = document.getElementById('inputGoal').value;

    const btn = document.getElementById('submitBtn');

    // Enable button only if all fields are non-empty
    if ((h + m + s) > 0 && g > 0) {
      btn.disabled = false;
    } else {
      btn.disabled = true;
    }
  }

  function submitOverlay() {
    const hr = parseInt(document.getElementById('inputHours').value) || 0;
    const min = parseInt(document.getElementById('inputMinutes').value) || 0;
    const sec = parseInt(document.getElementById('inputSeconds').value) || 0;

    // Convert to total seconds
    const totalDuration = hr * 3600 + min * 60 + sec;

    const goal = document.getElementById('inputGoal').value;
    document.getElementById('overlay').style.display='none';

    fetch(`/set_goal?duration=${totalDuration}&goal=${goal}`)
      .then(r => r.text())
      .then(d => console.log(d));
    }
    </script>
    )rawliteral");
  }

  //Hydration circle
  client.println(R"rawliteral(
  <div style='margin:20px auto; width:300;'>
    <svg id='hydrationCircle' viewBox='0 0 36 36' style='width:300px; height:300px;'>
      <!-- Background circle (uncovered portion) -->
      <path stroke='#00aaff' stroke-opacity='0.2' stroke-width='4' fill='none'
            d='M18 2 a 16 16 0 1 1 0 32 a 16 16 0 1 1 0 -32'></path>

      <!-- Progress circle (covered portion) -->
      <path id='hydrationProgress' stroke='#00aaff' stroke-width='4' fill='none'
            stroke-linecap='round' stroke-dasharray='0,100'
            d='M18 2 a 16 16 0 1 1 0 32 a 16 16 0 1 1 0 -32'></path>

      <!-- Center text -->
      <text id='hydrationText' x='18' y='20' font-size='8' text-anchor='middle' fill='#000'>0%</text>
    </svg>
    <div id='hydrationAmount' style='font-size:20px; font-weight:bold; margin-top:10px;'>0 mL</div>
  </div>

  <script>
  function updateHydrationCircle(percent, amount){
    const circle = document.getElementById('hydrationProgress');
    const text = document.getElementById('hydrationText');
    // percent between 0 and 100
    circle.setAttribute('stroke-dasharray', percent + ',100');
    text.textContent = percent + '%';
    document.getElementById('hydrationAmount').textContent = amount + ' mL';
  }
  </script>
  )rawliteral");

  // Past session data
  client.println("<div style='margin-top:30px;'></div>");
  client.println("<h2 style='font-size:36px;'>Your Past 7 Sessions</h2>");  
  client.println("<div id='historyCards' style='width:80%; margin:20px auto;'></div>");
  client.println("<button onclick=\"fetch('/action')\" style='padding:15px 30px; font-size:30px; background-color:white; border:2px solid #000000; border-radius:12px; cursor:pointer;'>Reset History</button>");

  // AJAX update
  client.println(R"rawliteral(
  <script>
  setInterval(function(){
    fetch('/data')
      .then(r => r.json())
      .then(d => {
        document.getElementById('web_goal_grams').innerHTML = d.web_goal_grams;
        document.getElementById('web_total_grams').innerHTML = '<b>' + d.web_total_grams + '</b>';
        if(d.refresh) location.reload();
        let cardsHTML='';
        for(let i=0;i<d.history.length;i++){
          const bgColor =
          (d.history[i].d === 0 && d.history[i].g === 0)
            ? '#ffffff'
            : (d.history[i].d < d.history[i].g)
              ? '#ffe6e6'
              : '#e6ffe6';
          cardsHTML += `<div style='border:1px solid black;border-radius:10px;padding:12px;margin-bottom:20px;background:${bgColor};'>
            <div><b>Session Length:</b> ${d.history[i].t} s</div>
            <div><b>Drank:</b> ${d.history[i].d} mL</div>
            <div><b>Goal:</b> ${d.history[i].g} mL</div>
          </div>`;
        }
        document.getElementById('historyCards').innerHTML = cardsHTML;
        let pct = Math.round((d.web_total_grams / d.web_goal_grams) * 100);
        updateHydrationCircle(pct || 0, d.web_total_grams);
      });
  },1000);
  </script>
  )rawliteral");

  client.println("</body></html>");
}

//main webserver handler
bool webserver_handle_client() {
  WiFiClient client = server.available();
  if (!client) return false;

  // Read request
  String request = webserver_read_request(client);

  // AJAX endpoint
  if (request.indexOf("GET /data") >= 0) {
    webserver_handle_data(client);
    client.stop();
    return true;
  }

  //HTML for reset button, which resets the past session entries
  if (request.indexOf("GET /action") >= 0) {
      storage_reset_entries();
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("Action completed.");
      client.stop();
      return true;
  }

  // Handle goal/duration update
  if (request.indexOf("GET /set_goal") >= 0) {
    // Parse query parameters
    int idxGoal = request.indexOf("goal=");
    int idxDuration = request.indexOf("duration=");
    if (idxGoal >= 0 && idxDuration >= 0) {
      float goal = request.substring(idxGoal + 5, request.indexOf('&', idxGoal)).toFloat();
      float duration = request.substring(idxDuration + 9, request.indexOf(' ', idxDuration)).toFloat();
      
      //pass in user input for goal and session duration for hydration.cpp calculations
      set_goal(goal);
      set_time_length(duration);
      reset();  //necessary for hydration.cpp to lock in new user input values
      set_state(STATE_RUNNING);  //state should be running to unlock rest of system

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("Goal and duration updated.");
      client.stop();
      return true;
    }
  }
  //--------------------------------------------------
  // Main HTML page
  //--------------------------------------------------
  webserver_send_page(client);
  client.stop();

  if (DEBUG) Serial.println("Client disconnected.");
}

//GETTERS AND SETTERS
bool get_web_request() {
  return web_request;
}

void set_refresh_flag(bool v) {
  refresh_flag = v;
}

void set_goal_grams(float goal_grams_param) {
  web_goal_grams = goal_grams_param;
}

void set_total_grams(float total_grams_param) {
  web_total_grams = total_grams_param;
}

void set_history(Entry entries[]) {
  for (int i = 0; i < MAX_ENTRIES; i++) {
      web_entries[i] = entries[i];
  }
}

void set_web_pin_state(uint8_t state) {
  digitalWrite(WEB_STATUS_PIN, state);
}

void set_web_request(bool input) {
  web_request = input;
}




