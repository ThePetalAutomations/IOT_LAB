/*
  Name:Obstacle Alarm
  Description:Obstacle Alarm
  PINOUTS FOR IR AND BUZZER
  IRsensor O/P= GPIO5
  Buzzer I/P= GPIO4
  ====================================================================================================================*/
#include "WiFi.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
String KITNO = WiFi.macAddress();
String module1 = "/IR";
String module2 = "/BUZZER";
String iPublish = KITNO + module1 ;
String rPublish = KITNO + module2;
int buzzerState;
String Response;
String irResponse;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
AsyncWebServer asyncWebServer(80);

//====================================================================================================================*/
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

#define IRsensor 5
#define Buzzer 4
int IRstate = 0;
//========================================================================================================================================================================
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(iPublish);
  dataRead();
  connectWithWiFi();
  pinMode(IRsensor, INPUT);
  pinMode(Buzzer, OUTPUT);

}
//====================================================================================================================
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
//====================================================================================================================
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
//====================================================================================================================
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
//====================================================================================================================*/
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
      mqttClient.subscribe(KITNO.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}
//====================================================================================================================*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  StaticJsonDocument<256> doc;
  deserializeJson(doc, (const byte*)payload, length);
  Serial.println("deserialized");
  String msg = doc["msg"];
  Serial.println(msg);
}
//====================================================================================================================*/
void loop() {
  // put your main code here, to run repeatedly:
  if ((WiFi.status() == WL_CONNECTED)) {
    if (mqttClient.connected()) {
      mqttClient.loop();
      IRloop();
    } else {
      Serial.println();
      connectWithMqtt();
    }
  }
  else
    connectWithWiFi();
}
//====================================================================================================================*/
void IRloop() {

  IRstate = digitalRead(IRsensor);
  if (IRstate == LOW) {
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    buzzerState = 1;
    doc["irState"] = IRstate;
    String irResponse = doc.as<String>();
    Serial.println(irResponse);
    mqttClient.publish(iPublish.c_str(), irResponse.c_str());
    root["buzzerState"] = buzzerState;
    String Response = root.as<String>();
    Serial.println(Response);
    mqttClient.publish(rPublish.c_str(), Response.c_str());
    digitalWrite(Buzzer, HIGH);
    delay(1000);
    Serial.println("*************");
  }
  else
  {
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    buzzerState = 0;
    doc["irState"] = IRstate;
    String irResponse = doc.as<String>();
    Serial.println(irResponse);
    mqttClient.publish(iPublish.c_str(), irResponse.c_str());
    root["buzzerState"] = buzzerState;
    String Response = root.as<String>();
    Serial.println(Response);
    mqttClient.publish(rPublish.c_str(), Response.c_str());
    digitalWrite(Buzzer, LOW);
    delay(1000);
  }
}//
