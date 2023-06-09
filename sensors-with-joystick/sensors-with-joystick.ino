#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

U8G2_SSD1327_MIDAS_128X128_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

enum TemperatureUnit {
  CELSIUS,
  FAHRENHEIT,
  KELVIN
};
TemperatureUnit tempUnit = CELSIUS;

#define JOY_X_PIN A7
#define JOY_Y_PIN A6
#define JOY_BUTTON_PIN 52

#define JOY_CENTER 512
#define JOY_DEADZONE 100

#define DEBOUNCE_DELAY 10          // Debounce time in milliseconds
unsigned long lastSwitchTime = 0;  


void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void draw_temperature(float temperature) {
  float tempToDisplay;

  switch (tempUnit) {
    case FAHRENHEIT:
      tempToDisplay = temperature * 1.8 + 32.0;
      break;
    case KELVIN:
      tempToDisplay = temperature + 273.15;
      break;
    default:
      tempToDisplay = temperature;
  }

  u8g2_prepare();
  u8g2.setCursor(10, 10);
  u8g2.print("Temp:");

  u8g2.setFont(u8g2_font_ncenB24_tr);
  u8g2.setCursor(10, 50);
  u8g2.print(tempToDisplay, 1);

  switch (tempUnit) {
    case FAHRENHEIT:
      u8g2.print(" °F");
      break;
    case KELVIN:
      u8g2.print(" K");
      break;
    default:
      u8g2.print(" °C");
      break;
  }

  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.drawUTF8(54, 90, "℃");
}

void setup(void) {
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(10, 0);
  digitalWrite(9, 0);
  pinMode(JOY_BUTTON_PIN, INPUT_PULLUP);
  u8g2.begin();
  sensors.begin();
  Serial.begin(9600);
  u8g2_prepare();
}

void loop(void) {
  float temperature = 11.01;

  int joyX = analogRead(JOY_X_PIN);
  int joyY = analogRead(JOY_Y_PIN);
  int joyButton = digitalRead(JOY_BUTTON_PIN);


  if ((joyY > JOY_CENTER + JOY_DEADZONE) || (joyX > JOY_CENTER + JOY_DEADZONE)) {
    if ((millis() - lastSwitchTime) > DEBOUNCE_DELAY) {
      lastSwitchTime = millis();
      switch (tempUnit) {
        case CELSIUS:
          tempUnit = FAHRENHEIT;
          break;
        case FAHRENHEIT:
          tempUnit = KELVIN;
          break;
        case KELVIN:
          tempUnit = CELSIUS;
          break;
      }
    }
  }
  else if ((joyY < JOY_CENTER - JOY_DEADZONE) || (joyX < JOY_CENTER - JOY_DEADZONE)) {
    if ((millis() - lastSwitchTime) > DEBOUNCE_DELAY) {
      lastSwitchTime = millis();
      switch (tempUnit) {
        case CELSIUS:
          tempUnit = KELVIN;
          break;
        case FAHRENHEIT:
          tempUnit = CELSIUS;
          break;
        case KELVIN:
          tempUnit = FAHRENHEIT;
          break;
      }
    }
  }

  u8g2.firstPage();
  do {
    draw_temperature(temperature);
  } while (u8g2.nextPage());
}