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

// These all need configuration
const String endpoint = "https://brydonleonard.com/register_device"; // Change if you're not using the managed version of GateKey
const String wifiName = "YOUR_WIFI_NAME";
const String wifiPass = "YOUR_WIFI_PASS";
const char* mqttEndpoint = "mqtt.brydonleonard.com"; // Change if you're not using the managed version of GateKey
const String deviceId = "YOUR_DEVICE_ID_HERE"; // This can be whatever you want, as long as you can remember it
uint16_t mqttPort = 1884;

// The CA cert for brydonleonard.com
const char* serverCACert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFjDCCA3SgAwIBAgINAgO8UKMnU/CRgCLt8TANBgkqhkiG9w0BAQsFADBHMQsw\n" \
"CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n" \
"MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMjAwODEzMDAwMDQyWhcNMjcwOTMwMDAw\n" \
"MDQyWjBGMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n" \
"Y2VzIExMQzETMBEGA1UEAxMKR1RTIENBIDFQNTCCASIwDQYJKoZIhvcNAQEBBQAD\n" \
"ggEPADCCAQoCggEBALOC8CSMvy2Hr7LZp676yrpE1ls+/rL3smUW3N4Q6E8tEFha\n" \
"KIaHoe5qs6DZdU9/oVIBi1WoSlsGSMg2EiWrifnyI1+dYGX5XNq+OuhcbX2c0IQY\n" \
"hTDNTpvsPNiz4ZbU88ULZduPsHTL9h7zePGslcXdc8MxiIGvdKpv/QzjBZXwxRBP\n" \
"ZWP6oK/GGD3Fod+XedcFibMwsHSuPZIQa4wVd90LBFf7gQPd6iI01eVWsvDEjUGx\n" \
"wwLbYuyA0P921IbkBBq2tgwrYnF92a/Z8V76wB7KoBlcVfCA0SoMB4aQnzXjKCtb\n" \
"7yPIox2kozru/oPcgkwlsE3FUa2em9NbhMIaWukCAwEAAaOCAXYwggFyMA4GA1Ud\n" \
"DwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwEgYDVR0T\n" \
"AQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQU1fyeDd8eyt0Il5duK8VfxSv17LgwHwYD\n" \
"VR0jBBgwFoAU5K8rJnEaK0gnhS9SZizv8IkTcT4waAYIKwYBBQUHAQEEXDBaMCYG\n" \
"CCsGAQUFBzABhhpodHRwOi8vb2NzcC5wa2kuZ29vZy9ndHNyMTAwBggrBgEFBQcw\n" \
"AoYkaHR0cDovL3BraS5nb29nL3JlcG8vY2VydHMvZ3RzcjEuZGVyMDQGA1UdHwQt\n" \
"MCswKaAnoCWGI2h0dHA6Ly9jcmwucGtpLmdvb2cvZ3RzcjEvZ3RzcjEuY3JsME0G\n" \
"A1UdIARGMEQwOAYKKwYBBAHWeQIFAzAqMCgGCCsGAQUFBwIBFhxodHRwczovL3Br\n" \
"aS5nb29nL3JlcG9zaXRvcnkvMAgGBmeBDAECATANBgkqhkiG9w0BAQsFAAOCAgEA\n" \
"bGMn7iPf5VJoTYFmkYXffWXlWzcxCCayB12avrHKAbmtv5139lEd15jFC0mhe6HX\n" \
"02jlRA+LujbdQoJ30o3d9T/768gHmJPuWtC1Pd5LHC2MTex+jHv+TkD98LSzWQIQ\n" \
"UVzjwCv9twZIUX4JXj8P3Kf+l+d5xQ5EiXjFaVkpoJo6SDYpppSTVS24R7XplrWf\n" \
"B82mqz4yisCGg8XBQcifLzWODcAHeuGsyWW1y4qn3XHYYWU5hKwyPvd6NvFWn1ep\n" \
"QW1akKfbOup1gAxjC2l0bwdMFfM3KKUZpG719iDNY7J+xCsJdYna0Twuck82GqGe\n" \
"RNDNm6YjCD+XoaeeWqX3CZStXXZdKFbRGmZRUQd73j2wyO8weiQtvrizhvZL9/C1\n" \
"T//Oxvn2PyonCA8JPiNax+NCLXo25D2YlmA5mOrR22Mq63gJsU4hs463zj6S8ZVc\n" \
"pDnQwCvIUxX10i+CzQZ0Z5mQdzcKly3FHB700FvpFePqAgnIE9cTcGW/+4ibWiW+\n" \
"dwnhp2pOEXW5Hk3xABtqZnmOw27YbaIiom0F+yzy8VDloNHYnzV9/HCrWSoC8b6w\n" \
"0/H4zRK5aiWQW+OFIOb12stAHBk0IANhd7p/SA9JCynr52Fkx2PRR+sc4e6URu85\n" \
"c8zuTyuN3PtYp7NlIJmVuftVb9eWbpQ99HqSjmMd320=\n" \
"-----END CERTIFICATE-----\n";

// The cert for mqtt.brydonleonard.com
const char* mqttCACert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFYDCCBEigAwIBAgIQQAF3ITfU6UK47naqPGQKtzANBgkqhkiG9w0BAQsFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTIxMDEyMDE5MTQwM1oXDTI0MDkzMDE4MTQwM1ow\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwggIiMA0GCSqGSIb3DQEB\n" \
"AQUAA4ICDwAwggIKAoICAQCt6CRz9BQ385ueK1coHIe+3LffOJCMbjzmV6B493XC\n" \
"ov71am72AE8o295ohmxEk7axY/0UEmu/H9LqMZshftEzPLpI9d1537O4/xLxIZpL\n" \
"wYqGcWlKZmZsj348cL+tKSIG8+TA5oCu4kuPt5l+lAOf00eXfJlII1PoOK5PCm+D\n" \
"LtFJV4yAdLbaL9A4jXsDcCEbdfIwPPqPrt3aY6vrFk/CjhFLfs8L6P+1dy70sntK\n" \
"4EwSJQxwjQMpoOFTJOwT2e4ZvxCzSow/iaNhUd6shweU9GNx7C7ib1uYgeGJXDR5\n" \
"bHbvO5BieebbpJovJsXQEOEO3tkQjhb7t/eo98flAgeYjzYIlefiN5YNNnWe+w5y\n" \
"sR2bvAP5SQXYgd0FtCrWQemsAXaVCg/Y39W9Eh81LygXbNKYwagJZHduRze6zqxZ\n" \
"Xmidf3LWicUGQSk+WT7dJvUkyRGnWqNMQB9GoZm1pzpRboY7nn1ypxIFeFntPlF4\n" \
"FQsDj43QLwWyPntKHEtzBRL8xurgUBN8Q5N0s8p0544fAQjQMNRbcTa0B7rBMDBc\n" \
"SLeCO5imfWCKoqMpgsy6vYMEG6KDA0Gh1gXxG8K28Kh8hjtGqEgqiNx2mna/H2ql\n" \
"PRmP6zjzZN7IKw0KKP/32+IVQtQi0Cdd4Xn+GOdwiK1O5tmLOsbdJ1Fu/7xk9TND\n" \
"TwIDAQABo4IBRjCCAUIwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw\n" \
"SwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5pZGVudHJ1\n" \
"c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTEp7Gkeyxx\n" \
"+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEB\n" \
"ATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQu\n" \
"b3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0LmNvbS9E\n" \
"U1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFHm0WeZ7tuXkAXOACIjIGlj26Ztu\n" \
"MA0GCSqGSIb3DQEBCwUAA4IBAQAKcwBslm7/DlLQrt2M51oGrS+o44+/yQoDFVDC\n" \
"5WxCu2+b9LRPwkSICHXM6webFGJueN7sJ7o5XPWioW5WlHAQU7G75K/QosMrAdSW\n" \
"9MUgNTP52GE24HGNtLi1qoJFlcDyqSMo59ahy2cI2qBDLKobkx/J3vWraV0T9VuG\n" \
"WCLKTVXkcGdtwlfFRjlBz4pYg1htmf5X6DYO8A4jqv2Il9DjXA6USbW1FzXSLr9O\n" \
"he8Y4IWS6wY7bCkjCWDcRQJMEhg76fsO3txE+FiYruq9RUWhiF1myv4Q6W+CyBFC\n" \
"Dfvp7OOGAN6dEOM4+qR9sdjoSYKEBpsr6GtPAQw4dy753ec5\n" \
"-----END CERTIFICATE-----\n";


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

WiFiClientSecure mqttWifiClient;
WiFiClientSecure registrationWifiClient;
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
  httpClient.begin(registrationWifiClient, endpoint);
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int responseCode = httpClient.POST("device_id=" + deviceId);

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
  mqttWifiClient.setCACert(mqttCACert);
  registrationWifiClient.setCACert(serverCACert);
  mqttClient.setClient(mqttWifiClient);
  mqttClient.setServer(mqttEndpoint, mqttPort);
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