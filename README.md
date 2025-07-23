# Proximaty-metal-detector-mqtt
ESP8266 + RTC + Proximity Sensor MQTT reporting

**Description**
 “This project detects metal objects using a proximity sensor with ESP8266 and DS3231 RTC, then reports the status with timestamps over MQTT.”

**Features**
Real-time metal detection
RTC timestamp logging
MQTT status updates
Buzzer alert

**Hardware Required**
ESP8266 module
DS3231 RTC module
Proximity sensor (PNP)
Buzzer

**Wiring as per the code**
Wiring / Pin Connections
Pin	Function	Notes
D1 (GPIO5)	Buzzer	Output buzzer pin
D2 (GPIO4)	Proximity Sensor	Input sensor pin
D6 (GPIO12)	I2C SDA (RTC)	RTC data line
D7 (GPIO13)	I2C SCL (RTC)	RTC clock line

**Setup Instructions**
Clone or download the code from this repository.
Modify WiFi SSID and password in main.ino.
Modify MQTT credentials as needed.
Upload the sketch using Arduino IDE.

**Usage**
 The device will sound a buzzer and send MQTT messages whenever metal is detected or removed.
