/*
  Arduino Code
  Created on: 09/06/2023
  Author: b4rt
  Email: root.b4rt@gmail.com
*/


#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <DHT.h>

//Defining PINS NUMBER
#define DHT_PIN 2 //kY-015 DATA PIN
#define JOY_X_PIN A7
#define JOY_Y_PIN A6
#define JOY_BUTTON_PIN 52

//Settings
#define JOY_CENTER 512 //limit center
#define JOY_DEADZONE 100 // deadzone for joystick performance
#define DEBOUNCE_DELAY 200  //Delay for avoid miss clicks on joystick
#define REFRESH_INTERVAL 3000 //Refresh interval for OLED
//#define SEND_INTERVAL 5000  //Sending interval from data to RasPi
#define BUFFER_SIZE 12      // buffer before seeding data
#define ALERT_THRESHOLD 26.0  // limit temperature to send alert
#define BATCH_INTERVAL 60000  //time to send the block of data to RasPi - 60s

U8G2_SSD1327_MIDAS_128X128_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
DHT dht(DHT_PIN, DHT11);

//Display views
enum DisplayMode {
    TEMPERATURE,
    HUMIDITY,
    ANOTHER_SENSOR
};

//state of display LOCKED:Views changing automatically - Joystick disabled, UNLOCKED: Views locked - Joystick enabled
enum State {
    LOCKED,
    UNLOCKED
};

//Data sent to RasPi
struct SensorData {
    float temperature;
    float humidity;
    unsigned long timestamp;
};

//Init values
DisplayMode displayMode = TEMPERATURE;
State currentState = LOCKED;
unsigned long lastSwitchTime = 0;
unsigned long lastRefreshTime = 0;
unsigned long lastSendTime = 0;
SensorData buffer[BUFFER_SIZE];
int bufferIndex = 0;
bool alertFlag = false;

//Setting format from Oled display
void u8g2_prepare(void) {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
}

//Drawing views
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

void add_to_buffer(float temperature, float humidity) {
    if (bufferIndex < BUFFER_SIZE) {
        buffer[bufferIndex].temperature = temperature;
        buffer[bufferIndex].humidity = humidity;
        buffer[bufferIndex].timestamp = millis();
        bufferIndex++;
    }
}

void check_alert(float temperature) {
    if (temperature > ALERT_THRESHOLD) {
        alertFlag = true;
    }
}

void send_buffer() {
    DynamicJsonDocument doc(1024); // Json 1024Bytes

    for (int i = 0; i < bufferIndex; i++) {
        JsonObject sensor = doc.createNestedObject();
        sensor["timestamp"] = buffer[i].timestamp;
        sensor["temperature"] = buffer[i].temperature;
        sensor["humidity"] = buffer[i].humidity;
    }

    serializeJson(doc, Serial);
    Serial.println();
    bufferIndex = 0;
}
void send_alert(float temperature) {
  DynamicJsonDocument doc(1024);
  doc["alert"] = "Critical value";
  doc["sensor"] = "temperature";
  doc["value"] = temperature;
  serializeJson(doc, Serial);
  Serial.println();
  alertFlag = false;
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


//Launching main loop
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

    //If state is unlocked, joystick is disabled
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
    // If state is locked, joystick is enabled
    } else {
        //If joystick is pressed, Lock Views
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
            //If joystick is not pressed, change views Right/Left
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

    //TODO
    //check another methods or libs for drawing u8g2
    //this loop should be optimized
    //is the only way to work with u8g2?
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

    add_to_buffer(temperature, humidity);
    check_alert(temperature);

    //Send data to serial if buffer is full or time is up
    if (millis() - lastSendTime > BATCH_INTERVAL) {
        lastSendTime = millis();
        send_buffer();
    }

    if (alertFlag) {
        send_alert(temperature);
    }
}
