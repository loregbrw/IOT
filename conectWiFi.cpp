#include <WiFi.h>
#include "FirebaseESP32.h"

const char* WIFI_SSID = "celular da lorena";
const char* WIFI_PW = "12345678";

const char* FB_HOST = "https://iiot-mml-default-rtdb.firebaseio.com/";
const char* FB_KEY = "E0eirdwpbyFAoHge8Hw55XhkoQ0vOVkU8FK1A1n1";

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

  fbconfig.database_url = FB_HOST;
  fbconfig.signer.tokens.legacy_token = FB_KEY;
  fbdo.setBSSLBufferSize(4096, 1024);
  Firebase.reconnectWiFi(true);

  Firebase.begin(&fbconfig, &fbauth);
}

void loop()
{
  float value = 3.14;

  bool status = Firebase.setFloat(fbdo, "/iiot-mml/valor", value);

  if(!status)
  {
    Serial.println(fbdo.errorReason().c_str());
  }

  delay(5000);
}