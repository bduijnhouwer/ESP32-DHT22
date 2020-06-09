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