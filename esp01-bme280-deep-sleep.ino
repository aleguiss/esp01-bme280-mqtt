// ESP-01 with BME-280 via MQTT with deep sleep
// Based on https://github.com/akkerman/espthp
// link https://glsk.net/2018/02/battery-powered-weather-station-with-esp8266-and-bme280/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Ticker.h>
#include "config.h"

#define STATUS_DISCONNECTED "disconnected"
#define STATUS_ONLINE "online"

#define WIFI_SSID "<wifi-name>"
#define WIFI_PASS "<wifi-password>"
#define MQTT_IP "192.168.0.107"
#define MQTT_PORT 1883

#define DEEP_SLEEP_S 900  // 900 s = 15 min.

#define BME280ON  // Comment to disable BME280 sensor readout.

//#define DEBUG  // Comment to disable debug serial output.
#ifdef DEBUG
  #define DPRINT(...)    Serial.print(__VA_ARGS__)
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINTLN(...)
#endif

ADC_MODE(ADC_VCC);

unsigned long previousMillis = 0;
const long interval = 10 * 60 * 1000;

const String chipId = String(ESP.getChipId());
const String hostName = "ESP-01_" + chipId;
const String baseTopic = "raw/" + chipId + "/";
const String tempTopic = baseTopic + "temperature";
const String humiTopic = baseTopic + "humidity";
const String presTopic = baseTopic + "pressure";
const String willTopic = baseTopic + "status";
const String vccTopic  = baseTopic + "vcc";
const String ipTopic   = baseTopic + "ip";

IPAddress ip;

WiFiClient WiFiClient;
PubSubClient client(WiFiClient);
#ifdef BME280ON
  Adafruit_BME280 bme; // I2C
#endif
Ticker ticker;

char temperature[6];
char humidity[6];
char pressure[7];
char vcc[10];

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#else
  pinMode(3, OUTPUT);  // Power up BME280 when powered via RX (GPIO3).
  digitalWrite(3, HIGH);
#endif
  delay(10);

#ifdef BME280ON
  Wire.begin(0, 2);
  Wire.setClock(100000);
  if (!bme.begin(0x76)) {
    DPRINTLN("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
#endif

  DPRINTLN();
  DPRINT("Connecting to ");
  DPRINTLN(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostName);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DPRINT(".");
  }

  DPRINTLN();
  DPRINTLN("WiFi connected");
  DPRINT("IP address: ");
  ip = WiFi.localIP();
  DPRINTLN(ip);

  client.setServer(MQTT_IP, MQTT_PORT);
  client.setCallback(callback);
  ticker.attach(600.0, publish);

  bool publishNow = false;

  yield();
  if (!client.connected()) {
    DPRINTLN("Attempting MQTT connection...");
    if (client.connect(chipId.c_str(), willTopic.c_str(), 1, true, STATUS_DISCONNECTED)) {
      DPRINTLN("MQTT client connected.");
      client.publish(willTopic.c_str(), STATUS_ONLINE, true);
      client.publish(ipTopic.c_str(), ip.toString().c_str(), true);
      client.subscribe("config/publish");
      publishNow = true;
    } else {
      DPRINT("failed, rc=");
      DPRINTLN(client.state());
    }
  }

  bmeRead();
  vccRead();

#ifndef DEBUG
  digitalWrite(3, LOW);
#endif

  if (client.connected()) {
    client.loop();
    if (publishNow) {
      publish();
      publishNow = false;
    }
  }

  client.disconnect();

  delay(500); // Needed to properly close connection.

  WiFi.disconnect();

  DPRINTLN("Going to deep sleep...");
  ESP.deepSleep(DEEP_SLEEP_S * 1000 * 1000);
}


void loop() {
}

void bmeRead() {
#ifdef BME280ON
  float t = bme.readTemperature();
  float h = bme.readHumidity();
  float p = bme.readPressure() / 100.0F;
#else
  float t = 20 + random(5);
  float h = 50 + random(10);
  float p = 1013 + random(10);
#endif

  dtostrf(t, 5, 1, temperature);
  dtostrf(h, 5, 1, humidity);
  dtostrf(p, 5, 1, pressure);
}

void printMeasurements() {
  DPRINT("t,h,p: ");
  DPRINT(temperature); DPRINT(", ");
  DPRINT(humidity);    DPRINT(", ");
  DPRINT(pressure);    DPRINTLN();
  DPRINT("vcc:   ");   DPRINTLN(vcc);
}

void publish() {
  printMeasurements();

  if (client.connected()) {
    DPRINTLN("publishing values");
    DPRINTLN(baseTopic);
    client.publish(tempTopic.c_str(), temperature);
    client.publish(humiTopic.c_str(), humidity);
    client.publish(presTopic.c_str(), pressure);
    client.publish(vccTopic.c_str(), vcc);
  } else {
    DPRINTLN("client not connected, not publishing");
  }
}

void vccRead() {
  float v  = ESP.getVcc() / 1000.0;
  dtostrf(v, 5, 2, vcc);
}

void callback(char* topic, byte* payload, unsigned int length) {
  std::string s( reinterpret_cast<char const*>(payload), length );
  DPRINT("message arrived: ");
  DPRINT(topic);
  DPRINT(" - ");
  DPRINT(s.c_str());
  DPRINTLN();
  if (s == "all" || s == chipId.c_str()) {
    publish();
  }
}
