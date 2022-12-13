#include <ESP8266WiFi.h>

void setup() {
  pinMode(D6, OUTPUT);  
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
}

void loop() {
  digitalWrite(D6, HIGH);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);

  digitalWrite(D6, LOW);
  digitalWrite(D7, HIGH);
  digitalWrite(D8, LOW);

  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, HIGH);

  delay(2000);
}
