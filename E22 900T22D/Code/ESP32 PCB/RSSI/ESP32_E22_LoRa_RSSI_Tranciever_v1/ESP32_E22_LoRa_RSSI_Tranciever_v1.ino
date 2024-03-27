#include "LoRa_E22.h"
#include <HardwareSerial.h>
#define M0 32 //3in1 PCB mizde pin 7
#define M1 33 //3in1 PCB mizde pin 6
#define RX 27 //3in1 PCB mizde pin RX
#define TX 35  //3in1 PCB mizde pin TX
 
HardwareSerial fixajSerial(1);                            //Serial biri seçiyoruz.
LoRa_E22 e22(TX, RX, &fixajSerial, UART_BPS_RATE_9600);

typedef struct{
  char type[15] = "RSSI Test";
}Data;
Data data;

unsigned long currentTimer = 0;
byte sampleTime = 2;

void setup() {
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);
  digitalWrite(M1, 0);
 
  Serial.begin(9600);
  delay(500);
  e22.begin();
  delay(500);
}

void loop() {
  if((millis() - currentTimer) > (sampleTime * 1000)){
    ResponseStatus rs = e22.sendFixedMessage(0, 2, 18, &data, sizeof(Data));
    Serial.println(rs.getResponseDescription());
    Serial.println("Gonderilen Veri: " + String(data.type));
    currentTimer = millis();
  }
}
