#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <firebaseESP32.H>
#include <PubSubClient.h>

const uint8_t PIN_INPUT = 33;
const uint8_t PIN_LED = 27;

const char* WIFI_SSID = "Vivo-Internet-BF17";
const char* WIFI_PW = "78814222";
const char* FB_HOST = "https://iiot-dta-default-rtdb.firebaseio.com";
const char* FB_KEY = "Ag5gJMhAnTWQgDVhegkDRF1uTjJfpMUDkXB8WBEa";

const char* MQTT_BROKER = "test.mosquitto.org";
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "";
const char* MQTT_PASSWORD = "";

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

#define DHTTYPE DHT11
DHT dht(PIN_INPUT, DHTTYPE);

FirebaseData fbdo;
FirebaseAuth fbauth;
FirebaseConfig fbconfig;

bool connectWiFi(const char*, const char*);
void callback(char*, byte*, unsigned int);
void readData();

long current_time = 0;
long current_time_led = 0;


void setup()
{
  Serial.begin(115200); //inicializa comunicação via USB, e passa o valor da velocidade da comunicação 
  
  dht.begin();
  pinMode(PIN_LED, OUTPUT);
  connectWiFi(WIFI_SSID, WIFI_PW);

  mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_client.setCallback(callback);

  fbconfig.database_url = FB_HOST;
  fbconfig.signer.tokens.legacy_token = FB_KEY; 
  fbdo.setBSSLBufferSize(4096, 1024);
  Firebase.reconnectWiFi(true);
  Firebase.begin(&fbconfig, &fbauth);

  while (!mqtt_client.connected())
  {
    String client_id = "mqttx_dta_esp_subsys_05";
    client_id += String(WiFi.macAddress());

    if (mqtt_client.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD))
      Serial.println("Conexao MQTT bem sucedida");
  }
  mqtt_client.subscribe("iiot-dta/check");
  mqtt_client.subscribe("iiot-dta/request");
}



void loop()
{
  if(millis() - current_time > 30000)
  {
    readData();
    current_time = millis();
  }

  if(millis() - current_time_led > 5000)
    digitalWrite(PIN_LED, false);

  mqtt_client.loop();
}




bool connectWiFi(const char* ssid, const char* pw)
{
  WiFi.mode(WIFI_STA); //modo estático, não muda o IP
  WiFi.disconnect();

  int qtde_wifi = WiFi.scanNetworks(); //retorna um int com o número de rede disponíveis

  if(qtde_wifi == 0)
  {
    return false;
  }

  WiFi.begin(WIFI_SSID,WIFI_PW); //iniciliza uma rede Wifi

  Serial.print("Conectando");
  int tentativa = 0;

  while(WiFi.status() != WL_CONNECTED)
  {
    tentativa ++;

    if (tentativa > 300)
    {
      return false;
    }

    Serial.print(".");
    delay(200);
  }
  Serial.print("Conectado com o IP: ");
  Serial.println(WiFi.localIP());

  return true;
}


void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.printf("Mensagem recebida no topico %s: ", topic);

  char message[length + 1];

  for(int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  Serial.println();

  if(strcmp(topic, "iiot-dta/check") == 0)
  {
    if(strcmp(message, "100") == 0)
      mqtt_client.publish("iiot-dta/check", "1 do murylo");
    else if(strcmp(message, "200") == 0)
    {
      digitalWrite(PIN_LED, true);
      current_time_led = millis();
    }
  }
  else if(strcmp(topic, "iiot-dta/request") == 0 && strcmp(message, "100") == 0)
    readData();
}


void readData()
{
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  if(isnan(umidade) || isnan(temperatura))
  {
    Serial.println("Falha na leitura do sendor DHT");
    return;
  }

  if(temperatura > 30)
    mqtt_client.publish("iiot-dta/request", "10 do murylo");

  Serial.printf("Humidade: %4.2f\nTemperatura: %7.2f\n\n", umidade, temperatura);

  FirebaseJson json;
  json.set("/humidity", umidade);
  json.set("/temperature", temperatura);

  if (Firebase.updateNode(fbdo, "/challenge02/subsys_11", json))
    Serial.println("Dado salvo com sucesso!");
  else
  {
    Serial.print("Falha: ");
    Serial.println(fbdo.errorReason().c_str());
  }
}