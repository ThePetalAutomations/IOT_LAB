/*Name:IOT Based Monitoring and Control for DC Motor
 * Description:The DC motor is controlled using 4 key switch
   and dashboard using IOT.
PINOUTS FOR 4 KEY SWITCH
  S1=GPIO5
  S2=GPIO17
  S3=GPIO16
  S4=GPIO4
PINOUTS FOR MOTOR DRIVER
  IN1 =GPIO25
  IN2 =GPIO26
  IN3 =GPIO27
  IN4 =GPIO14
*/
//==========================================================
#include "WiFi.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14

#define BUTTON1 5
#define BUTTON2 17
#define BUTTON3 16
#define BUTTON4 4

int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;
int buttonState4 = 0;

int previousButton1State = 0;
int previousButton2State = 0;
int previousButton3State = 0;
int previousButton4State = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

String KITNO = WiFi.macAddress();
String subTopic = "KIT/" + KITNO + "/DCMOTOR";
String motorPublish = KITNO + "/DCMOTOR";
String buttonPublish = KITNO + "/BUTTON";
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
  for (int n = addr_mqtt_user_name; n < addr_mqtt_user_name + 30; ++n) {
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
//=============================================================================================================
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, (const byte*)payload, length);
  Serial.println("deserialized");
  String motorState = doc["motorState"];
  Serial.print("motorState");
  Serial.println(motorState);
  if (motorState == "forward") {
    ForWard();
  }
  else if (motorState == "reverse") {
    BackWard();
  }
  else if (motorState == "stop")
    stopp();
}
void setup() {
  // put your setup code here, to run once:
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  pinMode(BUTTON4, INPUT);
  Serial.begin(115200);
  dataRead();
  connectWithWiFi();
  Serial.print("My Mac Address is: " + KITNO);
  DynamicJsonDocument doc(1024);
  doc["espState"] = "reset" ;
  String Response = doc.as<String>();
  mqttClient.publish(gPublish.c_str(), Response.c_str());
}
//=================================================================================================================
void loop() {
  // put your main code here, to run repeatedly:
  if ((WiFi.status() == WL_CONNECTED)) {
    if (mqttClient.connected()) {
      mqttClient.loop();
      driveLoop();
    } else {
      Serial.println();
      connectWithMqtt();
    }
  }
  else
    connectWithWiFi();
}
//=================================================================================================================
void driveLoop() {
  // put your main code here, to run repeatedly:
  buttonState1 = digitalRead(BUTTON1);
  buttonState2 = digitalRead(BUTTON2);
  buttonState3 = digitalRead(BUTTON3);
  buttonState4 = digitalRead(BUTTON4);
  Serial.print("buttonState1:");
  Serial.println(buttonState1);
  Serial.print("buttonState2:");
  Serial.println(buttonState2);
  Serial.print("buttonState3:");
  Serial.println(buttonState3);
  Serial.print("buttonState4:");
  Serial.println(buttonState4);
  if (!buttonState1 == previousButton1State)
  {
    previousButton1State = buttonState1;
    DynamicJsonDocument doc(1024);
    if (buttonState1 == 0) {
      doc["switchNo"] = "1";
      doc["switchState"] = buttonState1;
      String buttonResponse = doc.as<String>();
      Serial.println("buttonResponse");
      mqttClient.publish(buttonPublish.c_str(), buttonResponse.c_str());
      ForWard();
    }

  }
  else if (!buttonState2 == previousButton2State)
  {
    previousButton2State = buttonState2;
    DynamicJsonDocument doc(1024);
    if (buttonState2 == 0) {
      doc["switchNo"] = "2";
      doc["switchState"] = buttonState2;
      String buttonResponse = doc.as<String>();
      Serial.println("buttonResponse");
      mqttClient.publish(buttonPublish.c_str(), buttonResponse.c_str());
      BackWard() ;
    }

  }
  else if (!buttonState3 == previousButton3State)
  {
    previousButton3State = buttonState3;
    DynamicJsonDocument doc(1024);
    if (buttonState3 == 0) {
      doc["switchNo"] = "3";
      doc["switchState"] = buttonState3;
      String buttonResponse = doc.as<String>();
      Serial.println("buttonResponse");
      mqttClient.publish(buttonPublish.c_str(), buttonResponse.c_str());
      ForWard() ;
    }

  }

  else if (!buttonState4 == previousButton4State)
  {
    previousButton4State = buttonState4;
    DynamicJsonDocument doc(1024);
    if (buttonState4 == 0) {
      doc["switchNo"] = "4";
      doc["switchState"] = buttonState4;
      String buttonResponse = doc.as<String>();
      Serial.println("buttonResponse");
      mqttClient.publish(buttonPublish.c_str(), buttonResponse.c_str());
      stopp();
    }
  }
}

//==========================================================================================================
void ForWard() {
  DynamicJsonDocument doc(1024);
  doc["motorState"] = "forward" ;
  String relayResponse = doc.as<String>();
  Serial.println("relayResponse");
  mqttClient.publish(motorPublish.c_str(), relayResponse.c_str());
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void BackWard() {
  DynamicJsonDocument doc(1024);
  doc["motorState"] = "backward" ;
  String relayResponse = doc.as<String>();
  Serial.println("relayResponse" );
  mqttClient.publish(motorPublish.c_str(), relayResponse.c_str());
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(2000);
}
void stopp() {
  DynamicJsonDocument doc(1024);
  doc["motorState"] = "stop" ;
  String relayResponse = doc.as<String>();
  Serial.println("relayResponse");
  mqttClient.publish(motorPublish.c_str(), relayResponse.c_str());
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(2000);
}
