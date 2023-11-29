#include <Arduino.h>
#include <cmath>

const uint8_t PIN_TEMP[] = {33, 27};
const uint8_t PIN_LIGHT = 34;

const uint32_t ADC_MAX = (1 << 12) - 1;
const float VREF = 3.3f;
const float R1 = 10000;

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_TEMP[0], INPUT);
    pinMode(PIN_TEMP[1], INPUT);
    pinMode(PIN_LIGHT, INPUT);
}

void loop()
{
    uint16_t input_value1 = analogRead(PIN_TEMP[0]);
    uint16_t input_value2 = analogRead(PIN_TEMP[1]);
    uint16_t input_value3 = analogRead(PIN_LIGHT);

    float voltage1 = input_value1 * VREF / ADC_MAX;
    float voltage2 = input_value2 * VREF / ADC_MAX;
    float voltage3 = input_value3 * VREF / ADC_MAX;

    float resistence1 = R1 * voltage / (VREF - voltage1);
    float resistence2 = R1 * voltage / (VREF - voltage2);
    float resistence3 = R1 * voltage / (VREF - voltage3);

    float a = -19.49123972;
    float b = 204.88328885;
    float c = 208.78577114;

    float temp1 = a * log(resistence - b) + c;
    float temp2 = a * log(resistence - b) + c;
    float light = voltage3;

    Serial.printf("Temp1: %8.2f\t|\tTemp2: %8.2f\t|\tLight: %8.2f");

    delay(1000);
}