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
#define BATCH_INTERVAL 10000  //time to send the block of data to RasPi - 60s

// Read data from sensors every X seconds
unsigned long lastReadTime = 0;
#define READ_INTERVAL 2000

//LED RGB PIN 
//RGB led check pins
#define LED_BLUE 44  // BLUE
#define LED_RED 45    // RED
#define LED_GREEN 46 // GREEN


// Defining OLED display settings
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

float temperature = 0.0;
float humidity = 0.0;

float alert_threshold_temperature = 29.0; //Critical value for temperature
float alert_threshold_humidity = 90.0; //Critical value for humidity
bool led_state = false; //LED status - true: ON, false: OFF

bool testMode = false;
int actualLedOn = 99; //turn on certain color of LED


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

    if (alert_threshold_temperature < temperature) {
        alertFlag = true;
        controlRGBLED(2);
        return 0;

    }

    if (alert_threshold_temperature - 1 <= temperature && temperature <= alert_threshold_temperature) {

        controlRGBLED(6);
        return 0;

    }

    if (temperature < alert_threshold_temperature - 1) {

        controlRGBLED(99);
        return 0;
    }


}


void send_buffer() {
    DynamicJsonDocument doc(1024); //Json 1024

    for (int i = 0; i < bufferIndex; i++) {
        JsonObject sensor = doc.createNestedObject();
        sensor["temperature"] = buffer[i].temperature;
        sensor["humidity"] = buffer[i].humidity;
    }
    serializeJson(doc, Serial);
    Serial.println();
    bufferIndex = 0;
}

void send_alert(float temperature) {
    DynamicJsonDocument doc(1024);
    doc["type"] = "alert";
    doc["alert"] = "Critical value";
    doc["sensor"] = "temperature";
    doc["value"] = temperature;
    serializeJson(doc, Serial);
    Serial.println();
    alertFlag = false;
}

void send_actual_data() {
    DynamicJsonDocument doc(1024);
    doc["type"] = "values";
    doc["actual_led_on"] = actualLedOn;
    doc["test_mode_status"] = testMode;
    doc["actual_threshold_temperature"] = alert_threshold_temperature;
    serializeJson(doc, Serial);
    Serial.println();
}


void turnLEDOff() {
    controlRGBLED(99);
    //isLEDOn = false;
}


void controlRGBLED(int colorState) {
    if (colorState >= 0 && colorState <= 9) {
        switch (colorState) {
            case 0:  // Violet1
                analogWrite(LED_RED, 255);
                analogWrite(LED_GREEN, 43);
                analogWrite(LED_BLUE, 255);
                break;
            case 1:
                // Violet2
                analogWrite(LED_RED, 138);
                analogWrite(LED_GREEN, 43);
                analogWrite(LED_BLUE, 226);
                break;
            case 2:  // Red
                analogWrite(LED_RED, 255);
                analogWrite(LED_GREEN, 0);
                analogWrite(LED_BLUE, 0);
                break;
            case 3:  //random
                analogWrite(LED_RED, 128);
                analogWrite(LED_GREEN, 128);
                analogWrite(LED_BLUE, 128);
                break;

            case 4:
                // Magenta
                analogWrite(LED_RED, 255);
                analogWrite(LED_GREEN, 0);
                analogWrite(LED_BLUE, 255);
                break;
            case 5:  // Yellow
                analogWrite(LED_RED, 255);
                analogWrite(LED_GREEN, 255);
                analogWrite(LED_BLUE, 0);
                break;
            case 6:  // Orange
                analogWrite(LED_RED, 255);
                analogWrite(LED_GREEN, 165);
                analogWrite(LED_BLUE, 0);
                break;
            case 7:  // Cyan
                analogWrite(LED_RED, 0);
                analogWrite(LED_GREEN, 255);
                analogWrite(LED_BLUE, 255);
                break;
            case 8: // Blue
                analogWrite(LED_RED, 0);
                analogWrite(LED_GREEN, 0);
                analogWrite(LED_BLUE, 255);
                break;
            case 9: // Green
                analogWrite(LED_RED, 0);
                analogWrite(LED_GREEN, 255);
                analogWrite(LED_BLUE, 0);
                break;
        }
    } else {
        analogWrite(LED_RED, 0);
        analogWrite(LED_GREEN, 0);
        analogWrite(LED_BLUE, 0);
    }
}


void setup(void) {

    //Joystick
    pinMode(JOY_BUTTON_PIN, INPUT_PULLUP);


    //OLED begin
    u8g2.begin();
    dht.begin();

    //Serial
    Serial.begin(9600);

    //LED RGB
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    //OLED init
    u8g2_prepare();
}


//Launching main loop
void loop(void) {

    if (Serial.available()) {
        String jsonData = Serial.readStringUntil('\n');
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, jsonData);
        if (error) {
            Serial.println("Not valid data");
            return;
        }
        const char *type = doc["type"];
        if (strcmp(type, "change_value") == 0) {
            if (doc.containsKey("alert_threshold_temperature")) {
                alert_threshold_temperature = doc["alert_threshold_temperature"];
            }

            if (doc.containsKey("alert_threshold_humidity")) {
                alert_threshold_humidity = doc["alert_threshold_humidity"];
            }

            if (doc.containsKey("led_alert")) {
                led_state = doc["led_alert"];
                if (led_state) {
                    controlRGBLED(0);
                } else {
                    controlRGBLED(99);
                }
            }

        } else if (strcmp(type, "get_actual_values") == 0) {
            send_actual_data();
//                if (doc.containsKey("ask_status_led")) {
//                   send_actual_data();
//              }

        }
    }

    // read the sensor every 2 seconds
    if (millis() - lastReadTime > READ_INTERVAL) {
        lastReadTime = millis();
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        add_to_buffer(temperature, humidity);

        if (!testMode) {
            check_alert(temperature);

            if (alertFlag) {
                send_alert(temperature);
            }
        }

    }
    //Readings from joystick
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



    //Send data to serial if buffer is full or time is up
    if (millis() - lastSendTime > BATCH_INTERVAL) {
        lastSendTime = millis();
        send_buffer();
    }


}
