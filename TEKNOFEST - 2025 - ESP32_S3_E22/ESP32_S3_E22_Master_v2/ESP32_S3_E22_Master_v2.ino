#include "LoRa_E22.h"
#include <HardwareSerial.h>

#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define M0 4 //3in1 PCB mizde pin 7
#define M1 6 //3in1 PCB mizde pin 6
#define RX 18 //3in1 PCB mizde pin RX
#define TX 17  //3in1 PCB mizde pin TX

#define WIFI_SSID "mrfurkankaya"
#define WIFI_PASS "123456788"
#define MQTT_SERVER "192.168.93.162"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID  "TEKNOFEST"
#define MQTT_SUB_TOPIC  "CCS_RX"
#define MQTT_PUB_TOPIC  "CCS_TX"

WiFiClient espClient;
PubSubClient client(espClient);

boolean receive = false;

HardwareSerial fixajSerial(1);                            //Serial biri seçiyoruz.
LoRa_E22 e22(TX, RX, &fixajSerial, UART_BPS_RATE_115200);
 
typedef struct
{
  byte ID = 0;
  int position = 2000;
  byte leftRPM = 150;
  byte rightRPM = 150;
  byte sensor[8] = {0,0,1,1,1,1,0,0};
}LFR_Datas;

LFR_Datas LFR_Data;

void WiFi_Connect();
void WiFi_Control();
void MQTT_Connect();
void MQTT_Control();
void MQTT_Send();
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

  WiFi_Connect();

  client.setServer(MQTT_SERVER, MQTT_PORT); // (MQTT_IP_ADRESS, MQTT_PORT)
  client.setCallback(callback);
  MQTT_Connect();
}
 
void loop() {
  WiFi_Control();
  MQTT_Control();

  LoRa_Listen();
  if(receive){
    Serial.print("SLAVE LoRa'dan Gelen Veri: ");
    Serial.println(LFR_Data.ID);
    MQTT_Send();
    receive = false;
  }
  client.loop();
}

void LoRa_Send(){
  ResponseStatus rs = e22.sendFixedMessage(0, 2, 58, &LFR_Data, sizeof(LFR_Datas));
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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
  
  if (String(topic) == MQTT_SUB_TOPIC) {
    Serial.println("Topic doğrulandı!");
    StaticJsonDocument <256> doc;
    deserializeJson(doc,message);
  
    LFR_Data.ID = doc["ID"];
    LoRa_Send();
    delay(300); // 300 < delayTime < (SampleTime / 2)
    LoRa_Send();
    Serial.println("SLAVE LORA'ya veri iletildi!");
    Serial.println("İletilen Veri: " + String(LFR_Data.ID));
  }
}

void MQTT_Send(){
  StaticJsonDocument<500> JSONbuffer;
  JSONbuffer["ID"] = LFR_Data.ID;

  char JSONmessageBuffer[256];
  int Json_Byte = serializeJson(JSONbuffer, JSONmessageBuffer);

  //PubSubClient.cpp 376 satır!
  if (client.publish(MQTT_PUB_TOPIC, JSONmessageBuffer, true) == true){
    Serial.println("Success sending message to MQTT Broker");
  } 
  else{
    Serial.println("Error sending message to MQTT Broker!");
  }
}

void WiFi_Connect(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFi_Control(){
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi connection is loss!");
    WiFi_Connect();
  }

}

void MQTT_Connect(){
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(MQTT_SUB_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void MQTT_Control(){
  if (!client.connected()) {
    MQTT_Connect();
  }
}
