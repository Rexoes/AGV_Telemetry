#include <HardwareSerial.h>
#include <EByte_LoRa_E22_library.h>

#define M0 4 //3in1 PCB mizde pin 7
#define M1 6 //3in1 PCB mizde pin 6
#define RX 18 //3in1 PCB mizde pin RX
#define TX 17  //3in1 PCB mizde pin TX

boolean receive = false;
 
HardwareSerial fixajSerial(1);                            //Serial biri seçiyoruz.
LoRa_E22 e22(TX, RX, &fixajSerial, UART_BPS_RATE_115200);

unsigned long timer;
int sampleTime = 1500;

typedef struct
{
  byte ID = 0;
  int position = 2000;
  byte leftRPM = 150;
  byte rightRPM = 150;
  byte sensor[8] = {0,0,1,1,1,1,0,0};
}LFR_Datas;

LFR_Datas LFR_Data;

void LoRa_Listen();
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
  timer = 0;
}

void loop() {
  if((millis() - timer) > sampleTime){
     LoRa_Send();
     timer = millis();
  }
  
  
  LoRa_Listen();
  if(receive){
    Serial.print("MASTER LoRa'dan Gelen Veri: ");
    Serial.println(LFR_Data.ID);
    receive = false;
  }
}

void LoRa_Send(){
  LFR_Data.ID++;
  ResponseStatus rs = e22.sendFixedMessage(0, 1, 58, &LFR_Data, sizeof(LFR_Datas));
  Serial.println(rs.getResponseDescription());
}

void LoRa_Listen(){
  while (e22.available()  > 0) {
    // Gelen mesaj okunuyor
    ResponseStructContainer rsc = e22.receiveMessage(sizeof(LFR_Datas));
    LFR_Data = *(LFR_Datas*) rsc.data;

    rsc.close();
    receive = true;
  }
}
