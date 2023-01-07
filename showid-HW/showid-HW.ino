uint64_t chipId = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
//    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  chipId=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipId>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipId);//print Low 4bytes.

//  Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
//  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
//  Serial.print("Chip ID: "); Serial.println(chipId);
  
  delay(3000);

}
