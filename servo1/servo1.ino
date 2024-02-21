#include <ESP32Servo.h>
#include <esp_now.h>
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

#define SERVO1_PIN 22

Servo servo1;
int i = 0;

uint8_t MacAddressKeyPad[] = {0x3C, 0x61, 0x05, 0x03, 0xCA, 0x04};
uint8_t MacAddressUltrasonic[] = {0xE8, 0xDB, 0x84, 0x00, 0xFB, 0x3C};
uint8_t MacAddressScanner[] = {0xE8, 0xDB, 0x84, 0x01, 0x07, 0x90};

typedef struct servo_struct{
  int servo_status;//1 = lock, 0 = unlock
} servo_struct;

typedef struct ultrasonic_send {
  int stateUltra; // 1 = ประตูปิด, 0 = ประตูเปิด
} ultrasonic_send;

typedef struct recieve_status_scanner {
  int scanner_status;
} recieve_status_scanner;

servo_struct send_servo;
ultrasonic_send ultrasonic;

esp_now_peer_info_t peerInfoUltrasonic; //ultrasonic
esp_now_peer_info_t peerInfoKeyPad;
esp_now_peer_info_t peerInfoScanner;

bool compareMac(const uint8_t * a, uint8_t * b){
  for(int i=0;i<6;i++){
    if(a[i]!=b[i])
      return false;    
  }
  return true;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  //receive data
  memcpy(&ultrasonic, incomingData, sizeof(ultrasonic));
  Serial.println(ultrasonic.stateUltra);
}

void setup(){
  Serial.begin(115200);
  
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  //peer ultrasonic
  memcpy(peerInfoUltrasonic.peer_addr, MacAddressUltrasonic, 6);
  peerInfoUltrasonic.channel = 0;  
  peerInfoUltrasonic.encrypt = false;
  if (esp_now_add_peer(&peerInfoUltrasonic) != ESP_OK){
    Serial.println("Ultrasonic: Failed to add peer");
    return;
  }
  //peer keypad
  memcpy(peerInfoKeyPad.peer_addr, MacAddressKeyPad, 6);
  peerInfoKeyPad.channel = 0;  
  peerInfoKeyPad.encrypt = false;
  if (esp_now_add_peer(&peerInfoKeyPad) != ESP_OK){
    Serial.println("KeyPad: Failed to add peer");
    return;
  }
  //peer scanner
  memcpy(peerInfoScanner.peer_addr, MacAddressScanner, 6);
  peerInfoScanner.channel = 0;  
  peerInfoScanner.encrypt = false;
  if (esp_now_add_peer(&peerInfoScanner) != ESP_OK){
    Serial.println("Scanner: Failed to add peer");
    return;
  }
  
  servo1.attach(SERVO1_PIN);
  send_servo.servo_status = 1;
  servo1.write(0); 
  
  Serial.print(" - start - \n");
}

void loop(){
  //lock
  if (ultrasonic.stateUltra == 1 && send_servo.servo_status == 0) {
    for (i=0; i<=90; i++) {
      servo1.write(i);
      delay(10);
    }
    Serial.println("lock the door");
    send_servo.servo_status = 1;
    esp_err_t result = esp_now_send(MacAddressKeyPad, (uint8_t *) &send_servo, sizeof(servo_struct)); //mode 1 reset
    if (result == ESP_OK) {
      Serial.println("sending_data - success\n - lock the door");
    }
    else {
      Serial.println("Error sending the data");
    }
      
  }
  //unlock
  else if (ultrasonic.stateUltra == 0 && send_servo.servo_status == 1) {
    for (i=90; i>=0; i--) {
      servo1.write(i);
      delay(10);
    }
    Serial.print(" - unlock the door\n");
    send_servo.servo_status = 0;
  }
}
