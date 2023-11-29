#include <WiFi.h>
#include "FirebaseESP32.h"
#include <cmath>

const uint8_t PIN_TEMP[] = {33, 32};
const uint8_t PIN_LIGHT = 34;

const uint32_t ADC_MAX = (1 << 12) - 1;
const float VREF = 3.3f;
const float R1 = 10000;

const char* WIFI_SSID = "celular da lorena";
const char* WIFI_PW = "12345678";

const char* FB_HOST = "https://iiot-dta-default-rtdb.firebaseio.com";
const char* FB_KEY = "Ag5gJMhAnTWQgDVhegkDRF1uTjJfpMUDkXB8WBEa";

FirebaseData fbdo;
FirebaseAuth fbauth;
FirebaseConfig fbconfig;

bool connectWiFi(const char* ssid, const char* pw)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  WiFi.begin(ssid, pw);

  Serial.print("Conectado!");
  int tentativas = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    tentativas++;

    if (tentativas > 300)
    {
      return false;
    }

    Serial.print(".");
    delay(200);
  }

  Serial.print(" Conectado com o IP: !");
  Serial.println(WiFi.localIP());

  return true;
}

void setup()
{
  Serial.begin(115200);
  connectWiFi(WIFI_SSID, WIFI_PW);

  pinMode(PIN_TEMP[0], INPUT);
  pinMode(PIN_TEMP[1], INPUT);
  pinMode(PIN_LIGHT, INPUT);

  fbconfig.database_url = FB_HOST;
  fbconfig.signer.tokens.legacy_token = FB_KEY;
  fbdo.setBSSLBufferSize(4096, 1024);
  Firebase.reconnectWiFi(true);

  Firebase.begin(&fbconfig, &fbauth);
}

void loop()
{
  uint16_t input_value1 = analogRead(PIN_TEMP[0]);
  uint16_t input_value2 = analogRead(PIN_TEMP[1]);
  uint16_t input_value3 = analogRead(PIN_LIGHT);

  float voltage1 = input_value1 * VREF / ADC_MAX;
  float voltage2 = input_value2 * VREF / ADC_MAX;
  float voltage3 = input_value3 * VREF / ADC_MAX;

  float resistence1 = R1 * voltage1 / (VREF - voltage1);
  float resistence2 = R1 * voltage2 / (VREF - voltage2);
  float resistence3 = R1 * voltage3 / (VREF - voltage3);

  float a = -19.49123972;
  float b = 204.88328885;
  float c = 208.78577114;

  float temp1 = a * log(resistence1 - b) + c;
  float temp2 = a * log(resistence2 - b) + c;
  float light = voltage3;

  FirebaseJson json;
  bool status;

  json.set("/subsys_12/temperature/temp_sensor_00", temp1);
  json.set("/subsys_12/temperature/temp_sensor_01", temp2);
  json.set("/subsys_12/luminosity", light);
  status = Firebase.updateNode(fbdo, "iiot-dta", json);

  if(!status)
  {
    Serial.println(fbdo.errorReason().c_str());
  }

  delay(5000);
}