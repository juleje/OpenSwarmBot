import paho.mqtt.client as mqtt
import json

# function is a callback that gets called whenever a message is received on the subscribed topic. It prints out the received message.
def on_message(client, userdata, message):
    x = str(message.payload.decode("utf-8"))
    y = json.loads(x)
   # print("Received voltage: ", y["voltage"])
   # print("Received RPM: ", y["RPM"])
    print("Received control: ", y["control"])


# creates an instance of the MQTT client with version 2 of the callback API.
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.connect('mqtt.eclipseprojects.io',
               1883)  # establishes a connection to the MQTT broker hosted at 'mqtt.eclipseprojects.io' on port 1883.
client.on_message = on_message  # assigns the previously defined on_message function as the callback for processing incoming messages.
client.subscribe(
     'III2024/05/sense')  # subscribes the client to the MQTT topic 'III2024/05/sense', indicating that it wants to receive messages published to this topic.
client.subscribe(
     'III2024/05/control')  # subscribes the client to the MQTT topic 'III2024/05/control', indicating that it wants to receive messages published to this topic.

# starts a loop that runs the MQTT client's network loop to handle incoming messages indefinitely. The argument 10 specifies the maximum number of seconds the loop will run.
client.loop_forever(10)
