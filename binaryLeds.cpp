#include <Arduino.h>

const uint8_t PINOS_LED[] = {25,26,27,14};
const uint8_t PINO_BUTTON = 33;

void setup()
{
  for(auto pino : PINOS_LED)
  {
    pinMode(pino, OUTPUT);
  }
  pinMode(PINO_BUTTON, INPUT);
}

long tempo = millis();
uint8_t current_count = 0;
bool click = false;
bool lastClick, run = false;

void loop()
{
  lastClick = click;
  click = digitalRead(PINO_BUTTON);

  if(click && !lastClick)
  {
    run = !run;
    current_count = 0;
  }

  if (run && millis() - tempo > 1000)
  {

    uint8_t current = current_count;

    for(int i = 3; i >= 0; i--)
    {
      digitalWrite(PINOS_LED[i],  current % 2);
      current /= 2;
    }

    current_count = current_count == 15 ? 0 : current_count + 1;
    tempo = millis();
  }

  else if (!run)
  {
    for (int i = 0; i < 4; i++)
    {
        digitalWrite(PINOS_LED[i],  0);
    }
  }
}