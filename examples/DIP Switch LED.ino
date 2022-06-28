/*Name:Controlling LED Using Switch 
 *Description:Using 8xchannel dip switch we can control 8xchannel LED's.
 *LED1= GPIO15       SWITCH1=GPIO14
 *LED2= GPIO2        SWITCH2=GPIO27
 *LED3= GPIO4        SWITCH3=GPIO26
 *LED4= GPIO16       SWITCH4=GPIO25
 *LED5= GPIO17       SWITCH5=GPIO33
 *LED6= GPIO5        SWITCH6=GPIO32
 *LED7= GPIO18       SWITCH7=GPIO35
 *LED8= GPIO19      SWITCH8=GPIO34
//=====================================================================================================*/
#include "WiFi.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

String KITNO = WiFi.macAddress();
String swPublish = KITNO + "/SWITCH" ;
String ledPublish = KITNO + "/LED" ;
String gPublish =  KITNO + "/GENERAL";


#define LED1 15
#define LED2 2
#define LED3 4
#define LED4 16
#define LED5 17
#define LED6 5
#define LED7 18
#define LED8 19


#define SW1 14
#define SW2 27
#define SW3 26
#define SW4 25
#define SW5 33
#define SW6 32
#define SW7 35
#define SW8 34

int s1state;
int s2state;
int s3state;
int s4state;
int s5state;
int s6state;
int s7state;
int s8state;

int previousSw1State=1;
int previousSw2State=1;
int previousSw3State=1;
int previousSw4State=1;
int previousSw5State=1;
int previousSw6State=1;
int previousSw7State=1;
int previousSw8State=1;

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
      // mqttClient.setCallback(callback);
      // mqttClient.subscribe(subTopic.c_str());
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
}

void configure() {
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(SW4, INPUT);
  pinMode(SW5, INPUT);
  pinMode(SW6, INPUT);
  pinMode(SW7, INPUT);
  pinMode(SW8, INPUT);

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
      control();
    } else {
      Serial.println();
      connectWithMqtt();
    }
  }
  else
    connectWithWiFi();
}

void control() {
  // put your main code here, to run repeatedly:
  s1state = digitalRead(SW1);
  s2state = digitalRead(SW2);
  s3state = digitalRead(SW3);
  s4state = digitalRead(SW4);
  s5state = digitalRead(SW5);
  s6state = digitalRead(SW6);
  s7state = digitalRead(SW7);
  s8state = digitalRead(SW8);

  if (!s1state == previousSw1State) {
    previousSw1State = s1state;
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    if (s1state == 0) {
      doc["switchNo"] = "1";
      doc["switchState"] = s1state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED1, HIGH);//led on
      root["ledNo"] = "1";
      root["state"] = "1";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
    else {

      doc["switchNo"] = "1";
      doc["switchState"] = s1state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED1, LOW);//led on
      root["ledNo"] = "1";
      root["state"] = "0";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
  }
  else if (!s2state == previousSw2State) {
    previousSw2State = s2state;
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    if (s2state == 0) {
      doc["switchNo"] = "2";
      doc["switchState"] = s2state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED2, HIGH);//led on
      root["ledNo"] = "2";
      root["state"] = "1";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
    else {
      doc["switchNo"] = "2";
      doc["switchState"] = s2state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED2, LOW);//led on
      root["ledNo"] = "2";
      root["state"] = "0";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
  }
  else if (!s3state == previousSw3State) {
    previousSw3State = s3state;
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    if (s3state == 0) {
      doc["switchNo"] = "3";
      doc["switchState"] = s3state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED3, HIGH);//led on
      root["ledNo"] = "3";
      root["state"] = "1";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
    else {
      doc["switchNo"] = "3";
      doc["switchState"] = s3state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED3, LOW);//led on
      root["ledNo"] = "3";
      root["state"] = "0";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
  }
  else if (!s4state == previousSw4State) {
    previousSw4State = s4state;
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    if (s4state == 0) {
      doc["switchNo"] = "4";
      doc["switchState"] = s4state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED4, HIGH);//led on
      root["ledNo"] = "4";
      root["state"] = "1";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
    else {

      doc["switchNo"] = "4";
      doc["switchState"] = s4state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED4, LOW);//led on
      root["ledNo"] = "4";
      root["state"] = "0";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
  }
  else if (!s5state == previousSw5State) {
    previousSw5State = s5state;
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    if (s5state == 0) {
      doc["switchNo"] = "5";
      doc["switchState"] = s5state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED5, HIGH);//led on
      root["ledNo"] = "5";
      root["state"] = "1";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
    else {

      doc["switchNo"] = "5";
      doc["switchState"] = s5state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED5, LOW);//led on
      root["ledNo"] = "5";
      root["state"] = "0";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
  }
  else if (!s6state == previousSw6State) {
    previousSw6State = s6state;
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    if (s6state == 0) {
      doc["switchNo"] = "6";
      doc["switchState"] = s6state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED6, HIGH);//led on
      root["ledNo"] = "6";
      root["state"] = "1";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
    else {

      doc["switchNo"] = "6";
      doc["switchState"] = s6state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED6, LOW);//led on
      root["ledNo"] = "6";
      root["state"] = "0";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
  }
  else if (!s7state == previousSw7State) {
    previousSw7State = s7state;
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    if (s7state == 0) {
      doc["switchNo"] = "7";
      doc["switchState"] = s7state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED7, HIGH);//led on
      root["ledNo"] = "7";
      root["state"] = "1";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
    else {

      doc["switchNo"] = "7";
      doc["switchState"] = s7state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED7, LOW);//led on
      root["ledNo"] = "7";
      root["state"] = "0";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
  }
  else if (!s8state == previousSw8State) {
    previousSw8State = s8state;
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument root(1024);
    if (s8state == 0) {
      doc["switchNo"] = "8";
      doc["switchState"] = s8state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED8, HIGH);//led on
      root["ledNo"] = "8";
      root["state"] = "1";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
    else {

      doc["switchNo"] = "8";
      doc["switchState"] = s8state;
      String swResponse = doc.as<String>();
      Serial.println(swResponse);
      mqttClient.publish(swPublish.c_str(), swResponse.c_str());
      digitalWrite(LED8, LOW);//led on
      root["ledNo"] = "8";
      root["state"] = "0";
      String ledResponse = root.as<String>();
      Serial.println(ledResponse);
      mqttClient.publish(ledPublish.c_str(), ledResponse.c_str());
    }
  }
}
