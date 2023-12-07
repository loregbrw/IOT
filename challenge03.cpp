#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <firebaseESP32.H>
#include <PubSubClient.h>

const uint8_t PIN_BUTTON = 25;
const uint8_t PIN_DHT = 26;
const uint8_t PIN_BUZZER = 27;
const uint8_t PIN_NTC1 = 32;
const uint8_t PIN_LDR = 33;
const uint8_t PIN_NTC2 = 35;

const uint32_t ADC_MAX = (1 << 12) - 1;
const float VREF = 3.3f;
const float R1 = 10000;

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

FirebaseData fbdo;
FirebaseAuth fbauth;
FirebaseConfig fbconfig;

long current_time = 0;
long current_time_led = 0;
bool click, run = false;

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
    if(strcmp(topic, "iiot-dta/check") == 0 && strcmp(message, "100") == 0)
    {
        mqtt_client.publish("iiot-dta/check", "1 do murylo");
        digitalWrite(PIN_LED, true);
        current_time_led = millis();
    }
    else if(strcmp(topic, "iiot-dta/request") == 0 && strcmp(message, "100") == 0)
        readData();
}

void readData()
{
    float humidity = dht.readHumidity();
    float temperature_dht = dht.readTemperature();

    uint16_t input_value_ntc1 = analogRead(PIN_NTC1);
    uint16_t input_value_ntc2 = analogRead(PIN_NTC2);
    uint16_t input_value_ldr = analogRead(PIN_LDR);

    float voltage_ntc1 = input_value_ntc1 * VREF / ADC_MAX;
    float voltage_ntc2 = input_value_ntc2 * VREF / ADC_MAX;
    float voltage_ldr = input_value_ldr * VREF / ADC_MAX;

    float resistence_ntc1 = R1 * voltage_ntc1 / (VREF - voltage_ntc1);
    float resistence_ntc2 = R1 * voltage_ntc2 / (VREF - voltage_ntc2);
    float resistence_ldr = R1 * voltage_ldr / (VREF - voltage_ldr);

    float a = -19.49123972;
    float b = 204.88328885;
    float c = 208.78577114;

    float temperature_ntc01 = a * log(resistence_ntc1 - b) + c;
    float temperature_ntc02 = a * log(resistence_ntc2 - b) + c;
    float luminosity = voltage_ldr;

    if(isnan(humidity) || isnan(temperature_dht))
    {
        Serial.println("Falha na leitura do sendor DHT");
        return;
    }

    if(temperature_dht > 30)
    {
        mqtt_client.publish("iiot-dta/request", "10 do murylo");
        //buzzer
    }

    Serial.printf("Humidity: %4.2f\nTemperature_dht: %7.2f\n\n", humidity, temperature_dht);

    FirebaseJson json;
    json.set("/time", millis());
    json.set("/luminosity", luminosity);
    json.set("/humidity", humidity);
    json.set("/temperature_dht", temperature_dht);
    json.set("/temperature_ntc01", temperature_ntc01);
    json.set("/temperature_ntc02", temperature_ntc02);

    if (Firebase.updateNode(fbdo, "/challenge03/subsys_11", json))
      Serial.println("Dado salvo com sucesso!");
    else
    {
        Serial.print("Falha: ");
        Serial.println(fbdo.errorReason().c_str());
    }
}

void setup()
{
  Serial.begin(115200); //inicializa comunicação via USB, e passa o valor da velocidade da comunicação 

  dht.begin();
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_NTC1, INPUT);
  pinMode(PIN_NTC2, INPUT);
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
    if(millis() - current_time > 1000)
    {
        readData();
        current_time = millis();
    }
    if(millis() - current_time_led > 5000)
        digitalWrite(PIN_LED, false);

    click = digitalRead(PIN_BUTTON);
    if (click)
    {
        /* code */
    }



    mqtt_client.loop();
}
