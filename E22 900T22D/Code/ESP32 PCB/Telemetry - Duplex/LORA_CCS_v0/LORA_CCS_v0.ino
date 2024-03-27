#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "LoRa_E22.h"
#include <HardwareSerial.h>

const char* WiFi_SSID = "Repeter";
const char* WiFi_PASS = "057921236";
const char* MQTT_SERVER = "192.168.0.194";
const char* MQTT_CLIENT_ID = "CCS";
const char* MQTT_SUB_TOPIC = "CCS_RX";
const char* MQTT_PUB_TOPIC = "CCS_TX";


WiFiClient espmqtt;
PubSubClient mqtt(espmqtt);

#define LORA_ADRESS_H 0
#define LORA_ADRESS_L 2
#define LORA_CHANNEL 18

#define M0 32 //3in1 PCB mizde pin 7
#define M1 33 //3in1 PCB mizde pin 6
#define RX 27 //3in1 PCB mizde pin RX
#define TX 35  //3in1 PCB mizde pin TX
 
HardwareSerial fixajSerial(1);                            //Serial biri seçiyoruz.
LoRa_E22 e22(TX, RX, &fixajSerial, UART_BPS_RATE_9600);

typedef struct{
  byte ID;
  byte ADRESS_H;
  byte ADRESS_L;
  byte CHANNEL;
  byte command;
}LORA_DATA;

LORA_DATA loraData = {58, 0, 1, 18, 0};

boolean LORA_AVAILABLE = false;

void LoRa_Listen(void);
void LoRa_Send(void);
void Telemetri_Info(void);
void WiFi_MQTT_Check(void);

void setup() {
  Serial.begin(9600);
  
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);
  digitalWrite(M1, 0);
 
  delay(500);
  e22.begin();
  delay(500);

  WiFi_Connect();
  mqtt.setServer(MQTT_SERVER, 1883);
  mqtt.setCallback(callback);
  MQTT_Connect();
}

void loop() {
  //WiFi ve MQTT bağlantısını kontrol et!
  WiFi_MQTT_Check();

  LoRa_Listen();
  if(LORA_AVAILABLE == true){
    //Gelen veriyi parçala!
    Telemetri_Info();    
    LORA_AVAILABLE = false;
  }
  
  mqtt.loop();
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
  
  if (String(topic) == MQTT_SUB_TOPIC) {
    StaticJsonDocument <256> doc;
    deserializeJson(doc,message);
  
    loraData.ID = doc["ID"];
    loraData.ADRESS_H = doc["ADRESS_H"];
    loraData.ADRESS_L = doc["ADRESS_L"];
    loraData.CHANNEL = doc["CHANNEL"];
    loraData.command = doc["command"];

    LoRa_Send();
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
  delay(200);
  rs = e22.sendFixedMessage(LORA_ADRESS_H, LORA_ADRESS_L, LORA_CHANNEL, &loraData, sizeof(LORA_DATA));
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

void WiFi_Connect(){
  if(WiFi.status() != WL_CONNECTED){
    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WiFi_SSID);
  
    WiFi.begin(WiFi_SSID, WiFi_PASS);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void MQTT_Connect(){
   while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected");
      // Subscribe
      mqtt.subscribe(MQTT_SUB_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 2 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
  }
}

void WiFi_MQTT_Check(){
  WiFi_Connect();
  if (!mqtt.connected()) {
    MQTT_Connect();
  }
}
