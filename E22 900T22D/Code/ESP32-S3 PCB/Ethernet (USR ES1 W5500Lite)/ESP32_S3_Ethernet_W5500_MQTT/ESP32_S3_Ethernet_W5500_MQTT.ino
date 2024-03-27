#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
 
//const char* MQTT_SERVER = "192.168.0.160";
const char* MQTT_SERVER = "192.168.10.34";
const char* MQTT_CLIENT_ID = "CCS";
const char* MQTT_SUB_TOPIC = "CCS_RX";
const char* MQTT_PUB_TOPIC = "CCS_TX";
 
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
 
// Set the static IP address to use if the DHCP fails to assign

#define MYIPADDR 192,168,10,28
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,10,1
#define MYGW 192,168,10,1

//#define MYIPADDR 192,168,0,28
//#define MYIPMASK 255,255,255,0
//#define MYDNS 192,168,0,1
//#define MYGW 192,168,0,1
 
// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient EthClient;
//PubSubClient mqtt(Ethclient);
PubSubClient mqtt;
 
// Variables
unsigned long timer = 0;

 
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Begin Ethernet");
  
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);    // ESP32-S3 PCB Ethernet CS Pin -> CS -> GPIO10!
  
  IPAddress ip(MYIPADDR);
  IPAddress dns(MYDNS);
  IPAddress gw(MYGW);
  IPAddress sn(MYIPMASK);
  Ethernet.begin(mac, ip, dns, gw, sn);
  Serial.println("STATIC OK!");
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true) {
          delay(1); // do nothing, no point running without Ethernet hardware
        }
      }
      if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
      }
        
  delay(5000);
  
  Serial.print("Local IP : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : ");
  Serial.println(Ethernet.dnsServerIP());
  
  Serial.println("Ethernet Successfully Initialized");

  mqtt.setClient(EthClient);
  mqtt.setServer(MQTT_SERVER, 1883);
  mqtt.setCallback(callback);
  MQTT_Connect();
}
 
void loop() {
  if (!mqtt.connected()) {
    MQTT_Connect();
  }
  if((millis() - timer) > 5000){
    boolean state = mqtt.publish(MQTT_PUB_TOPIC, "Hello");
    Serial.println("MQTT Publish State: " + String(state));
    timer = millis();
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
  
//  if (String(topic) == MQTT_SUB_TOPIC) {
//    StaticJsonDocument <256> doc;
//    deserializeJson(doc,message);
//  
//    loraData.ID = doc["ID"];
//    loraData.ADRESS_H = doc["ADRESS_H"];
//    loraData.ADRESS_L = doc["ADRESS_L"];
//    loraData.CHANNEL = doc["CHANNEL"];
//    loraData.command = doc["command"];
//
//    LoRa_Send();
//  }
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
