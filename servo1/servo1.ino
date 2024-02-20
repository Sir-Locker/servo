#include <ESP32Servo.h>

#define SERVO1_PIN 22
#define BUTTON_PIN 21

Servo servo1;

int servo_state = 0;
int currentState = 0;
int i = 0;

void setup(){
  Serial.begin(115200);
  servo1.attach(SERVO1_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  servo1.write(0); 
  Serial.print(" - start - \n");
}

void loop(){
  currentState = digitalRead(BUTTON_PIN);
  if (currentState == LOW) {
    Serial.print("button pressed!!\n");
    servo_state = !servo_state;
    if (servo_state == 0) {
      for (i=0; i<=90; i++) {
        servo1.write(i);
        delay(10);
      }
      Serial.print(" - unlock the door\n");
    }
    else {
      for (i=90; i>=0; i--) {
        servo1.write(i);
        delay(10);
      }
      Serial.print(" - lock the door\n");
    }
  }
}
