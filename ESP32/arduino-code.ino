/*Just a little program to read the temperature and humidity from a DHT22 sensor and
pass it to MQTT.

B. Duijnhouwer
June, 8th 2020
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <dht.h>

#define wifi_ssid "***WIFI_SSID***"             //wifi ssid
#define wifi_password "***WIFI_PASSWORD***"     //wifi password

#define mqtt_server "***SERVER_NAME***"       // server name or IP
#define mqtt_user "***MQTT_USER***"           // username
#define mqtt_password "***MQTT_PASSWORD***"   // password

#define topic "glasshouse/dhtreadings"
#define debug_topic "glasshouse/debug"                   //Topic for debugging

/* definitions for deepsleep */
#define uS_TO_S_FACTOR 1000000        /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 180              /* Time ESP32 will go to sleep for 5 minutes (in seconds) */

bool debug = true;             //Display log message if True

#define DHT22_PIN 14

dht DHT;
WiFiClient espClient;
PubSubClient client(espClient);

char data[80];

void setup()
{
    Serial.begin(115200);
    setup_wifi();                           //Connect to Wifi network

    client.setServer(mqtt_server, 1883);    // Configure MQTT connection, change port if needed.

    if (!client.connected()) {
      reconnect();
    }

    // READ DATA
    int chk = DHT.read22(DHT22_PIN);

    float t = DHT.temperature;
    float h = DHT.humidity;

    String dhtReadings = "{\"temperature\":\"" + String(t) + "\", \"humidity\":\"" + String(h) + "\"}";
    dhtReadings.toCharArray(data, (dhtReadings.length() + 1));

    if ( debug ) {
      Serial.print("Temperature : ");
      Serial.print(t);
      Serial.print(" | Humidity : ");
      Serial.println(h);
    } 

    // Publish values to MQTT topics
    client.publish(topic, data);   // Publish readings on topic (glasshouse/dhtreadings)
    if ( debug ) {    
      Serial.println("Readings sent to MQTT.");
    }

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //go to sleep
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
    Serial.println("Going to sleep as normal now.");
    esp_deep_sleep_start();

}


//Setup connection to wifi
void setup_wifi() {
  delay(20);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

 Serial.println("");
 Serial.println("WiFi is OK ");
 Serial.print("=> ESP32 new IP address is: ");
 Serial.print(WiFi.localIP());
 Serial.println("");
}

//Reconnect to wifi if connection is lost
void reconnect() {

  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("OK");
    } else {
      Serial.print("[Error] Not connected: ");
      Serial.print(client.state());
      Serial.println("Wait 5 seconds before retry.");
      delay(5000);
    }
  }
}


void loop()
{

}