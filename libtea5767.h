/***************************************************************************** 
  libtea.h
  This is a library for the TEA5767HN "FM radio on chip"
  The chip is from NXP, see datasheet for example at: 
  ----> https://www.sparkfun.com/datasheets/Wireless/General/TEA5767.pdf
  This chip is interfaced by I2C bus, so you can communicate with it via 
  the Arduino Wire library, writing and reading its registers. 

  This library uses an LCD display to print frequency (LiquidCrystal library)
  This library uses an optional keypad Arduino 4x4 (Keypad library)

  https://github.com/diamond2016/TEA5767sk
  Written by Riccardo G Corsi - 1st release april 2018.  
  GNU General Public License v3.0
*****************************************************************************/

#ifndef LIBTEA_H
#define LIBTEA_H


#include "Arduino.h"
#include "Wire.h"

#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include "Keypad.h"

// address to interface the TEA5767 
#define TEA5767_I2CADDR 0x60

// range of frequencies of TEA5767 
#define FREQ_LOW 87
#define FREQ_HIGH 108
#define NUM_DATABYTE 5



class TEA5767 {
 public:
  TEA5767();
   
  void begin(float frq);  // initialize registers and sets initial frequency
  char* printFrequency();
  char* printFrequency(float frq);
  void readRegisters(void);
  void writeRegisters(void);
  boolean getRF(void);
  boolean getBLF(void);
  uint16_t getPLL(void);
  float getFreq(void);
  void setFreq(float frq);
  void seek(void);
  void keypadMenu(void);
  
 private:
  void initRegisters(void);
  void keypadFreq(byte val);
  boolean rf;
  boolean blf;
  uint16_t pll;
  float freq;
  char freqDisplay[12];
  boolean stereo;
  byte lev;
  byte ci;

  byte registers[NUM_DATABYTE];
  byte registers_read[NUM_DATABYTE];
  byte posFreq;
  float newFreq;

/**** section used only if you have the Arduino 4x4 keypad **/
  static const byte ROWS = 4;  // four rows
  static const byte COLS = 4;  // three columns
  static const byte CHANS = 4; // 4 channels pre-stored
  char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
  };
  byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
  byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

  // some (Italy) fm radio frequency change as you wish
  float chans[CHANS] = { 88.6,  // RadioMaria
                       94.86, // RadioUNO
                       97.2,  // Radio 2
                       90.7,  // RadioFreccia
  };  
  static const byte FREQ_DIGITS = 5;
  byte FREQUENCY[FREQ_DIGITS];
  // I2C AZDelivery 
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,2,1,0,4,5,6,7,3,POSITIVE);
  // Keypad 4x4
  Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );  

}; // TEA5767 class


#endif //  LIBTEA_H
