/***
 * Original Code: https://github.com/matthijskooijman/arduino-lmic/blob/master/examples/ttn-abp/ttn-abp.ino
 * Modified By: Bikash Narayan Panda (weargeniuslabs@gmail.com) 
 * ***/
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <U8x8lib.h>


static const PROGMEM u1_t NWKSKEY[16] = { 0xD2, 0x13, 0x33, 0xF4, 0xB4, 0xAA, 0xA0, 0x50, 0xD0, 0x61, 0xAE, 0xD4, 0x8F, 0xDF, 0x95, 0x86 };
static const u1_t PROGMEM APPSKEY[16] = { 0x8C, 0x3E, 0x03, 0xAC, 0xA4, 0xCC, 0xD5, 0xAC, 0x22, 0xD8, 0x0B, 0xF1, 0xE7, 0x8E, 0x48, 0x7F };
static const u4_t DEVADDR = 0x260BE5D2 ; // <-- Change this address for every node!

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static uint8_t mydata[] = "Hi from WGLabz!";
static osjob_t sendjob;

#define SCK     5    // GPIO5  -- heltec
#define MISO    19   // GPIO19 -- heltec
#define MOSI    27   // GPIO27 -- heltec
#define SS      18   // GPIO18 -- heltec
#define RST     14   // GPIO14 -- heltec
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)


// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = SS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = RST,
  .dio = {DI0, 3, 4},
};
//OLED Declaration 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

//LED and Button Pins
int buttonPin = 13;
int ledPin=12;
int boardLED=25;
int lastState=0;

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_TXCOMPLETE:
           ledFLash(2);
           Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
           u8x8.drawString(0, 2, "Data Sent");
           u8x8.drawString(0, 4, "Button Released");
           if(LMIC.dataLen) {
               // data received in rx slot after tx
               Serial.print(F("Data Received: "));
               u8x8.drawString(0, 3, "Data Received: ");
               Serial.print(LMIC.dataLen);
               Serial.print(F(" bytes for downlink: 0x"));
               for (int i = 0; i < LMIC.dataLen; i++) {
                   if (LMIC.frame[LMIC.dataBeg + i] < 0x10) {
                       Serial.print(F("0"));
                   }
                   Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
               }
               Serial.println();
           }
           // Schedule next transmission
           // os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
           delay(2000);
           u8x8.clearLine(1);
           u8x8.clearLine(2);
           u8x8.clearLine(3);
           u8x8.clearLine(4);
            break;
         default:
            Serial.println(F("Unknown event"));
            u8x8.drawString(0, 2, "Unknown event");
            break;
    }
}

void ledFLash(int flashes){
    int lastStateLED=digitalRead(ledPin);
    for(int i=0;i<flashes;i++){
        digitalWrite(ledPin, HIGH);
        delay(300);
        digitalWrite(ledPin, LOW);
        delay(300);
    }
    digitalWrite(ledPin,lastStateLED);
}
void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    u8x8.drawString(0, 4, "Button Pressed");
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
        u8x8.drawString(0, 1, "OP_TXRXPEND, not sending");
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
        u8x8.drawString(0, 1, "Packet queued");
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    //setup the display
    u8x8.begin();
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.drawString(0, 0, "WGLabz LoRa Test");

    Serial.begin(115200);
    Serial.println(F("Starting"));

    //In/Out Pins
    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    digitalWrite(buttonPin, HIGH);

    // LMIC init &RESET
    os_init();
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly 
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

//    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
//    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

    LMIC_setupChannel(0, 923200000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 923400000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 923600000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 923800000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 924000000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 924200000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 924400000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 924600000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 924500000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band


    // Disable link check validation
    LMIC_setLinkCheckMode(0);
    LMIC.dn2Dr = DR_SF9;
    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // In my place we are not using 868 MHz
    LMIC_disableChannel(0);
    LMIC_disableChannel(1);
    LMIC_disableChannel(2);
}

void loop() {
    os_runloop_once();
    if(!digitalRead(buttonPin)){
        //
        delay(400);
        do_send(&sendjob);
        //
    }
}
