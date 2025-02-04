import time
import paho.mqtt.client as mqtt
from flask import Flask, render_template, request

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')


@app.route('/move', methods=['POST'])
def move():
    allowedCommands = ['forward', 'backward', 'right', 'left', 'side45', 'side135', 'side225', 'side315', 'clockwise', 'counterclockwise']
    command = request.form['direction']

    if command in allowedCommands :
        print(command)
        control = '{ "control": "' + command + '" }'
        mqttc.publish('III2024/05/control', control)

    return '', 204


if __name__ == '__main__':
    # creates an instance of the MQTT client with version 2 of the callback API
    mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

    # establishes a connection to the MQTT broker hosted at 'mqtt.eclipseprojects.io' on port 1883.
    # and starts a background thread to handle MQTT communication asynchronously.
    mqttc.connect('mqtt.eclipseprojects.io', 1883)
    mqttc.loop_start()

    app.run(debug=True)

    mqttc.disconnect()
    mqttc.loop_stop()


