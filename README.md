![alt text](https://content.instructables.com/F0N/LV6I/KB6IE87P/F0NLV6IKB6IE87P.LARGE.jpg?auto=webp&frame=1&width=700&height=1024&fit=bounds)

## Temperature and Humidity Using ESP32-DHT22-MQTT-MySQL-Python-PHP

My girlfriend wanted a glasshouse, so I made her one. But I wanted a temperature and humidity sensor inside the glasshouse. So, I googled for examples and started experimenting.

My conclusion was that all the examples that I found were not exactly what I wanted to build. I grabbed a lot of little parts of code and combined them. It took me quite a while to finish my first working build because the documentation of most examples were too difficult for me to understand or they assumed part that I should know?? But I didn’t know nothing (yet) ☹

That’s why I build this instructable. A “beginning-until the end” tutorial for literally everybody to understand. (At least I hope 😊)


### How it works ...

* The end-product is an ESP32-CAM with a DHT22 sensor attached to it which get’s it power from a 18650 battery. Every three minutes it reads the temperature and humidity and sends this over WiFi to an external MQTT server and then goes to sleep (for three minutes) to use as less battery as needed.
* On a Debian server, (which could also be a raspberry pi I guess) I have python3, a MQTT server, a MySQL server and a webserver.
* The python3 script runs as a service and whenever it receives a MQTT message, it counts the previous number of entries (index number) and increments this by one. Then it reads the values of the temperature and the humidity from the MQTT message. It checks for false values and whenever the values are correct, it sends the values together with the new index number and the current date and time to a MySQL server.
* The webserver has a PHP script which reads the values from the MySQL server and makes a nice graph from it using Google Charts. (example)


### Supplies:

The parts I used are the following:

* ESP32-CAM (The reason I used the cam version is because it has an external antenna connector on it. There are probably also other ESP32’s you could use) 
* External antenna 
* AM2302 DHT22 sensor (This one has a built-in resistor, so you only need three wires)
[https://www.amazon.de/gp/product/B07CM2VLBK/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1](https://www.amazon.de/gp/product/B07CM2VLBK/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1)
* 18650 battery shield v3 
* 18650 battery (NCR18650B) 
* Old micro USB cable (for connecting the ESP32 to the battery shield) 
* Old micro USB cable (for connecting the ESP32 to the battery shield) 

Extra needed:

* USB to TTL connector (picture)
[https://www.amazon.de/FT232RL-Seriell-Unterst%C3%BCtzung-Kompatibilit%C3%A4t-Download/dp/B07V6XQTDH/ref=sr_1_6?dchild=1&keywords=usb+zu+ttl&qid=1591687427&s=diy&sr=1-6](https://www.amazon.de/FT232RL-Seriell-Unterst%C3%BCtzung-Kompatibilit%C3%A4t-Download/dp/B07V6XQTDH/ref=sr_1_6?dchild=1&keywords=usb+zu+ttl&qid=1591687427&s=diy&sr=1-6)
* Soldering iron 
* 3D printer (only needed for housing case)


## Step 1: Upload the Arduino Code to the ESP32-CAM
![alt-text](https://content.instructables.com/FJS/F4OU/KB6IE68O/FJSF4OUKB6IE68O.LARGE.jpg?auto=webp&frame=1&fit=bounds)

# So let’s begin!

To upload the Arduino code to the ESP32-CAM, you have to connect the USBtoTTL connector to the ESP32 using the schematics above.

The Arduino code is:
```javascript
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
```
### **And don't forget to replace the credentials with your own credentials!**


## Step 2: Wire Up!

![Alt text](https://content.instructables.com/FPD/2P27/KB6IE68N/FPD2P27KB6IE68N.LARGE.jpg?auto=webp&frame=1&fit=bounds)

For the power, I used an old USB cable of which I cut off the USB-A connector. There are four wires in the USB cable, we only need the black and the red ones.

So, connect everything according to the schedule above.


## Step 3: Python3 Script

The Python3 script goes into a place where it’s accessible to the root user.

I used /root/scripts/glasshouse/glasshouse.py for this script.
The contents of the python script is:

```python
# Python3 script to connect to MQTT, read values and write them into MySQL
#
# B. Duijnhouwer
# June, 8th 2020
#
# version: 1.0
#
#
import paho.mqtt.client as mqtt
import json
import pymysql
pymysql.install_as_MySQLdb()
import MySQLdb
from datetime import datetime

db= MySQLdb.connect("localhost", "glasshouse", "***MYSQL_USERNAME***", "***MYSQL_PASSWORD***")
cursor=db.cursor()

broker_address= "localhost"          #Broker address
port = 1883                          #Broker port
user = "***MQTT_USERNAME***"         #Connection username
password = "***MQTT_PASSWORD***"     #Connection password

def on_connect(client, userdata, flags, rc):  # The callback for when the client connects to the broker
    print("Connected with result code {0}".format(str(rc)))  # Print result of connection attempt
    client.subscribe("glasshouse/dhtreadings/#")

def on_message(client, userdata, msg):  # The callback for when a PUBLISH message is received from the server.
    cursor.execute ("select * from sensordata")
    numrows = int (cursor.rowcount)
    newrow = numrows + 1

    now = datetime.now()
    formatted_date = now.strftime('%Y-%m-%d %H:%M:%S')

    payload = json.loads(msg.payload.decode('utf-8'))
    print("New row: "+str(newrow))
    temperature = float(payload["temperature"])
    humidity = float(payload["humidity"])
    print("Temperature: "+str(temperature))
    print("Humidity: "+str(humidity))
    print("DateTime: "+str(formatted_date))
    if (( temperature > -20) and (temperature < 40)) and ((humidity >= 0) and (humidity <= 100)):
      cur = db.cursor()
      cur.execute("INSERT INTO glasshouse.sensordata (idx, temperature, humidity, timestamp) VALUES ("+str(newrow)+", "+str(temperature)+", "+str(humidity)+", %s)", (formatted_date))
      db.commit()
      print("data received and imported in MySQL")
    else:
      print("data exceeded limits and is NOT imported in MySQL")


client = mqtt.Client("duijnhouwer-com-glasshouse-script")
client.username_pw_set(user, password=password)
client.on_connect = on_connect  # Define callback function for successful connection
client.on_message = on_message  # Define callback function for receipt of a message
client.connect(broker_address, port=port)          #connect to broker
client.loop_forever()  # Start networking daemon
```
### Don't forget to replace the MySQL username and password and the MQTT username and password to your own credentials !

You can make the script run as a service by creating two files.

The first one is “/etc/init/glasshouse.conf” with the following contents:

```python
start on runlevel [2345]
stop on runlevel [!2345]
exec /root/scripts/glasshouse/glasshouse.py
```

The second one is “/etc/systemd/system/multi-user.target.wants/glasshouse.service”
with the following contents:

```python
[Unit]
Description=Glasshouse Monitoring Service
After=multi-user.target
<br>[Service]
Type=simple
Restart=always
RestartSec=1
ExecStart=/usr/bin/python3 /root/scripts/glasshouse/glasshouse.py

[Install]
WantedBy=multi-user.target
```
You can make this run as a service using the following command:

```python
systemctl enable glasshouse
```
and start it using: 

```python
systemctl start glasshouse
```


## Step 4: MySQL Server
You have to create a new MySQL database with just one table in it.

The code for creating the table is:
```python
CREATE TABLE `sensordata` (
  `idx` int(11) DEFAULT NULL,
  `temperature` float DEFAULT NULL,
  `humidity` float DEFAULT NULL,
  `timestamp` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```


## Step 5: Webserver
The webserver has two files, the index.php file and one config.ini file

The contents of the config.ini file is:
```python
[database]
db_host = "localhost"
db_name = "glasshouse"
db_table = "sensordata"
db_user = "***DATABASE_USER***"
db_password = "***DATABASE_PASSWORD***"
```

Where offcourse you replace ***DATABASE_USER*** and ***DATABASE_PASSWORD*** with your own credentials.

The contents of the index.php file is:
```python
<?php
# Loading config data from *.ini-file
$ini = parse_ini_file ('config.ini');

# Assigning the ini-values to usable variables
$db_host = $ini['db_host'];
$db_name = $ini['db_name'];
$db_table = $ini['db_table'];
$db_user = $ini['db_user'];
$db_password = $ini['db_password'];

# Prepare a connection to the mySQL database
$connection = new mysqli($db_host, $db_user, $db_password, $db_name);

?>
<!-- start of the HTML part that Google Chart needs -->
<html>
<head>
        <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
<!-- This loads the 'corechart' package. -->
    <script type="text/javascript">
        google.charts.load('current', {'packages':['corechart']});
        google.charts.setOnLoadCallback(drawChart);

                function drawChart() {
                var data = google.visualization.arrayToDataTable([
//                      ['Timestamp', 'Temperature', 'Humidity', 'Heat Index'],
                        ['Timestamp', 'Temperature', 'Humidity'],
<?php

# This query connects to the database and get the last 10 readings
$sql = "SELECT temperature, humidity, timestamp FROM $db_table";

$result = $connection->query($sql);

# This while - loop formats and put all the retrieved data into ['timestamp', 'temperature', 'humidity'] way.
        while ($row = $result->fetch_assoc()) {
                $timestamp_rest = substr($row["timestamp"],10,6);
                echo "['".$timestamp_rest."',".$row['temperature'].",".$row['humidity']."],";
//              echo "['".$timestamp_rest."',".$row['temperature'].",".$row['humidity'].",".$row['heatindex']."],";
                }
?>
]);

// Curved line
var options = {
                title: 'Temperature and humidity',
                curveType: 'function',
                legend: { position: 'bottom' },
                hAxis: {
                        slantedText:true,
                        slantedTextAngle:45
                        }
                };

// Curved chart
var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));
chart.draw(data, options);

} // End bracket from drawChart
//</script>

<!-- The charts below is ony available in the 'bar' package -->
<script type="text/javascript">
</script>
</head>

<?php

# Prepare a connection to the mySQL database
$connection = new mysqli($db_host, $db_user, $db_password, $db_name);

?>
<div id="curve_chart" style="width: 1600px; height: 640px;"></div>
<div id="barchart_values" style="width: 900px; height: 480px;"></div>
<div id="top_x_div" style="width: 900px; height: 480px;"></div>
```

## Step 6: 3D Printed Housing
For the housing, I used two separate housings, one for the ESP32-CAM and DHT22 together and one for the 18650 battery shield.

[Case_bottom.stl] (https://content.instructables.com/ORIG/FCY/DNS4/KB6IE7CU/FCYDNS4KB6IE7CU.stl)

[Case_top.stl] (https://content.instructables.com/ORIG/FSY/2SN2/KB6IE7CV/FSY2SN2KB6IE7CV.stl)

[18650_base.stl] (https://content.instructables.com/ORIG/FCH/HBMO/KB6IE7CW/FCHHBMOKB6IE7CW.stl)

[18650_lid.stl] (https://content.instructables.com/ORIG/FCG/FIU0/KB6IE7CX/FCGFIU0KB6IE7CX.stl)


## Step 7: The Final Result!

![alt text](https://content.instructables.com/FI9/92LP/KB6IE7U1/FI992LPKB6IE7U1.LARGE.jpg?auto=webp&frame=1&width=700&height=1024&fit=bounds)
![alt text](https://content.instructables.com/FDD/13L6/KB6IE7U0/FDD13L6KB6IE7U0.LARGE.jpg?auto=webp&frame=1&width=263&height=1024&fit=bounds)
![alt text](https://content.instructables.com/FCE/GXAE/KB6IE7TQ/FCEGXAEKB6IE7TQ.LARGE.jpg?auto=webp&frame=1&width=263&height=1024&fit=bounds)
![alt text](https://content.instructables.com/FZL/7PGE/KB6IE84M/FZL7PGEKB6IE84M.LARGE.jpg?auto=webp&frame=1&width=1024&fit=bounds)


The final result is also shown in the pictures above.

And whenever the battery is empty, you can charge it with a mini USB cable.
