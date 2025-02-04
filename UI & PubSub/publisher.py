import time
import paho.mqtt.client as mqtt
import json

# creates an instance of the MQTT client with version 2 of the callback API
mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# establishes a connection to the MQTT broker hosted at 'mqtt.eclipseprojects.io' on port 1883.
# and starts a background thread to handle MQTT communication asynchronously.
mqttc.connect('mqtt.eclipseprojects.io', 1883)
mqttc.loop_start()

#  publishes messages to the topic 'iii24/test' at one-second intervals. The loop runs 1000 times
msg = 0
for i in range(1000):
    time.sleep(4)
    sense = '{ "voltage": 2.1, "RPM": ' + str(msg) + '}'
    msg_info = mqttc.publish('III2024/05/sense', sense)
    msg += 1

mqttc.disconnect()
mqttc.loop_stop()
