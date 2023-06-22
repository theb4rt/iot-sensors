int redPin = 44;    // Pin connected to the red channel of the RGB LED
int greenPin = 45; // Pin connected to the green channel of the RGB LED
int bluePin = 46;  // Pin connected to the blue channel of the RGB LED
int delayTime = 50; // Delay between color changes

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  // Activate red and turn off green and blue (simulate emergency red light)
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
  delay(delayTime);
  
  // Activate red and blue (simulate transitioning to red and blue lights)
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 255);
  delay(delayTime);
  
  // Activate blue and turn off red (simulate emergency blue light)
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 255);
  delay(delayTime);
  
  // Activate blue and green (simulate transitioning to blue and green lights)
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 255);
  delay(delayTime);
}