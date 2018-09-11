/*
  Simple battery powered environmental sensor
  ESP-01S + BME280
  ESP8266WiFi https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
  PubSubClient https://github.com/knolleary/pubsubclient
  BME280 https://github.com/adafruit/Adafruit_BME280_Library
  
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
#include <Adafruit_BME280.h>

#define wifi_ssid "nao_hackeie"
#define wifi_pass "AnnaGabriella87"

#define mqtt_server "192.168.0.26"
#define mqtt_user "homeassistant"      // if exist
#define mqtt_password "toronto18"  //idem
#define mqtt_port 1883

// ADC_MODE(ADC_VCC); // Allows the ESP.getVcc() function


void setup() {
  Serial.begin(115200); 
  // put your setup code here, to run once:
  setup_wifi();
}

//SETUP WIFI CONNECTION
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi CONNECTED ");
  Serial.print("=> ESP8266 IP address: ");
  Serial.print(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("WiFi CONNECTED");
  delay(1000);
}
