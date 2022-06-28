/*
  Name:Touch with LED
  Description:When sensor is pressed LED will be turned ON and LED will be turned OFF
  when sensor is again pressed.
  PINOUTS FOR IR AND BUZZER
  Touchsensor I/O = GPIO18
  LED1            = GPIO19
 */

#include "WiFi.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#define SENSOR_PIN  18 // ESP32 pin GIOP18 connected to the sensor's SIGNAL pin of touch sensor
#define LED_PIN 19

WiFiClient espClient;
PubSubClient mqttClient(espClient);

String KITNO = WiFi.macAddress();
String subTopic = "KIT/" + KITNO + "/TOUCHSWITCH";
String Publish =  KITNO + "/TOUCHSWITCH";
String gPublish =  KITNO + "/GENERAL";


int addr_wifi_ssid = 50;
int addr_wifi_password = 70;
int addr_mqtt_host = 90;
int addr_mqtt_user_name = 110;
int addr_mqtt_password = 130;
int addr_mqtt_port = 150;

String read_ssid;
String read_password;
String mqtt_user_name;
String mqtt_password;
String mqtt_endpoint;
int read_mqtt_port;

const char * serverSSID = "IOT-LAB-KIT";
const char * serverPassword = "Petal#2021";

// Variables will change:
int lastState = LOW;      // the previous state from the input pin
int currentState;         // the current reading from the input pin

void dataRead() {
  EEPROM.begin(512);

  Serial.print("My Mac Address is: " + KITNO);
  read_ssid = readWifiSsidFromMemory();
  read_password = readWifiPasswordFromMemory();
  mqtt_user_name = readMqttUserNameFromMemory();
  mqtt_password = readMqttPasswordFromMemory();
  mqtt_endpoint = readMqttHostFromMemory();
  read_mqtt_port = atoi(readMqttPortFromMemory().c_str());
}
void connectWithWiFi() {
  long connectionTimeWait = 0;
  long connectionTimeOut = 15000;
  Serial.println("Connecting with wifi");
  Serial.println(read_ssid);
  Serial.println("With password");
  Serial.println(read_password);
  WiFi.begin(read_ssid.c_str(), read_password.c_str());
  Serial.println("SSID data available in memory");
  while (WiFi.status() != WL_CONNECTED && connectionTimeWait < connectionTimeOut) {
    delay(500);
    Serial.println("Trying to connect WiFi..");
    connectionTimeWait = connectionTimeWait + 500;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to the WiFi network with IP: ");
    Serial.println(WiFi.localIP());
    connectionTimeWait = 0;
    connectWithMqtt();
  }
  else {
    Serial.println("Cannot connect to the WiFi network");
    Serial.println(WiFi.localIP());
  }
}

//========================================READ DATA FROM INTERNALA MEMORY OF ESP32=================================================================
String readWifiSsidFromMemory() {
  String wifiSsidData;
  for (int m = addr_wifi_ssid; m < addr_wifi_ssid + 20; ++m) {
    wifiSsidData += char(EEPROM.read(m));
  }
  return wifiSsidData;
}

String readWifiPasswordFromMemory() {
  String wifiPasswordData;
  for (int n = addr_wifi_password; n < addr_wifi_password + 20; ++n) {
    wifiPasswordData += char(EEPROM.read(n));
  }
  return wifiPasswordData;
}

String readMqttHostFromMemory() {
  String mqttHost;
  for (int n = addr_mqtt_host; n < addr_mqtt_host + 20; ++n) {
    mqttHost += char(EEPROM.read(n));
  }
  return mqttHost;
}

String readMqttPortFromMemory() {
  String mqttPort;
  for (int n = addr_mqtt_port; n < addr_mqtt_port + 20; ++n) {
    mqttPort += char(EEPROM.read(n));
  }
  return mqttPort;
}

String readMqttUserNameFromMemory() {
  String mqttUsername;
  for (int n = addr_mqtt_user_name; n < addr_mqtt_user_name + 20; ++n) {
    mqttUsername += char(EEPROM.read(n));
  }
  return mqttUsername;
}

String readMqttPasswordFromMemory() {
  String mqttPassword;
  for (int n = addr_mqtt_password; n < addr_mqtt_password + 20; ++n) {
    mqttPassword += char(EEPROM.read(n));
  }
  return mqttPassword;
}
//==================================================== CONNECT TO MQTT===============================================================================

void connectWithMqtt() {

  Serial.println(mqtt_endpoint);
  const char *read_mqtt_server = mqtt_endpoint.c_str();
  Serial.println("MQTT connecting to address");
  Serial.println(read_mqtt_server);
  Serial.println("With port");
  Serial.println(read_mqtt_port);

  mqttClient.setServer(read_mqtt_server, read_mqtt_port);
  mqttClient.setKeepAlive(60);

  Serial.println("Connecting to MQTT...");
  String clientId = KITNO;
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(clientId.c_str(), mqtt_user_name.c_str(), mqtt_password.c_str())) {
      Serial.println("connected");
      mqttClient.setCallback(callback);
      mqttClient.subscribe(subTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

}

void setup() {
  Serial.begin(115200);
  dataRead();
  connectWithWiFi();
  Serial.print("My Mac Address is: " + KITNO);
  DynamicJsonDocument doc(1024);
  doc["espState"] = "reset" ;
  String Response = doc.as<String>();
  mqttClient.publish(gPublish.c_str(), Response.c_str());
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  if ((WiFi.status() == WL_CONNECTED)) {
    if (mqttClient.connected()) {
      mqttClient.loop();
      sensorData();
      delay(1000);
    } else {
      Serial.println();
      connectWithMqtt();
    }
  }
  else
    connectWithWiFi();
}


void sensorData() {
  // read the state of the the input pin:
  currentState = digitalRead(SENSOR_PIN);

  if (lastState == LOW && currentState == HIGH) {
    Serial.println("The sensor is touched");
    digitalWrite(LED_PIN, HIGH);
    DynamicJsonDocument doc(1024);
    doc["switchNo"] = 1;
    doc["state"] = currentState;
    doc["module"] = "LED";
    String Response = doc.as<String>();
    mqttClient.publish(Publish.c_str(), Response.c_str());
  }
  // save the the last state
  else if (lastState == HIGH && currentState == LOW) {
    Serial.println("The sensor is not touched");
    digitalWrite(LED_PIN, LOW);
    DynamicJsonDocument doc(1024);
    doc["switchNo"] = 1;
    doc["state"] = currentState;
    doc["module"] = "LED";
    String Response = doc.as<String>();
    mqttClient.publish(Publish.c_str(), Response.c_str());
  }
  lastState = currentState;
}
