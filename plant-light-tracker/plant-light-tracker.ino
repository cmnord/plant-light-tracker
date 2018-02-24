#include <U8g2lib.h>
#define DELAY 1000

// constants
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, 19, 18);
const int BUTTON_PIN = 3;
const int PHOTOCELL_PIN = 23;
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int LIGHT_THRESHOLD = 100;

int photocellReading;

void setup() {
  delay(3000);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  oled.begin();
  Serial.begin(115200);
}

void loop() {
  oled.clearBuffer();
  drawScreen();
  photocellReading = analogRead(PHOTOCELL_PIN);
  drawBar(photocellReading);

  oled.sendBuffer();
}

void drawScreen() {
  oled.setFont(u8g2_font_t0_15_tr);
  oled.drawStr(0, 15, "Plant monitor");

  oled.setFont(u8g2_font_unifont_t_symbols);
  oled.drawGlyph(107, 15, 0x2618);
}

void drawBar(int reading) {
  int padding = 0;
  int glyph_width = 10;

  int bar_height = 10;
  int bar_width = SCREEN_WIDTH - padding * 2 - glyph_width;
  int bar_x = padding + glyph_width;
  int bar_y = SCREEN_HEIGHT - bar_height - padding;
  int glyph_x = padding;
  int glyph_y = SCREEN_HEIGHT - padding;
  int bar_scaled_width = map(reading, 0, 400, 0, bar_width);

  int symbol;
  if (reading > LIGHT_THRESHOLD) {
    symbol = 0x2600;
  }
  else {
    symbol = 0x2601;
  }

  oled.drawGlyph(glyph_x, glyph_y, symbol);
  oled.drawFrame(bar_x, bar_y, bar_width, bar_height);
  oled.drawBox(bar_x, bar_y, bar_scaled_width, bar_height);
}

