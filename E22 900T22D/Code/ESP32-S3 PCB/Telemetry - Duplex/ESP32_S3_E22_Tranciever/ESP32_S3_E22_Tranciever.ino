#include "LoRa_E22.h"
#include <HardwareSerial.h>
#define M0 4 //3in1 PCB mizde pin 7
#define M1 6 //3in1 PCB mizde pin 6
#define RX 18 //3in1 PCB mizde pin RX
#define TX 17  //3in1 PCB mizde pin TX
 
HardwareSerial fixajSerial(1);                            //Serial biri seçiyoruz.
LoRa_E22 e22(TX, RX, &fixajSerial, UART_BPS_RATE_9600);
 
typedef struct
{
  byte ID = 1;
  int position = 2000;
  byte leftRPM = 150;
  byte rightRPM = 150;
  byte sensor[8] = {0,0,1,1,1,1,0,0};
}LFR_Datas;

LFR_Datas LFR_Data;

void LoRa_Send();
 
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
  LoRa_Send();
  delay(1000);
}

void LoRa_Send(){
  ResponseStatus rs = e22.sendFixedMessage(0, 2, 18, &LFR_Data, sizeof(LFR_Datas));
  Serial.println(rs.getResponseDescription());
}
