import paho.mqtt.client as mqtt
import json
import time
from datetime import datetime

id = 'aa6ba08d-0ca2-41ec-8d4c-9e56112ff81f'

print("Comenzando lecturas")
client_telemetry_topic = id + '/telemetry'
client_name = id + 'nightligth_server'

mqtt_client = mqtt.Client(client_name)
mqtt_client.connect('test.mosquitto.org')

mqtt_client.loop_start()

def handle_telemetry(client, userdata, message):
    now = datetime.now()
    timestamp = datetime.timestamp(now)
    payload = json.loads(message.payload.decode())
    print("Mensaje recibido: ", payload)

mqtt_client.subscribe(client_telemetry_topic)
mqtt_client.on_message = handle_telemetry

while True:
    time.sleep(60*10)
