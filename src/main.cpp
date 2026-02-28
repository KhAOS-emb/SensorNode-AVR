#include <Arduino.h>

void setup() {
  Serial.begin(115200);     // UART starten, 115200 Baud
  Serial.println("SensorNode-AVR is up and running!");
}

void loop() {
  Serial.println("Heartbeat");
  delay(1000);
}