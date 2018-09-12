#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define wifi_ssid "<wifi_ssif>"
#define wifi_password "<wifi_pass>"

#define mqtt_server "<ip_address>"
#define mqtt_user "<username>"
#define mqtt_password "<pass>"

// In case you have more than one sensor, make each one a different number here
#define sensor_number "ESP01-A"
#define humidity_topic "sensor/" sensor_number "/humidity/percentRelative"
#define temperature_c_topic "sensor/" sensor_number "/temperature/degreeCelsius"
#define temperature_f_topic "sensor/" sensor_number "/temperature/degreeFahrenheit"
#define barometer_hpa_topic "sensor/" sensor_number "/barometer/hectoPascal"
#define barometer_inhg_topic "sensor/" sensor_number "/barometer/inchHg"
// Lookup for your altitude and fill in here, units hPa
// Positive for altitude above sea level
#define baro_corr_hpa 932.8 // = 692m above sea level    http://www.csgnetwork.com/pressurealtcalc.html


WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_BME280 bme; // I2C

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // Set SDA and SCL ports
  Wire.begin(0, 2);
  // Using I2C on the HUZZAH board SCK=#5, SDI=#4 by default

  // Start sensor
   if (!bme.begin()) {
     Serial.println("Could not find a valid BME280 sensor, check wiring!");
     while (1);
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return newValue < prevValue - maxDiff || newValue > prevValue + maxDiff;
}

long lastMsg = 0;
long lastForceMsg = 0;
bool forceMsg = false;
float temp = 0.0;
float hum = 0.0;
float baro = 0.0;
float diff = 1.0;


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    // MQTT broker could go away and come back at any time
    // so doing a forced publish to make sure something shows up
    // within the first 5 minutes after a reset >>> 300000
    // now set to 5000 = 5s
    if (now - lastForceMsg > 5000) {
      lastForceMsg = now;
      forceMsg = true;
      Serial.println("Forcing publish every 5 minutes...");
    }

    float newTemp = bme.readTemperature();
    float newHum = bme.readHumidity();
    float newBaro = bme.readPressure() / 100.0F;

    if (checkBound(newTemp, temp, diff) || forceMsg) {
      temp = newTemp;
      float temp_c = temp; // Celsius
      float temp_f = temp * 1.8F + 32.0F; // Fahrenheit
      Serial.print("New temperature:");
      Serial.print(String(temp_c) + " degC   ");
      Serial.println(String(temp_f) + " degF");
      client.publish(temperature_c_topic, String(temp_c).c_str(), true);
      client.publish(temperature_f_topic, String(temp_f).c_str(), true);
    }

    if (checkBound(newHum, hum, diff) || forceMsg) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum) + " %");
      client.publish(humidity_topic, String(hum).c_str(), true);
    }

    if (checkBound(newBaro, baro, diff) || forceMsg) {
      baro = newBaro;
      float baro_hpa = baro + baro_corr_hpa; // hPa corrected to sea level
      float baro_inhg = (baro + baro_corr_hpa) / 33.8639F; // inHg corrected to sea level
      Serial.print("New barometer:");
      Serial.print(String(baro_hpa) + " hPa   ");
      Serial.println(String(baro_inhg) + " inHg");
      client.publish(barometer_hpa_topic, String(baro_hpa).c_str(), true);
      client.publish(barometer_inhg_topic, String(baro_inhg).c_str(), true);
    }

    forceMsg = false;
  }
}
