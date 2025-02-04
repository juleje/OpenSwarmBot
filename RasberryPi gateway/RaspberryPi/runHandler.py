import ssl
import threading
import logging
import serial
from time import sleep
import paho.mqtt.client as mqtt
import json

commandMap = {
    "forward": b"\x00"
    , "backward": b"\x01"
    , "right": b"\x02"
    , "left": b"\x03"
    , "side45": b"\x04"
    , "side135": b"\x05"
    , "side225": b"\x06"
    , "side315": b"\x07"
    , "clockwise": b"\x08"
    , "counterclockwise": b"\x09"
    , "stop": b"\x0a"
}

def MQTT_TO_UART():
    def on_connect(client, userdata, flags, reason_code, properties):
        logging.info(f"Connected with result code {reason_code}")
        client.subscribe('III2024/05/control')

    def on_message(client, userdata, message):
        decoded = str(message.payload.decode("utf-8"))
        decoded_json = json.loads(decoded)
        control = decoded_json["control"]
        groupNumber = decoded_json["group"]

        if groupNumber == 5 and control in commandMap.keys():
            toBeSent = b"\x05" + commandMap[control]
            logging.info(
                f"Received command: {str(int.from_bytes(commandMap[control], byteorder='little'))}, received group number: {str(groupNumber)}")
            ser.write(toBeSent)

    mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

    mqttc.username_pw_set("rpi", "g5rpi")
    mqttc.tls_set(ca_certs='./certs/ca.crt',
                  certfile='./certs/client.crt',
                  keyfile='./certs/client.key',
                  cert_reqs=ssl.CERT_REQUIRED)

    mqttc.on_connect = on_connect
    mqttc.on_message = on_message

    mqttc.connect(broker_ip, broker_port)
    mqttc.loop_forever()


def UART_to_MQTT():
    def on_connect(client, userdata, flags, reason_code, properties):
        logging.info(f"Connected with result code {reason_code}")

    mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

    mqttc.username_pw_set("rpi", "g5rpi")
    mqttc.tls_set(ca_certs='./certs/ca.crt',
                  certfile='./certs/client.crt',
                  keyfile='./certs/client.key',
                  cert_reqs=ssl.CERT_REQUIRED)

    mqttc.on_connect = on_connect

    mqttc.connect(broker_ip, broker_port)
    mqttc.loop_start()

    while True:
        received_data = ser.read()
        sleep(0.03)
        data_left = ser.inWaiting()
        received_data += ser.read(data_left)

        if received_data is not None and len(received_data) == 6:
            groupEncoded = received_data[:2]
            voltageEncoded = received_data[2:4]
            rpmEncoded = received_data[4:6]

            groupDecoded = int.from_bytes(groupEncoded, byteorder='little')
            voltageDecoded = int.from_bytes(voltageEncoded, byteorder='little')
            rpmDecoded = int.from_bytes(rpmEncoded, byteorder='little')

            if groupDecoded == 5 and 0 <= voltageDecoded < 3000 and 0 <= rpmDecoded < 3000:
                mqttc.publish('III2024/05/sense',
                              '{"group":' + str(groupDecoded) + ', "voltage":' + str(voltageDecoded) + ', "RPM":' + str(rpmDecoded) + '}')


logging.basicConfig(filename='log', level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

broker_ip = "192.168.191.189"
broker_port = 8883

ser = serial.Serial("/dev/ttyS0", 9600)

# Runs both the UART_to_MQTT and MQTT_to_UART functions on separate threads
mqtt_to_uart = threading.Thread(target=MQTT_TO_UART, args=[])
uart_to_mqtt = threading.Thread(target=UART_to_MQTT, args=[])

threads = [mqtt_to_uart, uart_to_mqtt]

for thread in threads:
    thread.start()

for thread in threads:
    thread.join()

logging.shutdown()
