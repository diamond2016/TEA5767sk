/***************************************************************************** 
  libtea.cpp

  This is a library for the TEA5767HN "FM radio on chip"
  The chip is from NXP, see datasheet for example at: 
  ----> https://www.sparkfun.com/datasheets/Wireless/General/TEA5767.pdf
  This chip is interfaced by I2C bus, so you can communicate with it via 
  the Arduino Wire library, writing and reading its registers. 

  This library uses an LCD display to print frequency (LiquidCrystal library)
  This library uses an optional keypad (Arduino 4x4)

  Written by Riccardo G Corsi - 1st release april 2018.  
  GNU General Public License v3.0
*****************************************************************************/

#include "libtea5767.h"

// default constructor 
TEA5767::TEA5767(void){}

// init the chip
void TEA5767::begin(float frq)
{ 
  freq = frq;
  Wire.begin();

  lcd.begin(16,2);
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print(">TEA5767 FMRadio<");
  initRegisters();
  lcd.setCursor(0,1);
  lcd.print(printFrequency(frq));  

  // Serial.begin(9600);
}


// print frequency
char* TEA5767::printFrequency(float frq) {
  int integral_part = (int) frq;
  int decimal_part = (frq - integral_part)*100;
  sprintf (freqDisplay, "%d.%d MHz", integral_part, decimal_part);
  return freqDisplay;
}
  
char* TEA5767::printFrequency(void) {
  return printFrequency(freq);
}


// initialize registers
void TEA5767::initRegisters() {
  // set registers 1 && 2 (index 0 and 1)
  setFreq(freq);
  // set register 3 (index 2)
  registers[2] = 0xD0; // search up, mid volume, high side injection, stereo, no mute
  // set register 4 (index 3)
  registers[3] = 0x13; // eu band,clock 32768, stero noise cancelling on, ready flag SWPORT1
  // set register 5 (index 4)
  registers[4] = 0x00;
  writeRegisters();
  delay(10);
}


// read registers
void TEA5767::readRegisters() {
  byte c;
  
  Serial.println();
  Wire.requestFrom (TEA5767_I2CADDR,NUM_DATABYTE);
  for (int i = 0; i < NUM_DATABYTE; i++)
      if (Wire.available()) {
        c = Wire.read();
        //Serial.println(c, HEX);
        registers_read[i] = c;        
      }
  if (registers_read[0] & 0x80)
    rf = true;
  if (registers_read[0] & 0x40)
    blf = true;  
}


// write registers
void TEA5767::writeRegisters() {
  Wire.beginTransmission(TEA5767_I2CADDR);
  for (int i = 0; i < NUM_DATABYTE; i++) {
      Wire.write(registers[i]);
    }
  Wire.endTransmission();
}


// read RF flag (a frequency is set)
boolean TEA5767::getRF() { 
  readRegisters();
  return rf;
}

// read BLF flag
boolean TEA5767::getBLF() {
  readRegisters();
  return blf;
}

// read PLL (internal counter for frequency in TEA5767)
uint16_t TEA5767::getPLL() { 
  readRegisters();
  pll = 0;

  pll = registers_read[0] & 0x3F;
  pll = pll << 8;
  pll = pll + registers_read[1];
  return pll; 
}

// read actual frequency in Mhz and displays in LCD
float TEA5767::getFreq() { 
  float frq;

  readRegisters();
  pll = 0;

  pll = registers_read[0] & 0x3F;
  pll = pll << 8;
  pll = pll + registers_read[1];

  frq = ((pll * 32768) / 4) - 225000.;
  freq = frq / 1000000;
  lcd.setCursor(0,1);
  lcd.print(printFrequency(freq));  
  return freq; 
}

// sets frequency (and pll), frequency in MhZ
void TEA5767::setFreq(float frq) { 
  
  pll = (4 * (frq * 1000000 + 225000)) / 32768; 
  registers[0] = pll >> 8;
  registers[1] = pll & 0x00FF;

  frq = ((pll * 32768) / 4) - 225000.;
  freq = frq / 1000000;
  lcd.setCursor(0,1);
  lcd.print(printFrequency(freq)); 
  writeRegisters();
}

// seek (forward)
void TEA5767::seek() { 
  registers[0] |= 0x40; // SM = 1
  registers[2] |= 0x80; // SUD = 1 
  writeRegisters();
  getFreq();
  delay(100); 
}


// read from keypad
// A,B,C,D are the four channels preconfigured
// * seeks
// 99999# (min 3 digits max 5 digits plus # sets the frequency e.g. 10325# = 103.25Mhz

void TEA5767::keypadMenu() {
  char key = 0;
  float frq = 0;
  
  key = keypad.getKey();
  if (key == NO_KEY)
    return;
  Serial.println(key);
  
  switch (key) {
    // first channel in memory
    case 'A':
      setFreq(chans[0]);
      break;
    case 'B':
      setFreq(chans[1]);
      break;
    case 'C': 
      setFreq(chans[2]);
      break;
    case 'D':
      setFreq(chans[3]);
      break;           
    case '*':
      seek();
      break;
    case '0':
      keypadFreq(0);
      break;
    case '1':
      keypadFreq(1);
      break;
    case '2':
      keypadFreq(2);
      break;
    case '3':
      keypadFreq(3);
      break;
    case '4':
      keypadFreq(4);
      break;
    case '5':
      keypadFreq(5);
      break;
    case '6':
      keypadFreq(6);
      break;
    case '7':
      keypadFreq(7);
      break;
    case '8':
      keypadFreq(8);
      break;
    case '9':
      keypadFreq(9);
      break;
    case '#':
      setFreq(newFreq);
      newFreq = 0;
      posFreq = 0;
      break;                              
    default:
      break;
  }
}

// private function to set a frequency from keypad
// press #99999# to setfrquency in format 999.99 Mhz
void TEA5767::keypadFreq(byte val) {
  char key = 0;
  
  // read a fix format ABCDE#
  // A = frequency hundredths MHz
  // eg 10325 A=1 B=0 C=3 D=2 E=5
  // use 0 to complete 

  switch (posFreq) {
    // A
    case 0:
      newFreq = 100 * val ;
      posFreq++;
      break;
   // B
   case 1:
      newFreq += 10 * val;
      posFreq++;
      break;
  // C
  case 2:
      newFreq += val;
      posFreq++;
      break;
// D
  case 3:
      newFreq += 0.1 * val;
      posFreq++;
      break;
// E
  case 4:
      newFreq += 0.01 * val;
      posFreq++;
      break;

      default:
      break;
  }
 
}
