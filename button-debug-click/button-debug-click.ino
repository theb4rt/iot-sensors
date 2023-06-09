const int BUTTON_PIN = 48; // Button pin

bool buttonState = HIGH;          // current state of the button
bool lastButtonState = HIGH;      // previous state of the button
unsigned long lastDebounceTime = 0; // last time the button state changed
unsigned long debounceDelay = 50;  // debounce delay in milliseconds

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);
  Serial.println(reading);
  

  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // reset the debounce timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        Serial.println("Button Pressed");
      }
    }
  }

  lastButtonState = reading;
  delay(2000);
}