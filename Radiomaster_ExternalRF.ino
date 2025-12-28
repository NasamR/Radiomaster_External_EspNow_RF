#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

#define SBUS Serial1 // Using Serial1 to connect SBUS

// Variables for storing SBUS data
uint8_t startBytes[3];
uint8_t sbusData[22];

// Data structure for sending
typedef struct message {
  int ch1;
  int ch2;
  int ch3;
  int ch4;
  int ch5;
  int ch6;
  int ch7;
  int ch8;
  int ch9;
  int ch10;
} message;
message channels;

// MAC address
uint8_t broadcastAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // MAC address

// Tasks
void SbusTask(void *pvParameters);
void EspNowTask(void *pvParameters);

// Function to add zeros to a byte for a complete number
String expandBinary(int num) {
  String binary = "";
  for (int i = 7; i >= 0; i--) {
    // We check the i-th bit of the number, starting from the most significant one
    if ((num >> (7 - i)) & 1) {
      binary += "1";
    } else {
      binary += "0";
    }
  }
  return binary;
}

// SBUS to number converter function
int convertBinary(String data) {
  String result = "";
  result = result + data[10] + data[9] + data[8] + data[7] + data[6] + data[5] + data[4] + data[3] + data[2] + data[1] + data[0];
  return strtol(result.c_str(), NULL, 2);
}

void setup() {
  // Setting up SBUS
  SBUS.begin(100000, SERIAL_8E2, 0, 1); // Pins RX 0, TX 1 on the microcontroller that will receive data from the radio
  SBUS.setRxInvert(true);

  // Open the monitor port
  Serial.begin(115200);

  // Selecting a WiFi mode
  WiFi.mode(WIFI_STA);
 
  // Launching the ESP-NOW protocol
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Adding receiver mac
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
  
  // Creating tasks
  xTaskCreate(SbusTask, "SBUS Task", 2048, NULL, 1, NULL);
  xTaskCreate(EspNowTask, "EspNow Task", 2048, NULL, 1, NULL);
}

// We don't do anything in the main loop, the tasks work independently
void loop() {
  vTaskDelay(portMAX_DELAY);
}

// The task of reading data via SBUS
void SbusTask(void *pvParameters) {
  while (true) {
    if(SBUS.available() > 0) {
      SBUS.readBytes(startBytes, 3);  // Reading the first 3 bytes
      if ((startBytes[0] == 0x00) && (startBytes[1] == 0x00) && (startBytes[2] == 0x0F)) {  // If they match the two ending and one starting bytes of sbus
        // Read the next 22 bytes
        if(SBUS.available() > 22) {
          SBUS.readBytes(sbusData, 22);

          // We convert to bits and add them together
          String data = "";
          for (int i = 0; i < 22; i++) {
            data += expandBinary(sbusData[i]);
          }

          // We'll break it down into channels - I chose the first 10, but there are 16 in total, you can add them using the example
          int ch1 = convertBinary(data.substring(0, 11));
          int ch2 = convertBinary(data.substring(11, 22));
          int ch3 = convertBinary(data.substring(22, 33));
          int ch4 = convertBinary(data.substring(33, 44));
          int ch5 = convertBinary(data.substring(44, 55));
          int ch6 = convertBinary(data.substring(55, 66));
          int ch7 = convertBinary(data.substring(66, 77));
          int ch8 = convertBinary(data.substring(77, 88));
          int ch9 = convertBinary(data.substring(88, 99));
          int ch10 = convertBinary(data.substring(99, 110));

          // We write to the data packet for sending
          channels.ch1 = ch1;
          channels.ch2 = ch2;
          channels.ch3 = ch3;
          channels.ch4 = ch4;
          channels.ch5 = ch5;
          channels.ch6 = ch6;
          channels.ch7 = ch7;
          channels.ch8 = ch8;
          channels.ch9 = ch9;
          channels.ch10 = ch10;

          // Print
          Serial.print(ch1);
          Serial.print(" ");
          Serial.print(ch2);
          Serial.print(" ");
          Serial.print(ch3);
          Serial.print(" ");
          Serial.print(ch4);
          Serial.print(" ");
          Serial.print(ch5);
          Serial.print(" ");
          Serial.print(ch6);
          Serial.print(" ");
          Serial.print(ch7);
          Serial.print(" ");
          Serial.print(ch8);
          Serial.print(" ");
          Serial.print(ch9);
          Serial.print(" ");
          Serial.println(ch10);
        }
      }
    }
  }
}

// The task of sending data via ESP-NOW
void EspNowTask(void *pvParameters) {
  while (true) {
    esp_now_send(broadcastAddress, (uint8_t *) &channels, sizeof(channels));
  }
}