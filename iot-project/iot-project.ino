#include <U8g2lib.h>
#include <DHT.h>

#define DHT_PIN 2 // Data pin for DHT11 sensor is connected to digital pin 2 on Arduino

U8G2_SSD1327_MIDAS_128X128_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
DHT dht(DHT_PIN, DHT11);

enum DisplayMode {
  TEMPERATURE,
  HUMIDITY,
  ANOTHER_SENSOR // Placeholder for a third sensor
};

enum State {
  LOCKED,
  UNLOCKED
};

DisplayMode displayMode = TEMPERATURE;
State currentState = LOCKED;

#define JOY_X_PIN A7
#define JOY_Y_PIN A6
#define JOY_BUTTON_PIN 52

#define JOY_CENTER 512
#define JOY_DEADZONE 100
#define DEBOUNCE_DELAY 200
#define REFRESH_INTERVAL 3000
#define SEND_INTERVAL 5000

unsigned long lastSwitchTime = 0;
unsigned long lastRefreshTime = 0;
unsigned long lastSendTime = 0;

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
  u8g2.setFont(u8g2_font_ncenB24_tr);
  u8g2.setCursor(10, 50);
  u8g2.print(temperature, 1);
  u8g2.print(" C");
}

void draw_humidity(float humidity) {
  u8g2_prepare();
  u8g2.setCursor(10, 10);
  u8g2.print("Humidity:");
  u8g2.setFont(u8g2_font_ncenB24_tr);
  u8g2.setCursor(10, 50);
  u8g2.print(humidity, 1);
  u8g2.print(" %");
}

void draw_another_sensor(float value) {
  u8g2_prepare();
  u8g2.setCursor(10, 10);
  u8g2.print("Sensor:");
  u8g2.setFont(u8g2_font_ncenB24_tr);
  u8g2.setCursor(10, 50);
  u8g2.print(value, 1);
}

void setup(void) {
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(10, 0);
  digitalWrite(9, 0);
  pinMode(JOY_BUTTON_PIN, INPUT_PULLUP);
  u8g2.begin();
  dht.begin();
  Serial.begin(9600);
  u8g2_prepare();
}

void loop(void) {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  int joyX = analogRead(JOY_X_PIN);
  int joyY = analogRead(JOY_Y_PIN);
  int joyButton = digitalRead(JOY_BUTTON_PIN);

  if (joyButton == LOW) {
    currentState = (currentState == LOCKED) ? UNLOCKED : LOCKED;
    delay(200);
  }

  if (currentState == UNLOCKED) {
    if ((millis() - lastRefreshTime) > REFRESH_INTERVAL) {
      lastRefreshTime = millis();
      switch (displayMode) {
        case TEMPERATURE:
          displayMode = HUMIDITY;
          break;
        case HUMIDITY:
          displayMode = ANOTHER_SENSOR;
          break;
        case ANOTHER_SENSOR:
          displayMode = TEMPERATURE;
          break;
      }
    }
  } else {
    if (joyButton == LOW) {
      if ((millis() - lastSwitchTime) > DEBOUNCE_DELAY) {
        lastSwitchTime = millis();
        switch (displayMode) {
          case TEMPERATURE:
            displayMode = HUMIDITY;
            break;
          case HUMIDITY:
            displayMode = ANOTHER_SENSOR;
            break;
          case ANOTHER_SENSOR:
            displayMode = TEMPERATURE;
            break;
        }
      }
    } else {
      if ((joyY > JOY_CENTER + JOY_DEADZONE) || (joyX > JOY_CENTER + JOY_DEADZONE)) {
        if ((millis() - lastSwitchTime) > DEBOUNCE_DELAY) {
          lastSwitchTime = millis();
          switch (displayMode) {
            case TEMPERATURE:
              displayMode = HUMIDITY;
              break;
            case HUMIDITY:
              displayMode = ANOTHER_SENSOR;
              break;
            case ANOTHER_SENSOR:
              displayMode = TEMPERATURE;
              break;
          }
        }
      } else if ((joyY < JOY_CENTER - JOY_DEADZONE) || (joyX < JOY_CENTER - JOY_DEADZONE)) {
        if ((millis() - lastSwitchTime) > DEBOUNCE_DELAY) {
          lastSwitchTime = millis();
          switch (displayMode) {
            case TEMPERATURE:
              displayMode = ANOTHER_SENSOR;
              break;
            case HUMIDITY:
              displayMode = TEMPERATURE;
              break;
            case ANOTHER_SENSOR:
              displayMode = HUMIDITY;
              break;
          }
        }
      }
    }
  }

  u8g2.firstPage();
  do {
    switch (displayMode) {
      case TEMPERATURE:
        draw_temperature(temperature);
        break;
      case HUMIDITY:
        draw_humidity(humidity);
        break;
      case ANOTHER_SENSOR:
        float sensorValue = 0.0;
        draw_another_sensor(sensorValue);
        break;
    }
  } while (u8g2.nextPage());

  if (millis() - lastSendTime > SEND_INTERVAL) {
    lastSendTime = millis();
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.println();
  }
}
