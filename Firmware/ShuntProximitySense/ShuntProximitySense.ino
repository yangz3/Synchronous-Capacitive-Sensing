#include <CapacitiveSensor.h>

/*
 * CapitiveSense Library Demo Sketch
 * Paul Badger 2008
 * Uses a high value resistor e.g. 10 megohm between send pin and receive pin
 * Resistor effects sensitivity, experiment with values, 50 kilohm - 50 megohm. Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of foil/metal on this pin
 * Best results are obtained if sensor foil and wire is covered with an insulator such as paper or plastic sheet
 */

uint16_t serial_buffer [1];

CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);        // 10 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil
CapacitiveSensor   cs_4_5 = CapacitiveSensor(4,5);        // 10 megohm resistor between pins 4 & 6, pin 6 is sensor pin, add wire, foil
CapacitiveSensor   cs_4_8 = CapacitiveSensor(4,8);        // 10 megohm resistor between pins 4 & 8, pin 8 is sensor pin, add wire, foil

void setup()                    
{

   cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
   Serial.begin(115200);

}

void loop()                    
{
    long start = millis();
    //long total1 =  cs_4_2.capacitiveSensor(30);
    //long total2 =  cs_4_5.capacitiveSensor(30);
    long total3 =  cs_4_8.capacitiveSensor(30);
    serial_buffer[0] = (uint16_t)total3;
    
    Serial.write((uint8_t *)serial_buffer, sizeof(serial_buffer));
    Serial.write(0x80);                 // print sensor output 1
    

    delay(10);                             // arbitrary delay to limit data to serial port 
}


static inline uint16_t escapeTerminator (uint16_t v){
  if((v & 0xff) == 0x80){
    v++;
  }

  if((v >> 8) == 0x80){
    v += 0x100 - (v & 0xff); 
  }
  return v;
}



