/*
Name:RTC Based Load Control using IOT
Description:To learn how to get real time using RTC module, display message on 7 segment display and controlling 
the relay using ESP32. And learn how to control load based the real time through IOT
PINOUTS FOR RTC
  SCL=GPIO22
  SDA=GPIO23
PINOUTS FOR 7 SEGMENT LED
  DIO= GPIO19
  CLK= GPIO18
  SDA= GPIO23
PINOUT FOR RELAY
  IN= GPIO17
*/
//================================================================
#include "WiFi.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "RTClib.h"
#include <Wire.h>
#include <TM1637Display.h>
#define Relay 17
boolean setting = false;
int startTime=01;
int stopTime=01;
int displaytime;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
String KITNO = WiFi.macAddress();
String Publish =  KITNO + "/7SEGMENT";
String rPublish =  KITNO + "/RTC";
String relayPublish =  KITNO + "/RELAY";
String subTopic = "KIT/" + KITNO + "/RTC";
String gPublish =  KITNO + "/GENERAL";
// Define the connections pins for TM1637 4 digit 7 segment display
#define CLK 18
#define DIO 19

// Create rtc and display object
RTC_DS1307 rtc;
TM1637Display display = TM1637Display(CLK, DIO);
//======================================================================================================
//intialization for read from memory
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
//=================================================================================================================
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
//===============================================================================================================
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
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, (const byte*)payload, length);
  Serial.println("deserialized");
  int startHour = doc["startHour"];
  int startMinute = doc["startMinute"];
  int stopHour = doc["stopHour"];
  int stopMinute = doc["stopMinute"];
  Serial.println(startHour);
  Serial.println(startMinute);
  Serial.println(stopHour);
  Serial.println(stopMinute);
  startTime = (startHour * 100) + startMinute;
  stopTime = (stopHour * 100) + stopMinute;
  if (startTime == displaytime) {
    digitalWrite(Relay, HIGH);
    DynamicJsonDocument doc(1024);
    doc["relayState"] = "1";
    String Response = doc.as<String>();
    mqttClient.publish(relayPublish.c_str(), Response.c_str());
  }
  else if (stopTime == displaytime) {
    digitalWrite(Relay, LOW);
    DynamicJsonDocument doc(1024);
    doc["relayState"] = "0";
    String Response = doc.as<String>();
    mqttClient.publish(relayPublish.c_str(), Response.c_str());
  }
}
void setup() {
  // Begin serial communication at a baud rate of 9600
  Serial.begin(115200);
  dataRead();
  connectWithWiFi();
  DynamicJsonDocument doc(1024);
  doc["espState"] = "reset" ;
  String Response = doc.as<String>();
  mqttClient.publish(gPublish.c_str(), Response.c_str());
  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, LOW);
  // Check if RTC is connected correctly
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  // Check if the RTC lost power and if so, set the time:
  if (!rtc.isrunning()) {
    Serial.println("RTC lost power,lets set the time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  display.setBrightness(7);
  display.clear();
}
void loop() {
  // put your main code here, to run repeatedly:
  if ((WiFi.status() == WL_CONNECTED)) {
    if (mqttClient.connected()) {
      mqttClient.loop();
      getTime();
    } else {
      Serial.println();
      connectWithMqtt();
    }
  }
  else
    connectWithWiFi();
}

void getTime() {
  DateTime now = rtc.now();
  String Time = String(now.hour()) + ":" + String((now.minute()) + 00);
  Serial.println(Time);
  displaytime = (now.hour() * 100) + now.minute();
  Serial.println(displaytime);
  display.showNumberDecEx(displaytime, 0b11100000, true);
  display.showNumberDec(displaytime, true); // Prints displaytime without center colon.
  DynamicJsonDocument doc(1024);
  doc["rtcTime"] = Time;
  String Response = doc.as<String>();
  mqttClient.publish(rPublish.c_str(), Response.c_str());
  DynamicJsonDocument root(1024);
  root["time"] = Time;
  String Res = root.as<String>();
  mqttClient.publish(Publish.c_str(), Res.c_str());

  if (startTime == displaytime) {
    digitalWrite(Relay, HIGH);
    DynamicJsonDocument doc(1024);
    doc["relayState"] = "1";
    String Response = doc.as<String>();
    mqttClient.publish(relayPublish.c_str(), Response.c_str());
  }
  else if (stopTime == displaytime) {
    digitalWrite(Relay, LOW);
    DynamicJsonDocument doc(1024);
    doc["relayState"] = "0";
    String Response = doc.as<String>();
    mqttClient.publish(relayPublish.c_str(), Response.c_str());
  }

}
