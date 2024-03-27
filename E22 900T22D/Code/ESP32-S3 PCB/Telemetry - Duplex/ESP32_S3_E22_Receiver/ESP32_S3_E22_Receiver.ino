#include <Arduino.h>
#include <HardwareSerial.h>
#include <EByte_LoRa_E22_library.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define M0 4 //3in1 PCB mizde pin 7
#define M1 6 //3in1 PCB mizde pin 6
#define RX 18 //3in1 PCB mizde pin RX
#define TX 17  //3in1 PCB mizde pin TX

#define WIFI_SSID "Repeter"
#define WIFI_PASS "057921236"
#define MQTT_SERVER "192.168.0.194"
#define MQTT_PORT 1883
#define TOPIC "LFR"

WiFiClient espClient;
PubSubClient client(espClient);
boolean receive = false;
 
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

void WiFi_Connect();
void WiFi_Control();
void MQTT_Connect();
void MQTT_Control();
void LoRa_Listen();
void MQTT_Send();

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
  MQTT_Connect();
}

void loop() {
  WiFi_Control();
  MQTT_Control();

  client.loop();

  LoRa_Listen();
  MQTT_Send();
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
    if (client.connect("CCS_Device")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("IOT_GRUP7");
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

void LoRa_Listen(){
  while (e22.available()  > 0) {
    // Gelen mesaj okunuyor
    ResponseStructContainer rsc = e22.receiveMessage(sizeof(LFR_Datas));
    LFR_Data = *(LFR_Datas*) rsc.data;

    rsc.close();
    receive = true;
  }
}

void MQTT_Send(){
  if(receive){
    StaticJsonDocument<500> JSONbuffer;
    JSONbuffer["ID"] = LFR_Data.ID;
    JSONbuffer["position"] = LFR_Data.position;
    JSONbuffer["leftPwm"] = LFR_Data.leftRPM;
    JSONbuffer["rightPwm"] = LFR_Data.rightRPM;

    JsonArray sensors = JSONbuffer.createNestedArray("sensor");
    sensors.add(LFR_Data.sensor[0]);
    sensors.add(LFR_Data.sensor[1]);
    sensors.add(LFR_Data.sensor[2]);
    sensors.add(LFR_Data.sensor[3]);
    sensors.add(LFR_Data.sensor[4]);
    sensors.add(LFR_Data.sensor[5]);
    sensors.add(LFR_Data.sensor[6]);
    sensors.add(LFR_Data.sensor[7]);

    char JSONmessageBuffer[256];
    int Json_Byte = serializeJson(JSONbuffer, JSONmessageBuffer);

    //PubSubClient.cpp 376 satır!
    if (client.publish(TOPIC, JSONmessageBuffer, true) == true){
      Serial.println("Success sending message");
    } 
    else{
      Serial.println("Error sending message");
    }
    receive = false;
  }
}
