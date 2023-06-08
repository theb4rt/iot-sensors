#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to the Arduino digital pin 2
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup(void)
{
  // Start serial communication for debugging purposes
  Serial.begin(9600);
  // Start up the library
  sensors.begin();
}

void loop(void)
{ 
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  
  // Print the temperature in Celsius
  Serial.print("Temperature : ");
  Serial.print(sensors.getTempCByIndex(0)); 
  Serial.println("°C");
  
  // Wait 1 second before next reading
  delay(1000);
}