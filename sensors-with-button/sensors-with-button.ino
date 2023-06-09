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

const int BUTTON_PIN = 48;           // Button pin
bool buttonState = HIGH;             // current state of the button
bool lastButtonState = HIGH;         // previous state of the button
unsigned long lastDebounceTime = 0;  // last time the button state changed
unsigned long debounceDelay = 20;    // debounce delay in milliseconds
unsigned long lastOLEDUpdateTime = 0; // last time the OLED was updated
unsigned long oledUpdateDelay = 2000; // OLED update delay in milliseconds

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
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  u8g2.begin();
  sensors.begin();
  Serial.begin(9600);
  u8g2_prepare();
}

void loop(void) {
  //sensors.requestTemperatures();
  //float temperature = sensors.getTempCByIndex(0);
  float temperature = 11.01;

  int reading = digitalRead(BUTTON_PIN);

  // Check if button state has changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // reset the debounce timer
  }

  // If debounce period has passed
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed
    if (reading != buttonState) {
      buttonState = reading;

      // If the button has just been pressed
      if (buttonState == LOW) {
        Serial.println("Button Pressed");
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
  }

  // Save the current button state for the next loop iteration
  lastButtonState = reading;

  // Update OLED display
  if ((millis() - lastOLEDUpdateTime) > oledUpdateDelay) {
    lastOLEDUpdateTime = millis(); // reset the OLED update timer
    u8g2.firstPage();
    do {
      draw_temperature(temperature);
    } while (u8g2.nextPage());
  }
}