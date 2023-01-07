#include "painlessMesh.h"
#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"

// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)
//OLED pins to ESP32 0.96OLEDGPIOs :
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16


#define   MESH_PREFIX     "HelloMyMesh"
#define   MESH_PASSWORD   "hellomymeshnetwork"
#define   MESH_PORT       5555

SSD1306  display(0x3c, 4, 15);
#define SS      18
#define RST     14
#define DI0     26
#define BAND    433.175E6  //915E6

painlessMesh  mesh;
// Send my ID every 10 seconds to inform others

Task logServerTask(10000, TASK_FOREVER, []() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& msg = jsonBuffer.createObject();
  msg["topic"] = "logServer";
  msg["nodeId"] = mesh.getNodeId();
  String str;
  msg.printTo(str);
  mesh.sendBroadcast(str);
  // log to serial
  msg.printTo(Serial);
  Serial.printf("\n");
});


void setup() {


  Serial.begin(115200);

  pinMode(25, OUTPUT); //Send success, LED will bright 1 second

  while (!Serial);

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high


  Serial.println("LoRa PainlessMesh Server");

  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Initial OK!");
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(10, 5, "Mesh Server Node:");
  display.display();



  mesh.setDebugMsgTypes( ERROR | CONNECTION | S_TIME );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, STA_AP, WIFI_AUTH_WPA2_PSK, 6 );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection([](size_t nodeId) {
    Serial.printf("New Connection %u\n", nodeId);
  });
  mesh.onDroppedConnection([](size_t nodeId) {
    Serial.printf("Dropped Connection %u\n", nodeId);
  });
  // Add the task to the mesh scheduler
  mesh.scheduler.addTask(logServerTask);
  logServerTask.enable();
}


void loop() {
  mesh.update();
}


void receivedCallback( uint32_t from, String &msg ) {
  
  String tmp_string = msg.c_str();
  
  Serial.printf("logServer: Received from %u msg=%s\n", from, tmp_string);
  
  Serial.println("");
  Serial.println("Sending LoRa packet: "+tmp_string);
  
  
  //เมื่อได้รับข้อความจากใน mesh network ก็ส่งต่อผ่านไปยัง LoRa
  LoRa.beginPacket();
  LoRa.print(tmp_string);
  LoRa.endPacket();
  
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(10, 5, "Sending: "+tmp_string.substring(13,19));
  display.drawString(10, 20, "Temp: "+tmp_string.substring(49,55));
  display.drawString(10, 35, "Humid: "+tmp_string.substring(69,75));
  // write the buffer to the display
  display.display();


}
