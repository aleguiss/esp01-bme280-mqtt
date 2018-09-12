/*
  Based on https://github.com/akkerman/espthp
    raw/chipId/status
    raw/chipId/temperature
    raw/chipId/humidity
    raw/chipId/pressure
    raw/chipId/vcc
  
  Reset pin is high (with 10-12k resistor) and pulled low for reset. ESP.GPIO0 is pulled low for programming.
  
  After uploading the sketch:
    ESP.GPIO0 to BME280.SDA
    ESP.GPIO2 to BME280.SCL
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Ticker.h>

#define wifi_ssid "<wifi-ssid>"
#define wifi_pass "<wifi-pass>"

#define mqtt_ip "<mqtt-ip>"
#define mqtt_user "<mqtt-user>"
#define mqtt_pass "<mqtt-pass>"
#define mqtt_port 1883

#define debug_mode = true // true of false to print connection info

ADC_MODE(ADC_VCC);

const String chipId = String(ESP.getChipId());
const String baseTopic = "raw/" + chipId + "/";
const String tempTopic = baseTopic + "temperature";
const String humiTopic = baseTopic + "humidity";
const String presTopic = baseTopic + "pressure";
const String willTopic = baseTopic + "status";
const String vccTopic  = baseTopic + "vcc";


void setup() {
  delay(100);
  Serial.begin(115200);
  // CODE HERE
  delay(100);
  wificonnect();
  
}

void loop() {
  delay(1000);
  // If debug, print info
  if debug_mode == true {
    print_connection_info();
  }
  
}


// ===========================================
// ===============  FUNCTIONS  ===============
// ===========================================

void wificonnect() {
  WiFi.begin("wifi_ssid", "wifi_pass");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connection OK. IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.print("Base Topic for MQTT: ");
  Serial.println(baseTopic);
}

void print_connection_info() {
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Base Topic for MQTT: ");
  Serial.println(baseTopic);
}
