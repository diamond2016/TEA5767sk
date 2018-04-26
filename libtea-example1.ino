#include <libtea5767.h>

TEA5767 radio;
void setup() {
  // put your setup code here, to run once:
radio.begin(90.70);
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly
  radio.keypadMenu();
  Serial.print ("RF: "); Serial.println(radio.getRF());
  Serial.print ("PLL: "); Serial.println(radio.getPLL());
  Serial.println(radio.printFrequency());
  delay(800);
}
