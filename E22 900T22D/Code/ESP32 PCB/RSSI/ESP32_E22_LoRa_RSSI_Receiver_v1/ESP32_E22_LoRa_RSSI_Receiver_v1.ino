#include "LoRa_E22.h"
#include <HardwareSerial.h>
#define M0 32 //3in1 PCB mizde pin 7
#define M1 33 //3in1 PCB mizde pin 6
#define RX 27 //3in1 PCB mizde pin RX
#define TX 35  //3in1 PCB mizde pin TX

HardwareSerial fixajSerial(1);                            //Serial biri seçiyoruz.
LoRa_E22 e22(TX, RX, &fixajSerial, UART_BPS_RATE_9600);

typedef struct{
  char type[15];
}Data;
Data data;

void setup() {
  Serial.begin(9600);
  
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);
  digitalWrite(M1, 0);

  delay(500);
  e22.begin();
  delay(500);
}

void loop() {
  while(e22.available() > 1){
    ResponseStructContainer rc = e22.receiveMessageRSSI(sizeof(Data));
    if(rc.status.code != 1){
      Serial.println(rc.status.getResponseDescription());
    }
    else{
      Serial.println(rc.status.getResponseDescription());

      Serial.print("RSSI: ");
      Serial.println(rc.rssi, DEC);

      data = *(Data*)rc.data;
      Serial.print("Gelen Mesaj: ");
      Serial.println(data.type);

      rc.close();
    }
  }
}
