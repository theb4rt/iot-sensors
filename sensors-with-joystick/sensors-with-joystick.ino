#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

U8G2_SSD1327_MIDAS_128X128_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

enum TemperatureUnit {
  CELSIUS,
  FAHRENHEIT,
  KELVIN
};
TemperatureUnit tempUnit = CELSIUS;

int previousJoystickX = 512; // Center position of joystick X-axis

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

  //Serial.print("Temperature: ");
  //Serial.print(tempToDisplay, 1);
  //Serial.print(" ");
  switch (tempUnit) {
    case FAHRENHEIT:
      //Serial.println("°F");
      break;
    case KELVIN:
      //Serial.println("K");
      break;
    default:
      //Serial.println("°C");
      break;
  }
}

void setup(void) {
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(10, 0);
  digitalWrite(9, 0);
  u8g2.begin();
  sensors.begin();
  Serial.begin(9600);
}

void loop(void) {
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);

  int joystickX = analogRead(A0); // Assuming joystick X-axis is connected to analog pin A0

  if (joystickX > previousJoystickX + 100) {
    // Joystick moved to the right
    tempUnit = static_cast<TemperatureUnit>((tempUnit + 1) % 3);
    delay(300); // Debounce delay
    Serial.println("Joystick moved to the right");
  } else if (joystickX < previousJoystickX - 100) {
    // Joystick moved to the left
    tempUnit = static_cast<TemperatureUnit>((tempUnit + 2) % 3);
    delay(300); // Debounce delay
    Serial.println("Joystick moved to the left");
  }

  previousJoystickX = joystickX;

  u8g2.firstPage();
  do {
    draw_temperature(temperature);
  } while (u8g2.nextPage());

  delay(2000); // Delay for 2 seconds before refreshing the display
}
