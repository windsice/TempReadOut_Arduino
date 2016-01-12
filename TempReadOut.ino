/*
 * File: TempReadOut
 * Purpose: Use Arduino to read temperature out from a ADT7310.
 *          ADT7310 communicate Arduino through SPI interface.
 *          
 * 
 * ADT7310 dataSheet:
 * http://www.analog.com/media/en/technical-documentation/data-sheets/ADT7310.pdf
 * 
 */

#include <SPI.h>

/* 
 * Uno SPI Connection:
 * MOSI  - 11 or ICSP-4
 * MISO  - 12 or ICSP-1
 * SCK   - 13 or ICSP-3
 * CS/SS - 10
 */
const int CSpin = 10;
const int MOSIpin = 11;
const int MISOpin = 12;
const int SCKpin = 13;
const byte READ  = 0b01000000;
const byte WRITE = 0b00000000;

void sendCommand(uint8_t thisReg, uint8_t RW){

  Serial.print("Reg #");
  Serial.print(thisReg,HEX);
  Serial.print("\t");
  uint8_t command = (thisReg << 3) | RW;
  // Max speed of communication for the chip: dataIn min interval = 100ns -> 10MHz, or 10000000, choose the multiple of 2, thus 8000000
  // CPOL = 1, CPHA = 1
  // BUT SPI.begin() will set the SCK to low initially, thus CPOL -> 0, thus Mode = 0 
  // Ref: https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Clock_polarity_and_phase
  SPI.beginTransaction(SPISettings(8000000,MSBFIRST,SPI_MODE0));
  digitalWrite(CSpin,LOW);
  Serial.print("Sending Command: ");
  Serial.print(command,BIN);
  Serial.print("\t");
  SPI.transfer(command);
  delay(250);
  uint8_t foo = SPI.transfer(0);
  Serial.println(foo,HEX);
  digitalWrite(CSpin,HIGH);
  SPI.endTransaction();
}

void resetChip(){
  SPI.beginTransaction(SPISettings(8000000,MSBFIRST,SPI_MODE0));
  digitalWrite(CSpin,LOW);
  // a reset occurs if 32 consecutive 1s are seen on the DIN pin.
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  digitalWrite(CSpin,HIGH);
  SPI.endTransaction();
}

void setup() {
  Serial.begin(9600);

  pinMode(MISOpin,INPUT);

  SPI.begin();  
  resetChip();
  startReadTemp();
}

void readDefault(){
    /*
   * Code to test connection
   * default values:
   * Reg 0: 0x80
   * Reg 1: 0x00
   * Reg 2: 0x0000
   * Reg 3: 0xCX
   * Reg 4: 0x4980
   * Reg 5: 0x05
   * Reg 6: 0x2000
   * Reg 7: 0x0500
   */
  if (Serial.read() > 0) {
    Serial.print("\n\n\n\n\n\n\n\n\n\n");
    for(int i = 0; i < 6; i++)
    {
      if(i == 0 || i == 1 || i == 3 || i == 5)
      sendCommand(i,READ);
    }
  }
}

//continuous read mode
void startReadTemp(){
  SPI.beginTransaction(SPISettings(8000000,MSBFIRST,SPI_MODE0));
  digitalWrite(CSpin,LOW);

  uint8_t command = 0b01010100; //read temp continuously
  SPI.transfer(command);
}

void displayTemp(){

  delay(1000);
  uint16_t foo = SPI.transfer16(0);
  // 13-bit resolution, temp represented by [15:3]
  foo = foo >> 3;
  Serial.println(toTemp(foo));
}


float toTemp(uint16_t temp){ 

  float resolution = 0.0625;
  float Celsius = 0.0;
  bool positive;
  if((temp>>12) == 0)
    positive = true;
  else{
    positive = false;
    temp = ~temp + 1;
  }
  byte pos = 0;
  while(temp)
  {
    if(temp & 1)
      Celsius += pow(2,pos)*resolution;
    temp >>= 1;
    pos++;
    if(pos == 12)
      break;
  }

  if(positive)
    return Celsius;
  else
    return -Celsius;
}

void loop() {
  // put your main code here, to run repeatedly:
  displayTemp();
}
