#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(5, 6);   // nRF24L01 (CE, CSN)

const byte address[6] = "00002";

#define RIGHT_JOYSTICK_X A2
#define RIGHT_JOYSTICK_Y A3
#define RIGHT_TOGGLE_SWTICH 4
#define LEFT_TOGGLE_SWTICH 7

struct Joystick{
  byte POT_X;
  byte POT_Y;
  byte LIFT;
};

Joystick joystick = {127, 127, 0};
Joystick prev = {127, 127, 0};

boolean transmit = false;

void Update(void);
void Display(void);

void setup() {
  Serial.begin(9600);

  boolean radio_init = radio.begin();
  if(!radio_init)
    Serial.println("NRF24L01 Init Failed!");
  else
    Serial.println("NRF24L01 Init Succesful :)");

  radio.setAutoAck(false);  //radio.write() bilgisi AutoAck false olduğu için alınamaz!
  radio.setChannel(115);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS); 
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("Configurations Succesful :)");

  pinMode(LEFT_TOGGLE_SWTICH, INPUT_PULLUP);
}

void loop() {
  Update();
  if(transmit){
    radio.write(&joystick, sizeof(Joystick));
    Display();
    transmit = false;
  }
  //delay(500);
}

void Update(){
  byte offset = 40;
  
  byte current_POT_X = 255 - map(analogRead(RIGHT_JOYSTICK_X), 0, 1023, 0, 255);
  byte current_POT_Y = map(analogRead(RIGHT_JOYSTICK_Y), 0, 1023, 0, 255);
  byte current_LIFT = !digitalRead(LEFT_TOGGLE_SWTICH);

  if((current_POT_X < (prev.POT_X - offset)) | (current_POT_X > (prev.POT_X + offset))){
    joystick.POT_X = current_POT_X;
    prev.POT_X = current_POT_X;
    transmit = true;
  }
    
  if((current_POT_Y < (prev.POT_Y - offset)) | (current_POT_Y > (prev.POT_Y + offset))){
    joystick.POT_Y = current_POT_Y;
    prev.POT_Y = current_POT_Y;
    transmit = true;
  }
    
  if(prev.LIFT != current_LIFT){
    joystick.LIFT = current_LIFT;
    prev.LIFT = current_LIFT;
    transmit = true;
  }
}

void Display(){
  Serial.println("Joystick POT_X: " + String(joystick.POT_X));
  Serial.println("Joystick POT_Y: " + String(joystick.POT_Y));
  Serial.println("Joystick LIFT: " + String(joystick.LIFT));
}
