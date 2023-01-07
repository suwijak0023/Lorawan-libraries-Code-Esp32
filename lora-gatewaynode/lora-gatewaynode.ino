#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>



const char* ssid = "xxxWiFi-SSID";
const char* password = "xxxWiFi Password";
const char* mqtt_server = "xxx.xxx.xxx.xxx"; //<-- IP หรือ Domain ของ Server MQTT

long lastMsg = 0;
char msg[100];
int value = 0;

WiFiClient espClient;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}



PubSubClient client(mqtt_server, 1883, callback, espClient);




// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)
//OLED pins to ESP32 0.96OLEDGPIOs
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16
SSD1306  display(0x3c, 4, 15);
#define SS      18
#define RST     14
#define DI0     26
#define BAND    433.175E6  //915E6

void setup() {
  Serial.begin(115200);

  while (!Serial);
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  Serial.println("LoRa Receiver");

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

  setup_wifi();
  client.connect("ESP32Gateway", "joe1", "joe1");
  client.setCallback(callback);
  client.subscribe("command");



}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Gateway")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  String tmp_string, tmp_rssi;

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // try to parse packet
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    // read packet
    while (LoRa.available()) {
      //Serial.print((char)LoRa.read());
      tmp_string += (char)LoRa.read();
    }
    Serial.print(tmp_string);

    tmp_rssi = LoRa.packetRssi();

    // print RSSI of packet


    Serial.println("' with RSSI " + tmp_rssi);


    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(10, 0, "Received:");
    display.drawString(10, 15, "From: " + tmp_string.substring(13, 19) + " RSSI: " + tmp_rssi);
    display.drawString(10, 30, "Temp: " + tmp_string.substring(49, 55));
    display.drawString(10, 45, "Humid: " + tmp_string.substring(69, 75));
    // write the buffer to the display
    display.display();

    tmp_string.toCharArray(msg, 100);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("env", msg);  //ส่งข้อความ Temp + Humidity ออกไปที่ Topic "env"


  }
  tmp_string = "";
  tmp_rssi = "";


}
