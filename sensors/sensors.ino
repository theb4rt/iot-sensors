#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

U8G2_SSD1327_MIDAS_128X128_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void draw_temperature(float temperature) {
  u8g2_prepare();
  u8g2.setCursor(10, 10);
  u8g2.print("Temp:");

  u8g2.setFont(u8g2_font_ncenB24_tr); // Use a larger font for the temperature value
  u8g2.setCursor(10, 50);
  u8g2.print(temperature, 1);
  u8g2.print(" °C");

  // Draw the sun symbol
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.drawUTF8(54, 90,"℃");
}

void setup(void) {
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(10, 0);
  digitalWrite(9, 0);
  u8g2.begin();
  sensors.begin();
}

void loop(void) {
  sensors.requestTemperatures(); 
  float temperature = sensors.getTempCByIndex(0);

  u8g2.firstPage();
  do {
    draw_temperature(temperature);
  } while (u8g2.nextPage());

  delay(2000); // Delay for 2 seconds before refreshing the display
}
