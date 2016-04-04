/*  File:  Calibrate85.ino

  IDtimer.ino - A simple ATtiny85 Calibrate routine for the Yavapai
                Amateur Radio Club (YARC) ID Timer Project.

  Copyright (C) 2016 Douglas Theriault - NO1D

  Calibrate85.ino is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Calibrate85.ino is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with IDtimer.ino.  If not, see <http://www.gnu.org/licenses/>.

  References/Attributions:

  http://www.instructables.com/answers/How-to-get-an-ATtiny-to-run-an-accurate-clock/

  Particular thanks to: baycollie for the setup instructions
  
*/

#include <EEPROM.h>

#define OSCCAL_ADDR 0     // First byte of EEPROM is OSCCAL value
#define CALIBRATION 104   // Tweak for accurate frequency measurement

void setup()
{
  // Select value of OSCCAL 0..255 to adjust frequency of internal RC
  // Oscillator of the ATtiny85.  Should work with other AVR chips too.
  OSCCAL = CALIBRATION; // set OSCCAL to our calibration value
  
  CLKPR = 0x80; // set system clock to 8mhz with no prescale
  CLKPR = 0x00; // these 2 CLKPR instructions have to be run together in order 
                // to set clock to 8 Mhz
  TCNT0 = 0;    // initialize timer counter value to 0
  TCCR0A = 0;   // write 0 to timer 0 control registers
  TCCR0B = 0;
  
  TCCR0A |= (1 << COM0A0); //Timer0 in toggle mode (Table 11-2 of datasheet)
  TCCR0A |= (1 << WGM01);  //Start timer 1 in CTC mode (Table 11-5 )
  TCCR0B |= (1 << CS00);   // Prescaler table 11.6
  
  OCR0A = 0;               // 0 means Fclk/2 or 4Mhz

  // Set PB0 for OUTPUT mode for scope hookup.  Adjust as needed for
  // other chips/pins.
  pinMode(PB0, OUTPUT);
  pinMode(PB1, OUTPUT);
  
  
}

void loop() {

  // Write calibration value into first location of EEPROM
  EEPROM.write(OSCCAL_ADDR, CALIBRATION);

  // EEPROM writes take 3.3ms
  delay(100);

  // DEBUG:  read value from EEPROM, if not same as calibration value, 
  //         toggle PB1 so I can see on the scope.  
  //
  //         For some unknown reason, when I read value back out of EEPROM
  //         in the IDTimer.ino sketch, OSCCAL does not get set properly.
  //         
  for(;;) {
      if (EEPROM.read(OSCCAL_ADDR) != CALIBRATION) {
        digitalWrite(PB1, 1);
        delay(500);
        digitalWrite(PB1, 0);
        delay(500);
      }
  }
}
