#include <Arduino.h>

const float VREF = 3.3f;
const uint16_t ADCMAX = 4095;
const uint8_t PIN_INPUT = 12;
const uint8_t PINOS_LED[6] = {32,33,25,26,27,14};

void setup()
{
  pinMode(PIN_INPUT, INPUT);

  for(auto pino : PINOS_LED)
    pinMode(pino, OUTPUT);

  Serial.begin(115200);
}

void loop()
{
  uint16_t input_value = analogRead(PIN_INPUT);
  float voltage = input_value * VREF / ADCMAX;

  Serial.println(voltage);

  digitalWrite(PINOS_LED[0], voltage > 0.5);
  digitalWrite(PINOS_LED[1], voltage > 1);
  digitalWrite(PINOS_LED[2], voltage > 1.5);
  digitalWrite(PINOS_LED[3], voltage > 2);
  digitalWrite(PINOS_LED[4], voltage > 2.5);
  digitalWrite(PINOS_LED[5], voltage > 3);
}