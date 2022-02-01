#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <TimeLib.h>
#include <DHT.h>
#include <Wire.h>
#include <ArduinoJSON.h>
#include <PubSubClient.h>
#include <BH1750.h>
#include <NTPClient.h>

#include "config.h"

#define DHTPIN 2
#define DHTTYPE DHT11

boolean luz = false;
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

void connectWiFi(char *ssid, char * pass);
void reconnectMQTTClient();
void createMQTTClient();
void switch_light();

WiFiClient rp2040Client;
PubSubClient client(rp2040Client);
WiFiUDP wifiUdp;
NTPClient timeClient(wifiUdp, "es.pool.ntp.org", 1 * 3600, 60000);  // Ajust for your location

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(2000);
  Serial.setTimeout(8000);
  // Conectar a una wifi conocida según los datos que pongamos:
  Serial.println("Datos de red wifi:");
  Serial.println("Nombre de red:");
  String ssidHoy;
  String passHoy;
//  if(Serial.available()){
    ssidHoy = Serial.readStringUntil('+');
//  }
  Serial.println("Contraseña:");
//  if(Serial.available()){
    passHoy = Serial.readStringUntil('+');
//  }
  char *ssid = new char[ssidHoy.length() + 1];
  char *pass = new char[passHoy.length() + 1];
  strcpy(ssid, ssidHoy.c_str());
  strcpy(pass, passHoy.c_str());


  createMQTTClient();
  Wire.begin();
  lightMeter.begin();
  dht.begin();
  timeClient.begin();
  timeClient.update();

  delay(2000);
}

void loop() {
  reconnectMQTTClient();
  client.loop();
  IPAddress ip;
  ip = WiFi.localIP();
  Serial.println(ip);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float light = lightMeter.readLightLevel();
  timeClient.update();
  String timeNTP = timeClient.getFormattedTime();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  DynamicJsonDocument doc(1024);
  doc["hora"] = timeNTP;
  doc["luz"] = light;
  doc["temperatura"] = t;
  doc["humedad"] = h;

  string telemetry;
  JsonObject obj = doc.as<JsonObject>();
  serializeJson(obj, telemetry);
  Serial.print("Enviando telemetría ");
  Serial.println(telemetry.c_str());
  client.publish(CLIENT_TELEMETRY_TOPIC.c_str(), telemetry.c_str());

  delay(60000*10);

}

void switch_light() {
  luz = !luz;
  digitalWrite(LED_BUILTIN, luz);
}

void connectWiFi(char *ssid, char *pass)
{
    Serial.println(ssid);
    Serial.println(pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connecting to WiFi..");
        WiFi.begin(ssid, pass);
        delay(1500);
    }

    Serial.println("Connected!");
    Serial.println(WiFi.localIP());
}

void reconnectMQTTClient()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");

        if (client.connect(CLIENT_NAME.c_str()))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("Retying in 5 seconds - failed, rc=");
            Serial.println(client.state());

            delay(5000);
        }
    }
}

void createMQTTClient()
{
    client.setServer(BROKER.c_str(), 1883);
    reconnectMQTTClient();
}
