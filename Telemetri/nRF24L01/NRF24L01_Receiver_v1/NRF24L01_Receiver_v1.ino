#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//RF24 radio(5, 6);   // nRF24L01 (CE, CSN)
RF24 radio(7, 8);   // nRF24L01 (CE, CSN)

const byte address[6] = "00002";

struct Joystick{
  byte POT_X;
  byte POT_Y;
  byte LIFT;
};

Joystick joystick = {127, 127, 0};

#define LEFT_FORWARD_ENB 5
#define LEFT_BACKWARD_ENB 3
#define RIGHT_FORWARD_ENB 9
#define RIGHT_BACKWARD_ENB 6

struct AGV{
  byte DIRECTION;
  byte SPEED;
};

AGV agv = {0, 0};


void Display(void);
void Motor_Control(void);
void Forward(void);
void Backward(void);
void Left(void);
void Right(void);
void Stop(void);

void setup() {
  Serial.begin(9600);

  boolean radio_init = radio.begin();
  if(!radio_init)
    Serial.println("NRF24L01 Init Failed!");
  else
    Serial.println("NRF24L01 Init Succesful :)");

  radio.setAutoAck(false);  //radio.write() bilgisi AutoAck false olduğu için alınamaz! 
  radio.setChannel(115);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, address);
  radio.startListening();

  Serial.println("Configurations Succesful :)");

  pinMode(LEFT_FORWARD_ENB, OUTPUT); analogWrite(LEFT_FORWARD_ENB, 0);
  pinMode(LEFT_BACKWARD_ENB, OUTPUT); analogWrite(LEFT_BACKWARD_ENB, 0);
  pinMode(RIGHT_FORWARD_ENB, OUTPUT); analogWrite(RIGHT_FORWARD_ENB, 0);
  pinMode(RIGHT_BACKWARD_ENB, OUTPUT); analogWrite(RIGHT_BACKWARD_ENB, 0);
}

void loop() {
  //Read the data if available in buffer, buffer is 32byte.
  if (radio.available())
  {
    radio.read(&joystick, sizeof(Joystick));
    Display();
    Motor_Control();
  }
}


//Potansiyometre değerlerine göre aracı kontrol et!
void Motor_Control(){
  byte currentDirection = 0;
  byte X = joystick.POT_X;
  byte Y = joystick.POT_Y;
  //byte LIFT = joystick.LIFT;
  
  if(X > 200){
    if(Y > 100 && Y < 200)
      currentDirection = 2;
  }
  else if (X < 40){
    if(Y > 100 && Y < 200)
      currentDirection = 4; 
  }
  else if(Y > 230){
    if(X > 100 && X < 200)
      currentDirection = 1;
  }
  else if (Y < 30){
    if(X > 100 && X < 200)
      currentDirection = 3;
  }
  else
    currentDirection = 0;

  if(currentDirection != agv.DIRECTION){
    switch(currentDirection){
      case 1:{
        Serial.println("İleri"); Forward(); break;
      }
      case 2:{
        Serial.println("Sağ"); Right(); break;
      }
      case 3:{
        Serial.println("Geri"); Backward(); break;
      }
      case 4:{
        Serial.println("Sol"); Left(); break;
      }
      case 0:{
        Serial.println("Dur"); Stop(); break;
      }
    }
    agv.DIRECTION = currentDirection;
  }
}

void Display(){
  Serial.println("Joystick POT_X: " + String(joystick.POT_X));
  Serial.println("Joystick POT_Y: " + String(joystick.POT_Y));
  Serial.println("Joystick LIFT: " + String(joystick.LIFT));
}

void Forward(){
  if(agv.DIRECTION == 0){
    for(int i = 55; i <= 255; i += 20){ // 10 tekrar = 1000 ms max hız.
      analogWrite(LEFT_FORWARD_ENB, i);
      analogWrite(RIGHT_FORWARD_ENB, i);
      delay(100);
    }
  }
  else{
    Stop();
    Forward();
  }
  agv.DIRECTION = 1;
  agv.SPEED = 255;
  Serial.println("FORWARD");
}

void Backward(){
  if(agv.DIRECTION == 0){
    for(int i = 55; i <= 255; i += 20){ // 10 tekrar = 1000 ms max hız.
      analogWrite(LEFT_BACKWARD_ENB, i);
      analogWrite(RIGHT_BACKWARD_ENB, i);
      delay(100);
    }
  }
  else{
    Stop();
    Backward();
  }
  agv.DIRECTION = 3;
  agv.SPEED = 255;
  Serial.println("BACKWARD");
}

void Left(){
  Stop();
  analogWrite(RIGHT_FORWARD_ENB, 255);
  analogWrite(LEFT_BACKWARD_ENB, 255);
  agv.DIRECTION = 4;
  agv.SPEED = 200;
  Serial.println("LEFT");
}

void Right(){
  Stop();
  analogWrite(RIGHT_BACKWARD_ENB, 255);
  analogWrite(LEFT_FORWARD_ENB, 255);
  agv.DIRECTION = 2;
  agv.SPEED = 200;
  Serial.println("RIGHT");
}

void Stop(){
  if(agv.DIRECTION == 1){
    for(int i = agv.SPEED - 5; i >= 0; i -= 25){ // 10 tekrar = 1000 ms max hız.
      analogWrite(LEFT_FORWARD_ENB, i);
      analogWrite(RIGHT_FORWARD_ENB, i);
      delay(100);
    }
  }
  else if(agv.DIRECTION == 3){
    for(int i = agv.SPEED - 5; i >= 0; i -= 25){ // 10 tekrar = 1000 ms max hız.
      analogWrite(LEFT_BACKWARD_ENB, i);
      analogWrite(RIGHT_BACKWARD_ENB, i);
      delay(100);
    }
  }
  else if(agv.DIRECTION == 2){
    for(int i = agv.SPEED; i >= 0; i -= 20){ // 10 tekrar = 1000 ms max hız.
      analogWrite(LEFT_FORWARD_ENB, i);
      analogWrite(RIGHT_BACKWARD_ENB, i);
      delay(100);
    }
  }
  else if(agv.DIRECTION == 4){
    for(int i = agv.SPEED - 5; i >= 0; i -= 20){ // 10 tekrar = 1000 ms max hız.
      analogWrite(LEFT_BACKWARD_ENB, i);
      analogWrite(RIGHT_FORWARD_ENB, i);
      delay(100);
    }
  }
  analogWrite(LEFT_FORWARD_ENB, 0);
  analogWrite(LEFT_BACKWARD_ENB, 0);
  analogWrite(RIGHT_FORWARD_ENB, 0);
  analogWrite(RIGHT_BACKWARD_ENB, 0);
  
  agv.DIRECTION = 0;
  agv.SPEED = 0;
  Serial.println("STOP");
}
