/*
  Name:Blinking LED’s
  Description:
  8 LED'S are blinking with delay and it will be upload to dashboard also.
  PINOUTS:
  *LED1= GPIO15
  *LED2= GPIO2
  *LED3= GPIO4
  *LED4= GPIO16
  *LED5= GPIO17
  *LED6= GPIO5
  *LED7= GPIO18
  *LED8= GPIO19
 */
#include "WiFi.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


WiFiClient espClient;
PubSubClient mqttClient(espClient);

String KITNO = WiFi.macAddress();
String subTopic = "KIT/" + KITNO + "/LEDBLINK";
String Publish =  KITNO + "/LEDBLINK";
String gPublish =  KITNO + "/GENERAL";

#define LED1 15
#define LED2 2
#define LED3 4
#define LED4 16
#define LED5 17
#define LED6 5
#define LED7 18
#define LED8 19

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
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dataRead();
  configure();
  connectWithWiFi();
  DynamicJsonDocument doc(1024);
  doc["espState"] = "reset" ;
  String Response = doc.as<String>();
  mqttClient.publish(gPublish.c_str(), Response.c_str());
  Serial.print("My Mac Address is: " + KITNO);
}
void configure() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  pinMode(LED7, OUTPUT);
  pinMode(LED8, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  digitalWrite(LED6, LOW);
  digitalWrite(LED7, LOW);
  digitalWrite(LED8, LOW);
}
void loop() {
  // put your main code here, to run repeatedly:
  if ((WiFi.status() == WL_CONNECTED)) {
    if (mqttClient.connected()) {
      mqttClient.loop();
      LEDON();
      delay(1000);
      LEDOFF();
      delay(1000);
    } else {
      Serial.println();
      connectWithMqtt();
    }
  }
  else
    connectWithWiFi();
}

void LEDON() {
  DynamicJsonDocument doc(1024);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
  digitalWrite(LED7, HIGH);
  digitalWrite(LED8, HIGH);

  doc["led1State"] = "1";
  doc["led2State"] = "1";
  doc["led3State"] = "1";
  doc["led4State"] = "1";
  doc["led5State"] = "1";
  doc["led6State"] = "1";
  doc["led7State"] = "1";
  doc["led8State"] = "1";
  String Response = doc.as<String>();
  mqttClient.publish(Publish.c_str(), Response.c_str());
  Serial.println(Response);
}
void LEDOFF() {
  DynamicJsonDocument doc(1024);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  digitalWrite(LED6, LOW);
  digitalWrite(LED7, LOW);
  digitalWrite(LED8, LOW);
  doc["led1State"] = "0";
  doc["led2State"] = "0";
  doc["led3State"] = "0";
  doc["led4State"] = "0";
  doc["led5State"] = "0";
  doc["led6State"] = "0";
  doc["led7State"] = "0";
  doc["led8State"] = "0";
  String Response = doc.as<String>();
  mqttClient.publish(Publish.c_str(), Response.c_str());
  Serial.println(Response);
}
