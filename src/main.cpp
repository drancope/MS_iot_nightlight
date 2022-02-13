#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
//#include <Time.h>
#include <TimeLib.h>
#include <DHT.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <BH1750.h>
#include <NTPClient.h>
#include <SFE_BMP180.h>

#include "config.h"

#define DHTPIN 2
#define DHTTYPE DHT11
#define ALTITUDE 840.0


boolean luz = false;
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
SFE_BMP180 pressureSensor;

void connectWiFi(char *ssid, char *pass);
void reconnectMQTTClient();
void createMQTTClient(char *broker);
void switch_light();
float readPressureFromSensor();

void callback(char* topic, byte* payload, unsigned int length) {
  ;
}

WiFiClient rp2040Client;
PubSubClient client(BROKER.c_str(), 1883, callback, rp2040Client);
WiFiUDP wifiUdp;
NTPClient timeClient(wifiUdp, "es.pool.ntp.org", 1 * 3600, 60000);  // Ajust for your location

void setup() {
  String servidorMqtt;
  char *broker;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(3, INPUT_PULLDOWN);
  Serial.begin(115200);
  delay(4000);
  if(digitalRead(3)) {
    Serial.setTimeout(8000);
    // Conectar a una wifi conocida según los datos que pongamos:
    Serial.println("Datos de red wifi:");
    Serial.println("Nombre de red:");
    String ssidHoy;
    String passHoy;
    ssidHoy = Serial.readStringUntil('\r');
    Serial.read(); //damn LF after CR
    Serial.println("Contraseña:");
    passHoy = Serial.readStringUntil('\r');
    Serial.read(); //damn LF after CR
    char *ssid = new char[ssidHoy.length() + 1];
    char *pass = new char[passHoy.length() + 1];
    strcpy(ssid, ssidHoy.c_str());
    strcpy(pass, passHoy.c_str());
    connectWiFi(ssid, pass);
    broker = new char[BROKER.length() +1];
    strcpy(broker, BROKER.c_str());
  } else {
    Serial.println("Datos de servidor MQTT: ");
    servidorMqtt = Serial.readStringUntil('\r');
    Serial.read(); //damn LF after CR
    char *ssid = new char[SSID.length() +1];
    char *pass = new char[PASSWORD.length() +1];
    broker = new char[servidorMqtt.length() +1];
    strcpy(ssid, SSID.c_str());
    strcpy(pass, PASSWORD.c_str());
    strcpy(broker, servidorMqtt.c_str());
    connectWiFi(ssid, pass);
  }
  createMQTTClient(broker);
  Wire.begin();
  lightMeter.begin();
  dht.begin();
  pressureSensor.begin();
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
  uint16_t light = lightMeter.readLightLevel();
  float presion = readPressureFromSensor();
  timeClient.update();
  String timeNTP = timeClient.getFormattedDate();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  DynamicJsonDocument doc(1024);
  doc["hora"] = timeNTP;
  doc["luz"] = light;
  doc["temperatura"] = t;
  doc["humedad"] = h;
  doc["presion"] = presion;

  string telemetry;
  JsonObject obj = doc.as<JsonObject>();
  serializeJson(obj, telemetry);
  Serial.print("Enviando telemetría a ");
  Serial.print(BROKER.c_str());
  Serial.print(": ");
  Serial.println(telemetry.c_str());
  client.publish(CLIENT_TELEMETRY_TOPIC.c_str(), telemetry.c_str());

  int milis = millis();
  while (millis() -milis < 600000) {
      switch_light();
      delay(1000);
  }
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
        Serial.print("Attempting MQTT connection to ");
        Serial.print(BROKER.c_str());
        if (client.connect(CLIENT_NAME.c_str()))
        {
            Serial.println("  ...connected");
        }
        else
        {
            Serial.print(" Retying in 5 seconds - failed, rc=");
            Serial.println(client.state());

            delay(5000);
        }
    }
}

void createMQTTClient(char *broker)
{
    client.setServer(broker, 1883);
    reconnectMQTTClient();
}

float readPressureFromSensor()
{
  char status;
  double T, P, p0;

  status = pressureSensor.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = pressureSensor.getTemperature(T);
    if (status != 0)
    {
      status = pressureSensor.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = pressureSensor.getPressure(P,T);
        if (status != 0)
        {
          p0 = pressureSensor.sealevel(P,ALTITUDE);
          return p0;
        }
      }
    }
  }
}
