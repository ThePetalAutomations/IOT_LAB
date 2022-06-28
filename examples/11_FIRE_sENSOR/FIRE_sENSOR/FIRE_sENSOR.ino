/*
Name:IOT Based Fire Alarm System
Description:When heat or fire is detected relay will be turned ON and message will be shown in LCD display 
 it alsoshown in dashboard also you can monitoring the sensor and relay status
 PINOUTS
 FireSensor O/P = GPIO16
 Relay IN       = GPIO17
 PINOUTS FOR LCD DISPLAY
 * LCD RS pin to digital pin 13
 * LCD Enable pin to digital pin 12
 * LCD D1 pin to digital pin 14
 * LCD D2 pin to digital pin 27
 * LCD D3 pin to digital pin 26
 * LCD D4 pin to digital pin 25
 */
//========================================================================
#include <LiquidCrystal.h>
#include "WiFi.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#define sensorPin 16
#define relayPin 17
int sensorPinState = 0;
int previousState = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

String KITNO = WiFi.macAddress();
String Publish =  KITNO + "/FIRE";
String gPublish =  KITNO + "/GENERAL";
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

const int rs = 13, en = 12, d4 = 14, d5 = 27, d6 = 26, d7 = 25;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
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
  for (int m = addr_wifi_ssid; m < addr_wifi_ssid + 50; ++m) {
    wifiSsidData += char(EEPROM.read(m));
  }
  return wifiSsidData;
}

String readWifiPasswordFromMemory() {
  String wifiPasswordData;
  for (int n = addr_wifi_password; n < addr_wifi_password + 30; ++n) {
    wifiPasswordData += char(EEPROM.read(n));
  }
  return wifiPasswordData;
}

String readMqttHostFromMemory() {
  String mqttHost;
  for (int n = addr_mqtt_host; n < addr_mqtt_host + 30; ++n) {
    mqttHost += char(EEPROM.read(n));
  }
  return mqttHost;
}

String readMqttPortFromMemory() {
  String mqttPort;
  for (int n = addr_mqtt_port; n < addr_mqtt_port + 10; ++n) {
    mqttPort += char(EEPROM.read(n));
  }
  return mqttPort;
}

String readMqttUserNameFromMemory() {
  String mqttUsername;
  for (int n = addr_mqtt_user_name; n < addr_mqtt_user_name + 30; ++n) {
    mqttUsername += char(EEPROM.read(n));
  }
  return mqttUsername;
}

String readMqttPasswordFromMemory() {
  String mqttPassword;
  for (int n = addr_mqtt_password; n < addr_mqtt_password + 30; ++n) {
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
      //mqttClient.setCallback(callback);
      // mqttClient.subscribe(subTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}
//===============================================================================================================
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dataRead();
  connectWithWiFi();
  DynamicJsonDocument doc(1024);
  doc["espState"] = "reset" ;
  String Response = doc.as<String>();
  mqttClient.publish(gPublish.c_str(), Response.c_str());
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IOT LAB KIT");
  pinMode(sensorPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

}
//===============================================================================================================
void loop() {
  // put your main code here, to run repeatedly:
  if ((WiFi.status() == WL_CONNECTED)) {
    if (mqttClient.connected()) {
      mqttClient.loop();
      ldrLoop();
    } else {
      Serial.println();
      connectWithMqtt();
    }
  }
  else
    connectWithWiFi();
}
//===============================================================================================================
void ldrLoop() {
  // put your main code here, to run repeatedly:
  sensorPinState = digitalRead(sensorPin);
  if (!sensorPinState == previousState) {
    previousState = sensorPinState;
    if (sensorPinState == 0 )
    {
      DynamicJsonDocument doc(1024);
      doc["fireState"] = sensorPinState;
      doc["lcdContent"] = "Flame detected...! take action immediately.";
      doc["relayState"] = "1";
      String Response = doc.as<String>();
      mqttClient.publish(Publish.c_str(), Response.c_str());
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Flame detected.!");
      lcd.setCursor(0, 1);
      lcd.print(" take action....");
      Serial.println("Flame detected...! take action immediately." );
      digitalWrite(relayPin, HIGH);

      previousState = sensorPinState;
    }
    else if (sensorPinState == 1 )
    {
      DynamicJsonDocument doc(1024);
      doc["fireState"] = sensorPinState;
      doc["lcdContent"] = "No flame detected. stay cool";
      doc["relayState"] = "0";
      String Response = doc.as<String>();
      mqttClient.publish(Publish.c_str(), Response.c_str());
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("No flame detect.");
      lcd.setCursor(0, 1);
      lcd.print("stay cool");
      Serial.println("No flame detected. stay cool ");
      digitalWrite(relayPin, LOW);
      previousState = sensorPinState;
    }
  }
}
