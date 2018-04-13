// RADIO TEA5767sk 
// This is a sketch to manage the TEA5767 chip - and FM on chip
// With ths sketch you can set frequency reaf PLL/freq, seek frequency/
// The skecth uses and I2C LCd module (AzDelivery I2C LCd module) and the LiquidCrystal_I2C library
// Riccardo G Corsi 13.04.2018

#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define FREQ_LOW 87
#define FREQ_HIGH 108
#define FREQ_INIT 90.70

char buf[14]; 

#define NUM_DATABYTE 5
#define TEA5767_ADR 0x60
byte registers[NUM_DATABYTE];
byte registers_read[NUM_DATABYTE];

struct radio {
  boolean rf;
  boolean blf;
  unsigned int pll;
  float freq;
  boolean stereo;
  byte lev;
  byte ci;
} radioTEA5767;


// I2C AZDelivery 
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3,POSITIVE);


void setup()
{ 
  radioTEA5767.freq = FREQ_INIT;
  Wire.begin();
  Serial.begin(9600); 
  lcd.begin(16,2);
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print(">TEA5767 FMRadio<");
  init_registers();
  lcd.setCursor(0,1);
  lcd.print(printFreq(radioTEA5767.freq));  
}


void loop()
{
  read_registers();
  Serial.println (printFreq(radioTEA5767.freq));
  delay(500);
}


// print frequency
char *printFreq(float frq) {
  int integral_part = (int) frq;
  int decimal_part = (frq - integral_part)*100;
  sprintf (buf, "%d.%d MHz", integral_part, decimal_part);
  return buf;
}


// initialize registers
void init_registers() {
  // set registers 1 && 2 (index 0 and 1)
  setPLL(radioTEA5767.freq);
  // set register 3 (index 2)
  registers[2] = 0xD0; // search up, mid volume, high side injection, stereo, no mute
  // set register 4 (index 3)
  registers[3] = 0x13; // eu band,clock 32768, stero noise cancelling on, ready flag SWPORT1
  // set register 5 (index 4)
  registers[4] = 0x00;
  write_registers();
  delay(10);
}


// read registers
void read_registers() {
  byte c;
  
  Serial.println();
  Wire.requestFrom (TEA5767_ADR,NUM_DATABYTE);
  for (int i = 0; i < NUM_DATABYTE; i++)
      if (Wire.available()) {
        c = Wire.read();
        //Serial.println(c, HEX);
        registers_read[i] = c;        
      }
}


// write registers
void write_registers() {
  Wire.beginTransmission(TEA5767_ADR);
  for (int i = 0; i < NUM_DATABYTE; i++) {
      Wire.write(registers[i]);
    }
  Wire.endTransmission();
}


float getRF() { 
  boolean rf, blf;
  
  rf = false;
  blf = false;
  if (registers_read[0] & 0x80)
    rf = true;
  if (registers_read[0] & 0x40)
     blf = true;
  radioTEA5767.rf = rf;
  radioTEA5767.blf = blf;
  return rf; 
}

unsigned int getPLL() { 
  unsigned int pll;
  float frq;
  radioTEA5767.pll = 0;

  pll = registers_read[0] & 0x3F;
  pll = pll << 8;
  pll = pll + registers_read[1];
  radioTEA5767.pll = pll;

  frq = ((pll * 32768) / 4) - 225000.;
  radioTEA5767.freq = frq / 1000000;
  lcd.setCursor(0,1);
  lcd.print(printFreq(radioTEA5767.freq));  
  return pll; 
}

unsigned int setPLL(float frq) { 
  unsigned int pll;
  radioTEA5767.pll = 0;
  
  pll = (4 * (frq * 1000000 + 225000)) / 32768; 
  registers[0] = pll >> 8;
  registers[1] = pll & 0x00FF;
  radioTEA5767.pll = pll;

  frq = ((pll * 32768) / 4) - 225000.;
  radioTEA5767.freq = frq / 1000000;
  lcd.setCursor(0,1);
  lcd.print(printFreq(radioTEA5767.freq)); 
  write_registers();
  return pll; 
}

void seekDown() { 
  registers[0] |= 0x40; // SM = 1
  registers[2] &= 0x7F; // SUD = 0 
  write_registers();
  getPLL();
  delay(500); 
}

void seekUp() { 
  registers[0] |= 0x40; // SM = 1
  registers[2] |= 0x80; // SUD = 1 
  write_registers();
  getPLL();
  delay(500); 
}


