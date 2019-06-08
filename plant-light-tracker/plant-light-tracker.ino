#include <U8g2lib.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "secrets.h"

// hardware constants
#define DELAY 1000
const int SCL_PIN = 5;
const int SDA_PIN = 4;
const int BUTTON_PIN = 2;
const int PHOTOCELL_PIN = A0;
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int LIGHT_MIN = 0;
const int LIGHT_THRESHOLD = 600;
const int LIGHT_MAX = 800;
const int BUFFER_SIZE = 30;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0); //, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);


// wifi constants
const int response_timeout = 6000;
const String LOCATION = "America/New_York";
const int MIN_BETWEEN_FETCHES = 60;
time_t t_last_fetch;
time_t today;
const int WIFI_STATE_IDLE = 0;
const int WIFI_STATE_FETCHING = 1;
int wifi_state = WIFI_STATE_FETCHING;

// light constants
time_t t_received;
time_t t_last_light;
int photocell_reads[BUFFER_SIZE];
int avg_photocell_reading = 0;
int read_index = 0;
const int LIGHT_STATE_IDLE = 0;
const int LIGHT_STATE_COUNTING = 1;
int light_state = LIGHT_STATE_IDLE;

// button/screen constants
time_t t_last_on;
const int OLED_ON_SEC = 10;
int button_reading;
const int SCREEN_OFF = 0;
const int SCREEN_ON = 1;
const int SCREEN_OFF_BUTTON_PUSHED = 2;
const int SCREEN_ON_BUTTON_PUSHED = 3;
int screen_state = SCREEN_ON;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  oled.begin();
  Serial.begin(115200);
  setup_wifi();
}

void loop() {
  oled.clearBuffer();
  avg_photocell_reading = averaging_filter(
                            analogRead(PHOTOCELL_PIN),
                            photocell_reads,
                            8,
                            read_index
                          );
  button_reading = digitalRead(BUTTON_PIN);

  switch (light_state) {
    case LIGHT_STATE_IDLE:
      if (avg_photocell_reading > LIGHT_THRESHOLD) {
        t_last_light = now();
        light_state = LIGHT_STATE_COUNTING;
      }
      break;
    case LIGHT_STATE_COUNTING:
      unsigned long delta_light = now() - t_last_light;
      t_received += delta_light;
      t_last_light = now();
      if (avg_photocell_reading < LIGHT_THRESHOLD) {
        light_state = LIGHT_STATE_IDLE;
      }
      break;
  }

  switch (screen_state) {
    case SCREEN_ON:
      if (second(now() - t_last_on) > OLED_ON_SEC) {
        oled.setPowerSave(1);
        screen_state = SCREEN_OFF;
      }
      else if (!button_reading) {
        screen_state = SCREEN_ON_BUTTON_PUSHED;
      }
      break;
    case SCREEN_ON_BUTTON_PUSHED:
      if (button_reading) {
        oled.setPowerSave(1);
        screen_state = SCREEN_OFF;
      }
      break;
    case SCREEN_OFF:
      if (!button_reading) {
        screen_state = SCREEN_OFF_BUTTON_PUSHED;
      }
      break;
    case SCREEN_OFF_BUTTON_PUSHED:
      if (button_reading) {
        oled.setPowerSave(0);
        t_last_on = now();
        screen_state = SCREEN_ON;
      }
      break;
  }

  switch (wifi_state) {
    case WIFI_STATE_IDLE:
      if (minute(now() - t_last_fetch) > MIN_BETWEEN_FETCHES) {
        wifi_state = WIFI_STATE_FETCHING;
      }
      if (day(now()) != day(today)) {
        // reset amount of light received around midnight
        Serial.println("It's a new day...");
        t_received = 0;
        wifi_state = WIFI_STATE_FETCHING;
      }
      break;
    case WIFI_STATE_FETCHING:
      get_time();
      wifi_state = WIFI_STATE_IDLE;
      break;
  }

  drawHeader();
  drawBody();
  drawFooter(avg_photocell_reading);

  oled.sendBuffer();
}

void drawHeader() {
  oled.setFont(u8g2_font_t0_15_tr);
  oled.drawStr(0, 15, "Plant monitor");
  oled.setFont(u8g2_font_unifont_t_symbols);
  oled.drawGlyph(107, 15, 0x2618); // clover
}

void drawBody() {
  oled.setFont(u8g2_font_t0_11_tr);
  char message[40];
  snprintf(message, 40, "Today's sun: %02d:%02d:%02d",
           hour(t_received),
           minute(t_received),
           second(t_received)
          );
  oled.drawStr(0, 30, message);
}

/**
   Averaging filter from 6.08!
   https://iesc-s2.mit.edu/608/spring18
*/
int averaging_filter(int input, int stored_values[BUFFER_SIZE], int order, int &index) {
  stored_values[index] = input;
  int out = 0;
  float multiplier = 1.0 / (order + 1);
  if (order == 0) {
    out = input;
  } else {
    for (int i = 0; i <= order; i++) {
      out += stored_values[i];
    }
    index += 1;
    index %= (order + 1);
  }
  return multiplier * out;
}

void drawFooter(int reading) {
  int padding = 0;
  int glyph_width = 10;

  int bar_height = 10;
  int bar_width = SCREEN_WIDTH - padding * 2 - glyph_width;
  int bar_x = padding + glyph_width;
  int bar_y = SCREEN_HEIGHT - bar_height - padding;
  int glyph_x = padding;
  int glyph_y = SCREEN_HEIGHT - padding;
  int bar_scaled_width = map(reading, LIGHT_MIN, LIGHT_MAX, 0, bar_width);

  int symbol;
  if (light_state == LIGHT_STATE_COUNTING) {
    symbol = 0x2600;
  }
  else {
    symbol = 0x2601;
  }

  oled.setFont(u8g2_font_unifont_t_symbols);
  oled.drawGlyph(glyph_x, glyph_y, symbol);
  oled.drawFrame(bar_x, bar_y, bar_width, bar_height);
  oled.drawBox(bar_x, bar_y, bar_scaled_width, bar_height);
}

void setup_wifi() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_t0_11_tr);
  oled.drawStr(0, 40, "CONNECTING TO WIFI");
  oled.setFont(u8g2_font_unifont_t_symbols);
  oled.drawGlyph(109, 40, 0x23f3); // wifi??
  oled.sendBuffer();
  int count = 0;
  WiFi.disconnect();
  WiFi.begin(MY_SSID, MY_PW);
  while (WiFi.status() != WL_CONNECTED && count < 150) {
    delay(500);
    Serial.println(count);
    count++;
  }
  delay(2000);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi connected!");
    oled.clearBuffer();
    oled.setFont(u8g2_font_t0_17_tr);
    oled.drawStr(0, 40, "Connected!");
    oled.setFont(u8g2_font_unifont_t_symbols);
    oled.drawGlyph(107, 40, 0x2605);
    oled.sendBuffer();
  } else {
    Serial.println(WiFi.status());
    ESP.restart();
  }
}

void get_time() {
  String host = "api.timezonedb.com";
  String path = "v2/get-time-zone";
  String params = "?key=" + String(API_KEY) +
                  "&by=zone" +
                  "&zone=" + LOCATION +
                  "&fields=formatted";
  String time_str = get_request(host, path, params);
  String prefix = "<formatted>";
  String suffix = "</formatted>";
  int time_start = time_str.indexOf(prefix) + prefix.length();
  int time_end = time_str.indexOf(suffix);
  time_str = time_str.substring(time_start, time_end);
  int yr =   time_str.substring(0, 4).toInt();
  int mnth = time_str.substring(5, 7).toInt();
  int dy =   time_str.substring(8, 10).toInt();
  int hr =   time_str.substring(11, 13).toInt();
  int mn =   time_str.substring(14, 16).toInt();
  int sec =  time_str.substring(17, 19).toInt();
  setTime(hr, mn, sec, dy, mnth, yr);
  today = now();
}

String get_request(String host, String path, String params) {
  WiFiClient client;
  if (client.connect(host, 80)) {
    client.println("GET http://" + host + "/" + path + params + " HTTP/1.1");
    client.println("Host: " + host);
    client.print("\r\n\r\n");
    unsigned long count = millis();
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break;
      }
      if (millis() - count > response_timeout) break;
    }
    count = millis();
    String op; //create empty String object
    while (client.available()) {
      op += (char)client.read();
    }
    client.stop();
    return op;
  } else {
    Serial.println("connection failed");
    Serial.println("wait 0.5 sec...");
    client.stop();
    delay(500);
  }
}
