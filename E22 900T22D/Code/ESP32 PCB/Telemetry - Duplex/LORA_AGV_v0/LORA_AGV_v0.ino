#include "LoRa_E22.h"
#include <HardwareSerial.h>

#define M0 32 //3in1 PCB mizde pin 7
#define M1 33 //3in1 PCB mizde pin 6
#define RX 27 //3in1 PCB mizde pin RX
#define TX 35  //3in1 PCB mizde pin TX

#define LORA_ADRESS_H 0
#define LORA_ADRESS_L 1
#define LORA_CHANNEL 18

HardwareSerial fixajSerial(1);                            //Serial biri seçiyoruz.
LoRa_E22 e22(TX, RX, &fixajSerial, UART_BPS_RATE_9600);

typedef struct{
  byte ID;
  byte ADRESS_H;
  byte ADRESS_L;
  byte CHANNEL;
  byte command;
}LORA_DATA;

LORA_DATA loraData = {58, 0, 2, 18, 0};

boolean LORA_AVAILABLE = false;
unsigned long timer = 0;
int sampleTime = 500;

void LoRa_Listen(void);
void LoRa_Send(void);
void Telemetri_Info(void);

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
  LoRa_Listen();
  if(LORA_AVAILABLE == true){
    //Gelen veriyi parçala!
    Telemetri_Info();    
    LORA_AVAILABLE = false;
  }
  
  if((millis() - timer) > sampleTime){
    //AGV için sampleTime periyodunda telemetri verilerini güncelle ve CCS'ye gönder!
    loraData.command++;
    LoRa_Send();
    timer = millis();
  }
}

void LoRa_Listen(){
  while (e22.available()  > 0) {
    // Gelen mesaj okunuyor
    ResponseStructContainer rsc = e22.receiveMessage(sizeof(LORA_DATA));
    loraData = *(LORA_DATA*) rsc.data;

    rsc.close();
    LORA_AVAILABLE = true;
  }
}

void LoRa_Send(){
  ResponseStatus rs = e22.sendFixedMessage(LORA_ADRESS_H, LORA_ADRESS_L, LORA_CHANNEL, &loraData, sizeof(LORA_DATA));
  Serial.println(rs.getResponseDescription());
}

void Telemetri_Info(){
  Serial.println("\n************************************");
  Serial.println("ID: " + String(loraData.ID));
  Serial.println("LORA_ADRESS_H: " + String(loraData.ADRESS_H));
  Serial.println("LORA_ADRESS_L: " + String(loraData.ADRESS_L));
  Serial.println("LORA_CHANNEL: " + String(loraData.CHANNEL));
  Serial.println("COMMAND: " + String(loraData.command));
}
