#include <U8g2lib.h>
#define DELAY 1000

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, 19, 18);

void setup() {
  // put your setup code here, to run once:
  delay(3000);
  oled.begin();
  Serial.begin(115200);
  drawScreen();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void drawScreen() {
  Serial.println("drawing on screen");
  oled.clearBuffer();    //clear the screen contents
  oled.setFont(u8g2_font_t0_18_tf);
  oled.drawStr(0, 15, "Plant monitor!");

  oled.setFont(u8g2_font_unifont_t_symbols);
  oled.drawGlyph(5, 40, 0x2618);
  oled.sendBuffer();
}

