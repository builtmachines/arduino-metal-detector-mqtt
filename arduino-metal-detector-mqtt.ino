#include <Wire.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ------------------ WiFi & MQTT -------------------
const char* ssid = "Tride_LBC";
const char* password = "Tride@1234";

const char* mqtt_server = "fleet-data.evrides.in";
const int mqtt_port = 1883;
const char* mqtt_topic = "sensor/status";
const char* mqtt_username = "gmr";
const char* mqtt_password = "Tride@1234";
const char* imei = "861657079783473";  // Custom identifier

WiFiClient espClient;
PubSubClient client(espClient);

// ------------------ Pins -------------------
#define BUZZER_PIN D1           // GPIO5
#define SENSOR_PIN D2           // GPIO4
#define I2C_SDA D6              // GPIO12 (RTC SDA)
#define I2C_SCL D7              // GPIO13 (RTC SCL)

// ------------------ RTC -------------------
RTC_DS3231 rtc;
bool lastSensorState = HIGH;  // PNP sensors are HIGH when idle
DateTime triggerStartTime;    // Store Triggered start time

// ------------------ WiFi Setup -------------------
void setup_wifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 20) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi connection failed.");
  }
}

// ------------------ MQTT Reconnect -------------------
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP8266Client-" + String(ESP.getChipId());
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" -> retrying in 2 seconds");
      delay(2000);
    }
  }
}

// ------------------ Send MQTT JSON -------------------
void sendMQTTMessage(const char* status, DateTime now, unsigned long holdingTimeSec = 0) {
  char payload[300];

  if (strcmp(status, "Untriggered") == 0) {
    snprintf(payload, sizeof(payload),
      "{\"status\":\"%s\",\"timestamp\":\"%02d/%02d/%04d %02d:%02d:%02d\",\"imei\":\"%s\",\"holding_time\":%lu}",
      status,
      now.day(), now.month(), now.year(),
      now.hour(), now.minute(), now.second(),
      imei,
      holdingTimeSec
    );
  } else {
    snprintf(payload, sizeof(payload),
      "{\"status\":\"%s\",\"timestamp\":\"%02d/%02d/%04d %02d:%02d:%02d\",\"imei\":\"%s\"}",
      status,
      now.day(), now.month(), now.year(),
      now.hour(), now.minute(), now.second(),
      imei
    );
  }

  client.publish(mqtt_topic, payload);
  Serial.print("MQTT Sent: ");
  Serial.println(payload);
}

// ------------------ Setup -------------------
void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(I2C_SDA, I2C_SCL);
  if (!rtc.begin()) {
    Serial.println("RTC not found. Check wiring.");
    while (1);
  }

  // ----- One-time RTC manual set -----
  Serial.println("Manually setting RTC to current IST time.");
  //rtc.adjust(DateTime(2025, 6, 20, 12, 31, 0));  // Set your current time here

  // ---- IMPORTANT: COMMENT ABOVE LINE AFTER FIRST UPLOAD ----
  // rtc.adjust(DateTime(2025, 6, 20, 12, 0, 0));  // ← Comment after first upload

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  Serial.println("System ready: Proximity + RTC + MQTT");
}

// ------------------ Loop -------------------
void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  bool currentSensorState = digitalRead(SENSOR_PIN);  // LOW = metal near

  if (currentSensorState != lastSensorState) {
    DateTime now = rtc.now();

    if (currentSensorState == LOW) {
      Serial.println("Metal Detected!");

      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);

      triggerStartTime = now;  // Record start time
      sendMQTTMessage("Triggered", now);
    } else {
      Serial.println("Metal Removed!");

      unsigned long durationSec = now.unixtime() - triggerStartTime.unixtime();  // Calculate hold duration
      sendMQTTMessage("Untriggered", now, durationSec);
    }

    lastSensorState = currentSensorState;
  }

  delay(50);  // Debounce
}
