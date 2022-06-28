/*
  Name:USB Communication
  Description:Communicating ESP32 using the TTL
  PINOUTS:
/*USB TO TTL       ESP32
 * VCC             VIN
 * GND             GND
 * TX              RX0
 * RX              TX0
 USE PUTTY SOFTWARE TO WATCH THE OUTPUT.
//===================================================================*/
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
Serial.println("IOT LAB KIT");
delay(1000);
}
