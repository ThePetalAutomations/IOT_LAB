/*
Name:Print Text
Description:Create meg using 4x4 matrix keypad and sent it to dashboard.

PINOUTS FOR 4X4 MATRIX KEYPAD
R1=GPIO23  C1=GPIO19
R2=GPIO22  C2=GPIO18
R3=GPIO3   C3=GPIO5
R4=GPIO21  C4=GPIO17

PINOUTS FOR LCD DISPLAY
 * LCD RS pin to digital pin 13
 * LCD Enable pin to digital pin 12
 * LCD D1 pin to digital pin 14
 * LCD D2 pin to digital pin 27
 * LCD D3 pin to digital pin 26
 * LCD D4 pin to digital pin 25
*/
#include "WiFi.h"
#include "Keypad.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "LiquidCrystal.h"

#define DEFAULT_DELAY 300

WiFiClient espClient;
PubSubClient mqttClient(espClient);

String KITNO = WiFi.macAddress();
String Publish =  KITNO + "/LCDDISPLAY";
String gPublish =  KITNO + "/GENERAL";

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

int x = 0;                // Holds the LCD x position
int y = 0;                // Holds the LCD y position
int minValue = 0;         // Lower character location for T9 text entry
int maxValue = 0;         // Max character location for T9 text entry
int keyPressTime = 200;   // Number of loops check of the key
String msg = "";          // Holds the created message
String num = "";          // Holds the mobile number
String alpha = "!@_$%?1 ABC2 DEF3 GHI4 JKL5 MNO6 PQRS7 TUV8 WXYZ9 *  #0"; // Characters for T9 text entry

char hexaKeys[ROWS][COLS] = { // Character matrix for the keypad
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {23, 22, 3, 21};        // pin assignments for keypad rows
byte colPins[COLS] = {19, 18, 5, 17};        // pin assignments for keypad columns

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(13, 12, 14, 27, 26, 25);  // pin assignments for LCD

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
//=================================================================================================
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dataRead();
  connectWithWiFi();
  Serial.print("My Mac Address is: " + KITNO);
  DynamicJsonDocument doc(1024);
  doc["espState"] ="reset" ;
  String Response = doc.as<String>();
  mqttClient.publish(gPublish.c_str(), Response.c_str());
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("For Text  ");
  lcd.setCursor(0, 1);
  lcd.print("  Press D  ");
}
//=============================================================================================

void loop() {
// put your main code here, to run repeatedly:
  if ((WiFi.status() == WL_CONNECTED)) {
    if (mqttClient.connected()) {
      mqttClient.loop();
      processkey();
    } else {
      Serial.println();
      connectWithMqtt();
    }
  }
  else
    connectWithWiFi();
}
}
//================================================================================================
void processkey() {
  char key = customKeypad.getKey();
  if (isAlpha(key)) {   // check if key press is a letter
    enterMSG();  // process it according to keys
  }
}
void enterMSG() {
  char key;
  lcd.clear();  // clear the LCD display
  x = 0;        // init the x position to zero
  y = 0;        // init the y position to zero
  msg = "";     // clear the msg variable

  do {
    key = customKeypad.getKey();
    if(key == '1') {    // if a key is pressed,
      parseKey(0, 7, key);        // compare it to the alpha string array
    } else if (key == '2') {
      parseKey(8, 12, key);
    } else if (key == '3') {
      parseKey(13, 17, key);
    } else if (key == '4') {
      parseKey(18, 22, key);
    } else if (key == '5') {
      parseKey(23, 27, key);
    } else if (key == '6') {
      parseKey(28, 32, key);
    } else if (key == '7') {
      parseKey(33, 38, key);
    } else if (key == '8') {
      parseKey(39, 43, key);
    } else if (key == '9') {
      parseKey(44, 49, key);
    } else if (key == '0') {
      parseKey(52, 54, key);
    } else if (key == '*') {
      parseKey(50, 51, key);
    } else if (key == '#') {
      // do nothing
    }
  } while (key != '#');       // exit the loop when # is pressed

  lcd.setCursor(0, 0);        // these are for verification only
  lcd.print("created msg");   // feel free to modify it and
  lcd.setCursor(0, 1);        // adapt to your specific requirements
  lcd.print(msg);
  DynamicJsonDocument doc(1024);
  doc["content"] = msg;
  String Response = doc.as<String>();
  mqttClient.publish(Publish.c_str(), Response.c_str());
  delay(2000);
}

void parseKey(int minValue, int maxValue, char keyPress) {
  int ch = minValue;
  char key = keyPress;
  if (keyPress == '*') {              // if *, means backspace
    if ( (x > 0) || (y > 0) ) {       // prevent backspace when no character yet
      x = x - 1;                      // go back to previous character position
      lcd.setCursor(x, y);            // set the new lcd position
      lcd.print("*");                 // write *, which means for editing
      msg.remove(msg.length() - 1);   // remove the last character from the string
    }
  } else {
    for (int i = 0; i < keyPressTime; i++) {
      if (key == keyPress) {          // make sure that same key is press
        lcd.setCursor(x, y);          // set the lcd position
        lcd.print(alpha[ch]);         // print the character according to the character position
        ch++;                         // increment character position
        if (ch > maxValue) {          // if the character counter reached the max value
          ch = minValue;              // reset to min value
          i = 0;                      // reset the loop counter
        }
      }
      key = customKeypad.getKey();  // get the keypress
      delay(10);                    // delay for some time
    }
    x++;                    // increment the x position
    msg += alpha[ch - 1];   // add the character to the variable msg
    if (x > 15) {           // if the lcd reached the rightmost position
      y = 1;                // then wrap to the next line
      x = 0;                // in first character in the left
    }
  }
}
