/*
 * HelTec Automation(TM) LoRaWAN 1.0.2 OTAA example use OTAA, CLASS A
 *
 * Function summary:
 *
 * — use internal RTC(150KHz);
 *
 * — Include stop mode and deep sleep mode;
 *
 * — 15S data send cycle;
 *
 * — Informations output via serial(115200);
 *
 * — Only ESP32 + LoRa series boards can use this library, need a license
 * to make the code run(check you license here: http://www.heltec.cn/search/);
 *
 * You can change some definition in “Commissioning.h” and “LoRaMac-definitions.h”
 *
 * HelTec AutoMation, Chengdu, China.
 * https://heltec.org
 * support@heltec.cn
 *
 *this project also release in GitHub:
 *https://github.com/HelTecAutomation/ESP32_LoRaWAN
 */
#include <Wire.h>
#include <ESP32_LoRaWAN.h>
#include “Arduino.h”
#include <CayenneLPP.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1006.0)
Adafruit_BME280 bme; // I2C
float temp, pa, humid, alt;
CayenneLPP lpp(51); // create a buffer of 51 bytes to store the payload
unsigned long delayTime;
/*license for Heltec ESP32 LoRaWan, quary your ChipID relevant license: http://resource.heltec.cn/search */
uint32_t license[4] = { 0x7353D08A,0xFD73AD45,0x900DB6F3,0xD422CE24};
/* OTAA parametros*/
uint8_t DevEui[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0x20, 0x42 };
uint8_t AppEui[] = { 0x8B, 0x65, 0x00, 0xF0, 0x7E, 0xD5, 0xB3, 0x11 };
uint8_t AppKey[] = { 0xDA, 0x5A, 0x42, 0xB8, 0x71, 0x50, 0x2E, 0xBA, 0x68, 0x38, 0xC5, 0x28, 0xDE, 0x07, 0xC2, 0x11 };
/* ABP para* Need for Compile */
uint8_t NwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x11 };
uint8_t AppSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x11 };
uint32_t DevAddr = ( uint32_t )0x007e6a11;
/*LoraWan channelsmask, default channels 0–7*/
uint16_t userChannelsMask[6] = {
 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};
/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t loraWanClass = CLASS_A;
/*the application data transmission duty cycle. value in [ms].*/
uint32_t appTxDutyCycle = 30000;
/*OTAA or ABP*/
bool overTheAirActivation = true;
/*ADR enable*/
bool loraWanAdr = true;
/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = false;
/* Application port */
uint8_t appPort = 2;
/*!
 * Number of trials to transmit the frame, if the LoRaMAC layer did not
 * receive an acknowledgment. The MAC performs a datarate adaptation,
 * according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
 * to the following table:
 *
 * Transmission nb | Data Rate
 * — — — — — — — — | — — — — — -
 * 1 (first) | DR
 * 2 | DR
 * 3 | max(DR-1,0)
 * 4 | max(DR-1,0)
 * 5 | max(DR-2,0)
 * 6 | max(DR-2,0)
 * 7 | max(DR-3,0)
 * 8 | max(DR-3,0)
 *
 * Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
 * the datarate, in case the LoRaMAC layer did not receive an acknowledgment
 */
uint8_t confirmedNbTrials = 8;
/*LoraWan debug level, select in arduino IDE tools.
 * None : print basic info.
 * Freq : print Tx and Rx freq, DR info.
 * Freq && DIO : print Tx and Rx freq, DR, DIO0 interrupt and DIO1 interrupt info.
 * Freq && DIO && PW: print Tx and Rx freq, DR, DIO0 interrupt, DIO1 interrupt and MCU deepsleep info.
 */
uint8_t debugLevel = LoRaWAN_DEBUG_LEVEL;
/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
// Add your initialization code here
void setup() {
 
 Serial.begin(115200);
 unsigned status;
 status = bme.begin(0x76);
if (!status) {
 Serial.println(“Could not find a valid BME280 sensor, check wiring, address, sensor ID!”);
 }
 
 while (!Serial);

 temp = bme.readTemperature();
 humid = bme.readHumidity();
 pa= bme.readPressure() / 100.0F;
 alt= bme.readAltitude(SEALEVELPRESSURE_HPA);
SPI.begin(SCK, MISO, MOSI, SS);
 Mcu.init(SS, RST_LoRa, DIO0, DIO1, license);
 
 deviceState = DEVICE_STATE_INIT;
}
// The loop function is called in an endless loop
void loop() {
switch (deviceState) {
 case DEVICE_STATE_INIT: {
 #if(LORAWAN_DEVEUI_AUTO)
 LoRaWAN.generateDeveuiByChipID();
 #endif
 LoRaWAN.init(loraWanClass, loraWanRegion);
 break;
 }
 case DEVICE_STATE_JOIN: {
 LoRaWAN.displayJoining();
 LoRaWAN.join();
 break;
 }
 case DEVICE_STATE_SEND: {
 LoRaWAN.displaySending();
 prepareTxFrame(appPort);
 LoRaWAN.send(loraWanClass);
 deviceState = DEVICE_STATE_CYCLE;
 break;
 }
 case DEVICE_STATE_CYCLE: {
 // Schedule next packet transmission
 txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
 LoRaWAN.cycle(txDutyCycleTime);
 deviceState = DEVICE_STATE_SLEEP;
 break;
 }
 case DEVICE_STATE_SLEEP: {
 LoRaWAN.sleep(loraWanClass, debugLevel);
 break;
 }
 default: {
 deviceState = DEVICE_STATE_INIT;
 break;
 }
 }
}
static void prepareTxFrame(uint8_t port) {
Serial.print(“Time:”);
 Serial.println(millis());
Serial.print(“Temperature = “);
 Serial.print(temp);
 Serial.println(“ °C”);
 
 Serial.print(“Humidity = “);
 Serial.print(humid);
 Serial.println(“ %”);
Serial.print(“Pressure = “);
 Serial.print(pa);
 Serial.println(“ hPa”);
Serial.print(“Approx. Altitude = “);
 Serial.print(alt);
 Serial.println(“ m”);
Serial.println();
 
 lpp.reset(); // clear the buffer
 lpp.addTemperature(2, temp);
 lpp.addRelativeHumidity(3, humid);
 //lpp.addBarometricPressure(4, pa / 1000);
 lpp.addAnalogInput(5, alt);
appDataSize = lpp.getSize();
 uint8_t lppBuffer[128];
 memcpy(lppBuffer, lpp.getBuffer(), 128);
 memcpy(appData, lppBuffer, lpp.getSize());
}
