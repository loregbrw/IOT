#include <Arduino.h>
#include <WiFi.h>
#include <firebaseESP32.H>
#include <PubSubClient.h>

const uint8_t PIN_LEDS[] = {14, 25, 26, 27};
const uint8_t PIN_NTC01 = 32;
const uint8_t PIN_NTC02 = 33;
const uint8_t PIN_BUTTON = 12;

const uint32_t ADC_MAX = (1 << 12) - 1;
const float VREF = 3.3f;
const float R1 = 10000;

const char* WIFI_SSID = "celular da lorena";
const char* WIFI_PW = "12345678";
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

bool click = false;

bool connectWiFi(const char* ssid, const char* pw)
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    int qtde_wifi = WiFi.scanNetworks();
    if(qtde_wifi == 0)
    {
        return false;
    }
    WiFi.begin(WIFI_SSID,WIFI_PW);
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

void readData()
{
    digitalWrite(PIN_LEDS[0], 1);

    uint16_t input_value_ntc01 = analogRead(PIN_NTC01);
    uint16_t input_value_ntc02 = analogRead(PIN_NTC02);

    float voltage_ntc01 = input_value_ntc01 * VREF / ADC_MAX;
    float voltage_ntc02 = input_value_ntc02 * VREF / ADC_MAX;

    float resistence_ntc01 = R1 * voltage_ntc01 / (VREF - voltage_ntc01);
    float resistence_ntc02 = R1 * voltage_ntc02 / (VREF - voltage_ntc02);

    float a = -19.49123972;
    float b = 204.88328885;
    float c = 208.78577114;

    float temperature_01 = a * log(resistence_ntc01 - b) + c;
    float temperature_02 = a * log(resistence_ntc02 - b) + c;

    float mean_temperature = (temperature_01 + temperature_02) / 2;
    float dev_temperature = abs(temperature_01 - temperature_02) / sqrt(2);

    if (mean_temperature >= 27)
    {
        for (int i = 1; i <= 3; i++)
        {
            digitalWrite(PIN_LEDS[i],  1);
        }
    }
    else if (mean_temperature < 27 && mean_temperature >= 24)
    {
        digitalWrite(PIN_LEDS[1],  1);
        digitalWrite(PIN_LEDS[2],  1);
        digitalWrite(PIN_LEDS[3],  0);
    }
    else if (mean_temperature < 24 && mean_temperature >= 21)
    {
        digitalWrite(PIN_LEDS[1],  1);
        digitalWrite(PIN_LEDS[2],  0);
        digitalWrite(PIN_LEDS[3],  0);
    }
    else
    {
        for (int i = 1; i <= 3; i++)
        {
            digitalWrite(PIN_LEDS[i],  0);
        }
    }

    Serial.printf("\nTemperature 01: %.2f & Temperature 02: %.2f\nMean Temperature: %.2f & Dev Temperature: %.2f\n\n", temperature_01, temperature_02, mean_temperature, dev_temperature);

    FirebaseJson json;
    json.set("/temperatura_media", mean_temperature);
    json.set("/desvio_padrao", dev_temperature);

    if (Firebase.updateNode(fbdo, "/avaliacao/subsys_11", json))
      Serial.println("Dado salvo com sucesso!");
    else
    {
        Serial.print("Falha: ");
        Serial.println(fbdo.errorReason().c_str());
    }

    digitalWrite(PIN_LEDS[0], 0);
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

    if(strcmp(topic, "iiot-dta/request") == 0 && strcmp(message, "yey") == 0)
    {
        readData();
        mqtt_client.publish("iiot-dta/request", "Dado salvo com sucesso!");
    }
}

void setup()
{
    Serial.begin(115200);

    for(auto pin : PIN_LEDS)
    {
        pinMode(pin, OUTPUT);
    }
    pinMode(PIN_NTC01, INPUT);
    pinMode(PIN_NTC02, INPUT);
    pinMode(PIN_BUTTON, INPUT);
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
    click = digitalRead(PIN_BUTTON);

    if (click)
    {
        readData();
    }
    mqtt_client.loop();
}