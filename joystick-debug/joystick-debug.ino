const int X_PIN = A0; // Joystick X-axis pin
const int Y_PIN = A1; // Joystick Y-axis pin
const int BUTTON_PIN = 52; // Joystick button pin

void setup() {
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  // Read the joystick values
  int xValue = analogRead(X_PIN);
  int yValue = analogRead(Y_PIN);
  int buttonState = digitalRead(BUTTON_PIN);

  // Print the joystick values to the Serial Monitor
  Serial.print("X: ");
  Serial.print(xValue);
  Serial.print("\tY: ");
  Serial.print(yValue);
  Serial.print("\tButton: ");
  Serial.println(buttonState);

  delay(100); // Delay for smooth serial output
}







