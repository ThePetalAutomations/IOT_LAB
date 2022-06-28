/*
 * This code for configure wifi setting and mqtt settings.Whenever you need to 
change your password you should upload this code.
//========================================================================*/
#include "WiFi.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#undef  MQTT_MAX_PACKET_SIZE 
#define MQTT_MAX_PACKET_SIZE 500

String myMacAddress;
String msg;
boolean is_configuration_mode;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
AsyncWebServer asyncWebServer(80);

String WIFI_SSID = "0";
String WIFI_PASSWORD = "0";
String MQTT_HOST = "0";
String MQTT_USER_NAME = "0";
String MQTT_PASSWORD = "0";
String MQTT_PORT = "0";


int addr_wifi_ssid = 50;
int addr_wifi_password = 70;
int addr_mqtt_host =90;
int addr_mqtt_user_name = 110;
int addr_mqtt_password = 130;
int addr_mqtt_port = 150;

const char * serverSSID = "IOT-LAB-KIT";
const char * serverPassword = "Petal#2021";

const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>
 <head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Registration Page</title>
  <style>
   label {
     display: inline-block;
     width: 140px;
     font-size: 15px;
   }
   input {
     border: 1px solid green;
     border-radius: 3px;
     width: 200px;
     height: 20px;
     font-size: 15px;
   }
   button {
     background-color: green;
     border: none;
     color: white;
     border-radius: 3px;
     width: 340px;
     height: 35px;
     text-align: center;
     text-decoration: none;
     display: inline-block;
     font-size: 15px;
     margin: 4px 2px;
     transition-duration: 0.4s;
     cursor: pointer;
   }
  </style>
 </head>
 <body onload="getValues()">
  <br />
  <br />

  <label> WiFi-SSID: </label>
  <input type="text" id="wifiSSID" name="wifiSSID" value="" size="15" /> 
  <br />
  <br />

  <label> WiFi-Password: </label>
  <input type="text" id="wifiPassword" name="wifiPassword" value="" size="15" />
  <br />
  <br />

  <label> Endpoint Address: </label>
  <input type="text" id="mqttHost" name="mqttHost" value="" size="15" />
  <br />
  <br />

  <label> Mqtt User Name: </label>
  <input type="text" id="mqttUserName" name="mqttUserName" value="" size="15" />
  <br />
  <br />

  <label> Mqtt Password: </label>
  <input type="text" id="mqttPassword" name="mqttPassword" value="" size="15" />
  <br />
  <br />

  <label> Mqtt Port: </label>
  <input type="text" id="mqttPort" name="mqttPort" value="" size="15" />
  <br />
  <br />


  <button onclick="setValues();">WRITE</button>

  <br />
  <br />
  <br />
  <br />
  <br />
  <br />
  <br />
  <br />

  <label id="status"> </label>
 </body>
 <script>

   function getValues(){
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
           const data = JSON.parse(xhttp.responseText);
           document.querySelector("#wifiSSID").value = data.wifiSsid;
           document.querySelector("#wifiPassword").value = data.wifiPassword;
           document.querySelector("#mqttHost").value = data.mqttHost;
           document.querySelector("#mqttUserName").value = data.mqttUserName;
           document.querySelector("#mqttPassword").value = data.mqttPassword;
           document.querySelector("#mqttPort").value = data.mqttPort;
           
        }
    };
    xhttp.open("GET", "/getConfiguration", true);
    xhttp.send();
  }

  function setValues(){
    var wifiSsid = document.querySelector("#wifiSSID").value;
    var wifiPassword = document.querySelector("#wifiPassword").value;
    var mqttHost = document.querySelector("#mqttHost").value;
    var mqttUserName = document.querySelector("#mqttUserName").value;
    var mqttPassword = document.querySelector("#mqttPassword").value;
    var mqttPort = document.querySelector("#mqttPort").value;


    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
           document.getElementById("status").innerHTML = xhttp.responseText;
        }
        else {
           document.getElementById("status").value = "Failed";
        }
    };
    var url = "/dataToWrite?wifiSsid="+wifiSsid+"&wifiPassword="+encodeURIComponent(wifiPassword)+"&mqttHost="+mqttHost+"&mqttUserName="+mqttUserName+"&mqttPassword="+encodeURIComponent(mqttPassword)+"&mqttPort="+mqttPort;
    xhttp.open("POST",url ,true);
    xhttp.send();
  }
 </script>
</html>
)rawliteral";

void connectWithSAP() {
  Serial.print("\nSetting AP Soft Access Point…");
  Serial.println(WiFi.softAP(serverSSID, serverPassword) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  asyncWebServer.begin();
  asyncServerApis();
}

void notFound(AsyncWebServerRequest * request) {
  request -> send(404, "text/plain", "Not found");
}

void asyncServerApis() {
  asyncWebServer.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.print("\nRequested Main page Data");
    request -> send_P(200, "text/html", index_html);
  });

  asyncWebServer.on("/getConfiguration", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.print("\nRequested Configuration Data");
    DynamicJsonDocument doc(1024);
    doc["wifiSsid"] = readWifiSsidFromMemory();
    doc["wifiPassword"] = readWifiPasswordFromMemory();
    doc["mqttHost"] = readMqttHostFromMemory();
    doc["mqttUserName"] = readMqttUserNameFromMemory();
    doc["mqttPassword"] = readMqttPasswordFromMemory();
    doc["mqttPort"] = readMqttPortFromMemory();
    String jsonResponse = doc.as<String>();
    Serial.println(jsonResponse);
    request->send_P(200, "text/plain", jsonResponse.c_str());
  });

  asyncWebServer.on("/dataToWrite", HTTP_POST, [](AsyncWebServerRequest * request) {
    WIFI_SSID = request -> getParam("wifiSsid") -> value();
    WIFI_PASSWORD = request -> getParam("wifiPassword") -> value();
    MQTT_HOST = request -> getParam("mqttHost") -> value();
    MQTT_USER_NAME = request -> getParam("mqttUserName") -> value();
    MQTT_PASSWORD = request -> getParam("mqttPassword") -> value();
    MQTT_PORT = request -> getParam("mqttPort") -> value();

    Serial.println(WIFI_SSID);
    Serial.println(WIFI_PASSWORD);
    Serial.println(MQTT_HOST);
    Serial.println(MQTT_USER_NAME);
    Serial.println(MQTT_PASSWORD);
    Serial.println(MQTT_PORT);
    request -> send_P(200, "text/plain", "Success");
    
    insertDataToMemory(WIFI_SSID, WIFI_PASSWORD, MQTT_HOST, MQTT_USER_NAME, MQTT_PASSWORD, String(MQTT_PORT));
    is_configuration_mode = true;
  });
}
//=================================================setup================================
void setup() {
 Serial.begin(115200);
 EEPROM.begin(512); 
 connectWithSAP();
 connectWithWiFi();
 myMacAddress = WiFi.macAddress();
 Serial.print("My Mac Address is: "+myMacAddress);
}
//============================================connect to WiFi===============================
void connectWithWiFi(){
    long connectionTimeWait = 0;
    long connectionTimeOut = 15000;
    String read_ssid = readWifiSsidFromMemory();
    String read_password = readWifiPasswordFromMemory();

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
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("Connected to the WiFi network with IP: ");
      Serial.println(WiFi.localIP());
      connectionTimeWait = 0;
      connectWithMqtt();
    }
    else{
      Serial.println("Cannot connect to the WiFi network");
      Serial.println(WiFi.localIP());
    }
}
//==================================================Connect to MQTT================================
void connectWithMqtt(){
  String mqtt_endpoint = readMqttHostFromMemory();
  Serial.println(mqtt_endpoint);
  const char *read_mqtt_server = mqtt_endpoint.c_str();
  int read_mqtt_port = atoi(readMqttPortFromMemory().c_str());
  String mqtt_user_name = readMqttUserNameFromMemory();
  String mqtt_password = readMqttPasswordFromMemory();
    
  Serial.println("MQTT connecting to address");
  Serial.println(read_mqtt_server);
  Serial.println("With port");
  Serial.println(read_mqtt_port);

  mqttClient.setServer(read_mqtt_server, read_mqtt_port);
  mqttClient.setKeepAlive(60);
  
  Serial.println("Connecting to MQTT...");
  String clientId = myMacAddress;
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(clientId.c_str(), mqtt_user_name.c_str(), mqtt_password.c_str())) {
      Serial.println("connected");
      mqttClient.subscribe(myMacAddress.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}
//===================================================Write to the memory================================================

void insertDataToMemory(String wifiSsid, String wifiPassword, String mqttHost, String mqttUserName, String mqttPassword, String mqttPort){
  for (int i = addr_wifi_ssid; i < 512; i++){ 
    EEPROM.write(i, 0); 
  }
  for (int i = 0; i < wifiSsid.length(); i++){
    EEPROM.write(addr_wifi_ssid + i, wifiSsid[i]);
  }
  for (int i = 0; i < wifiPassword.length(); i++){
    EEPROM.write(addr_wifi_password + i, wifiPassword[i]);
  }
  for (int i = 0; i < mqttHost.length(); i++){
    EEPROM.write(addr_mqtt_host + i, mqttHost[i]);
  }
  for (int i = 0; i < mqttUserName.length(); i++){
    EEPROM.write(addr_mqtt_user_name + i, mqttUserName[i]);
  }
  for (int i = 0; i < mqttPassword.length(); i++){
    EEPROM.write(addr_mqtt_password + i, mqttPassword[i]);
  }
  for (int i = 0; i < mqttPort.length(); i++){
    EEPROM.write(addr_mqtt_port + i, mqttPort[i]);
  }
  if (EEPROM.commit()) {
    Serial.println("Data successfully committed");
  }else {
    Serial.println("ERROR! Data commit failed");
  }
}

//-----------------------------------READING-FROM-MEMORY-----------------------------------

String readWifiSsidFromMemory(){
  String wifiSsidData;
  for (int m = addr_wifi_ssid; m < addr_wifi_ssid + 20; ++m){
    wifiSsidData += char(EEPROM.read(m));
  }
  return wifiSsidData;
}

String readWifiPasswordFromMemory(){
  String wifiPasswordData;
  for (int n = addr_wifi_password; n < addr_wifi_password + 20; ++n){
    wifiPasswordData += char(EEPROM.read(n));
  }
  return wifiPasswordData;
}

String readMqttHostFromMemory(){
  String mqttHost;
  for (int n = addr_mqtt_host; n < addr_mqtt_host + 20; ++n){
    mqttHost += char(EEPROM.read(n));
  }
  return mqttHost;
}
String readMqttUserNameFromMemory(){
  String mqttUsername;
  for (int n = addr_mqtt_user_name; n < addr_mqtt_user_name + 20; ++n){
    mqttUsername += char(EEPROM.read(n));
  }
  return mqttUsername;
}

String readMqttPasswordFromMemory(){
  String mqttPassword;
  for (int n = addr_mqtt_password; n < addr_mqtt_password + 20; ++n){
    mqttPassword += char(EEPROM.read(n));
  }
  return mqttPassword;
}
String readMqttPortFromMemory(){
  String mqttPort;
  for (int n = addr_mqtt_port; n < addr_mqtt_port + 20; ++n){
    mqttPort += char(EEPROM.read(n));
  }
  return mqttPort;
}
//=============================loop===============================================
void loop() {
  if(is_configuration_mode){
    Serial.println("Configuration Mode enabled");
    is_configuration_mode = false;
    connectWithWiFi();
  }else{
    if((WiFi.status() == WL_CONNECTED)){
      if(mqttClient.connected()){
        mqttClient.loop();
     //you can add your code to run
      }else{
        Serial.println();
        connectWithMqtt();
      }
    }else{
      connectWithWiFi();
    }
  } 
}
