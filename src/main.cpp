#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include "notification.h"

#define SPEAKER 13
#define RED 12
#define GREEN 27
#define BLUE 14

// These need configuration
const String endpoint = "http://gatekey.dns/register_device";
const String wifiName = "";
const String wifiPass = "";

enum State {
  pendingRegistration,
  registrationComplete,
  pendingMqtt,
  mqttConnected
};

enum WifiState {
  disconnected,
  pending,
  connected
};

State state = pendingRegistration;
WifiState wifiState = disconnected;

WiFiClient wifiClient;
PubSubClient mqttClient;
HTTPClient httpClient;

Preferences preferences;
const char* preferencesNamespace = "GateKey";

Notification* notification;

String mqttPass = "";
String mqttUser = "";
String mqttTopic = "";

void initWifi() {
  notification->setWaitingWifi();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiName, wifiPass);
  Serial.println("Connecting to WiFi ");
}

bool registerDevice() {
  httpClient.begin(endpoint);
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int responseCode = httpClient.POST("device_id=12345");

  StaticJsonDocument<200> jsonDoc;
  String response = httpClient.getString();
  DeserializationError error = deserializeJson(jsonDoc, response);

  if (!error) {
    String outcome = jsonDoc["outcome"];

    if (outcome == "REGISTERED") {
      mqttUser = (const char*) jsonDoc["user"]["user"];
      mqttPass = (const char*) jsonDoc["user"]["password"];
      mqttTopic = (const char*) jsonDoc["topic"];

      preferences.putString("mqttUsr", mqttUser);
      preferences.putString("mqttPass", mqttPass);
      preferences.putString("mqttTopic", mqttTopic);

      Serial.print("Stashed new creds: " + mqttUser + ", " + mqttPass + ". Topic: " + mqttTopic);

      return true;
    } else {
      Serial.print("Response from the registration server: " + outcome);
      
      return false;
    }
  } else {
    Serial.print("Failed to deserialize registration response: ");
    Serial.println(error.f_str());

    return false;
  }
}

boolean checkRegistration() {
  mqttUser = preferences.getString("mqttUsr", "");
  mqttPass = preferences.getString("mqttPass", "");
  mqttTopic = preferences.getString("mqttTopic", "");

  bool registered = (mqttUser != "") && (mqttPass != "") && (mqttTopic != "");

  Serial.print("Checking registration: ");
  Serial.println(registered);

  return registered;
}

void mqttCallback(String topic, byte* message, unsigned int length) {
  Serial.print("[");
  Serial.print(topic);
  Serial.print("]" );
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  notification->gateOpened();
  Serial.println();
}

void initMqtt() {
  mqttClient.setClient(wifiClient);
  mqttClient.setServer("192.168.1.100", 1883);
  mqttClient.setCallback(mqttCallback);
}

bool reconnectMqtt() {
  Serial.println("Attempting MQTT connection with creds: " + mqttUser + ", " + mqttPass);

  if (mqttClient.connect("notifier", mqttUser.c_str(), mqttPass.c_str())) {
    Serial.println("Connected to MQTT broker.");
    mqttClient.subscribe("GateKey/household/default");
    return true;
  } else {
    Serial.print("Failed to connect. MQTT return code = " + String(mqttClient.state()));
    return false;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  notification = new Notification(SPEAKER, RED, GREEN, BLUE);

  preferences.begin(preferencesNamespace, false); // false -> RW mode

  // preferences.clear();

  initMqtt();
}

/**
              Wifi State───────────┐
               │   ┌────────────┐  │
               │ ┌─►disconnected│  │
               │ │ └─────┬──────┘  │
               │ │   ┌───▼───┐     │
               │ │   │pending│     │
               │ │   └───┬───┘     │
               │ │  ┌────▼────┐    │
               │ └──┤connected│    │
               │    └─────────┘    │
               └───────┬────▲──────┘
                       │    │
    MQTT state─────────▼────┴─────────────────┐
     │                 ┌────────┐             |
     │           ┌─────▼────┐  new reg        |
     │           │pendingReg├──fail           |
     │           └───┬──┬───┘                 |
     │       existing│  │new reg              |
     │            reg│  │success              |
     │           ┌───▼──▼────┐                |
     │           │regComplete│                |
     │           └─────┬─────┘                |
     │                 │                      |
     │           ┌─────▼─────┐                |
     │      ┌────►pendingMqtt◄───────┐        |
     │      │    └───┬──┬────┘       │        |
     │  connect──────┘  │connected   │        |
     │     fail         │            │        |
     │          ┌───────▼─────┐      │        |
     │          │mqttConnected├─ disconnected |
     │          └─────────────┘               │
     └────────────────────────────────────────┘

- The WiFi state machine is only executed when WiFi is disconnected.
- If WiFi does disconnected, the MQTT state machine pauses until WiFi is reconnected.
- On reconnection, the MQTT state machine unpauses and executes from the state it was in pre-disconnection
*/
void loop() {
  // Being connected to the internet is a prerequisite for everything else.
  if (WiFi.status() != WL_CONNECTED && wifiState == connected) {
    wifiState = disconnected;
  }

  switch (wifiState) {
    case disconnected:
      notification->setWaitingWifi();
      wifiState = pending;
      initWifi();
      return;
    case pending:
      if (WiFi.status() == WL_CONNECTED) {
        wifiState = connected;
        Serial.println("IP address is " + String(WiFi.localIP()));
      }
      return;
    // Connected state does nothing.
  }

  switch (state) {
    case pendingRegistration:
      notification->setWaitingCreds();
      if (checkRegistration()) {
        state = registrationComplete;
      } else if (registerDevice()) {
        state = registrationComplete;
      }
      break;
    case registrationComplete:
      state = pendingMqtt;
      break;
    case pendingMqtt:
      notification->setWaitingMqtt();
      if (reconnectMqtt()) {
        state = mqttConnected;
        notification->setConnected();
      }
      break;
    case mqttConnected:
      if (!mqttClient.connected()) {
        state = pendingMqtt;
      } else {
        mqttClient.loop();
      }
      break;
  }

  notification->loop();
  delay(500);
}