/*	File:  IDtimer.ino
 *
 *	IDtimer.ino - A simple ID Timer sketch supporting the Yavapai
 *                    Amateur Radio Club (YARC) ID Timer Project.
 *
 *	Copyright (C) 2016 Douglas Theriault - NO1D
 *
 *	IDtimer.ino is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	IDtimer.ino is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with IDtimer.ino.  If not, see <http://www.gnu.org/licenses/>.
 */


//
// State Machine Definitions.
//
#define STATE_IDLE 0
#define STATE_TIMING 1
#define STATE_ALARM 2
#define STATE_ERROR 3
#define STATE_TEST 4

//
// Timeouts are in ms; 1000 ms per minute
// NOTE:  Values were adjusted after manual measurements for each timer
// setting.  These were tested on an ATtiny85 using internal 8Mhz clocking.
//
#define TIMEOUT_10M 6097
#define TIMEOUT_5M  3048
#define TIMEOUT_3M  1825
#define TIMEOUT_2M  1217

// 
// Timer Selection Voltage Divider menu input values
//
// Based on scaling the input ADC from 10bits down to 4bits.
//
#define TSEL_10M 13
#define TSEL_5M  9
#define TSEL_3M  5
#define TSEL_2M  1
#define TSEL_15M 16

//
// Some Global Variables
//
int ID = 0;
int TX = 1;
int PTT = 4;
int TSEL = 3;
int ALARM = 2;
int ATIME = 100;
int PTT_State = 0;
int TSEL_Value = 0;
int timeout = 0;
int fsm_state = STATE_IDLE;


// 
// Function:  Setup()
//
// Description:
//
// Called once during initialization.  Used to setup the 
// various pins of the ATtiny85.
//
void setup()
{
  pinMode(ID, OUTPUT);
  pinMode(TX, OUTPUT);
  pinMode(PTT, INPUT_PULLUP);
  pinMode(TSEL, INPUT);
  pinMode(ALARM, OUTPUT);

  // default analog vref of 5v.
  analogReference(DEFAULT);

  // TODO:  May not be needed
  for (int x = 0; x < 100; x++) {
    int value = analogRead(TSEL);
  }
}

//
// Function:  alarm()
//
// Description:
//
// Sounds the Piezo alarm buzzer.
//
void alarm()
{
  for (int x = 0; x < ATIME; x++) {
    digitalWrite(ALARM, HIGH);
    delay(1);
    digitalWrite(ALARM, LOW);
    delay(1);
  }
}

// 
// Function:  readPTT()
//
// Description:
//
// Reads state of the PTT pin 3/D4 on the ATtiny85.  LOW indicates
// PTT is not set, HIGH indicates active and that we should start the
// timing operation.
//
void readPTT() {

  PTT_State = digitalRead(PTT);

  if (PTT_State == LOW) {
    fsm_state = STATE_IDLE;
  }
  if (PTT_State == HIGH) {
    fsm_state = STATE_TIMING;
  }
}

// 
// Function:  readTSEL()
//
// Description:
//
// Reads the Timer Selection voltage from the voltage divider network.
// Input is scaled from 10bits down to 4bits first, then timer selection
// is determined and proper timer selection is returned.
//
int readTSEL() {

  int rValue = 0;

  rValue = analogRead(TSEL);    // Read Timer Select, 10bits

  rValue = map(rValue, 0, 1023, 0, 16);

  if (rValue > TSEL_10M) return TSEL_15M;
  if (rValue > TSEL_5M)  return TSEL_10M;
  if (rValue > TSEL_3M)  return TSEL_5M;
  if (rValue > TSEL_2M)  return TSEL_3M;

  return TSEL_2M;
}

//
// Function:  checkTimeout()
//
// Description:
//
// Basic switch statement to determine if we have reached timeout condition for
// a given timer selection. 
// 
void checkTimeout() {

  // Here we check for timeouts based on the configuration from TSEL switch
  // This actually could be performed inside the Timing state only (NOTE: move???)
  //

  // Now check to see if timeout has expired based on the user
  // configured setting TSEL_Val.
  switch (TSEL_Value) {

    case TSEL_10M:
      if (timeout >= TIMEOUT_10M) {
        fsm_state = STATE_ALARM;
      }
      break;

    case TSEL_5M:
      if (timeout >= TIMEOUT_5M) {
        fsm_state = STATE_ALARM;
      }
      break;

    case TSEL_3M:
      if (timeout >= TIMEOUT_3M) {
        fsm_state = STATE_ALARM;
      }
      break;

    case TSEL_2M:
      if (timeout >= TIMEOUT_2M) {
        fsm_state = STATE_ALARM;
      }
      break;

    default:      // Not clear on value
      fsm_state = STATE_ERROR;
  }
}

//
// Function:  loop()
//
// Description:
//
// This is the Arduino sketch main loop routine.  This routine is entered after the
// call to setup() and should never return.  This is the main routine for the ID
// Timer routine.
//
void loop()
{

  // Basic state machine.  IDLE is such, Timing means we're counting how long PTT
  // has been active.  If Timer expires, enter Alarm state where we sound the ID
  // buzzer.  Once PTT released, return to the IDLE state.
  //

  switch (fsm_state) {
    case STATE_IDLE:
      // Reset timeout counter and turn off indicators
      //
      timeout = 0;
      digitalWrite(TX, LOW);
      digitalWrite(ID, LOW);

      // Read TSEL for timer configuration requested
      TSEL_Value = readTSEL();

      // Poll the PTT pin
      PTTRead();
      break;

    case STATE_TIMING:
      // Light the TX led to indicate PTT set and we're transmitting
      digitalWrite(TX, HIGH);

      // While in this state we increment a timeout counter
      timeout = timeout + 1;

      // Poll the PTT pin to see if released before timer expires
      PTTRead();

      // Check to see if we hit timeout value
      checkTimeout();
      break;

    case STATE_ALARM:
      // Once a second beep the ID alarm.  This anoyance continues until the
      // user releases PTT and returns to the IDLE state.  Or unsolders the
      // Piezo speaker.
      alarm();
      
      // In addition, light the ID indicator
      digitalWrite(ID, HIGH);

      // Delay so we're not too annoying
      delay(500);
      
      // Poll the PTT pin to see when they release and clear timer/alarm
      PTTRead();
      break;

      // Should never get here !  Ever, did I say ever?  We can get here if
      // the measurement of the TSEL voltage divider is outside our valid range.
      // But really, should never get here.
    case STATE_ERROR:
      digitalWrite(TX, HIGH);
      digitalWrite(ID, LOW);
      delay(200);
      digitalWrite(TX, LOW);
      digitalWrite(ID, HIGH);
      delay(200);
      digitalWrite(TX, HIGH);
      digitalWrite(ID, LOW);
      delay(200);
      digitalWrite(TX, LOW);
      digitalWrite(ID, HIGH);
      delay(200);
      digitalWrite(TX, LOW);
      digitalWrite(ID, LOW);
      alarm();
      // Poll the PTT pin to see when they release and clear timer/alarm
      PTTRead();
      break;

      // And should never get here.  So force back to IDLE
    default:
      fsm_state = STATE_IDLE;
      break;
  }

  // Delay 100ms between polling period.  This is the basis for our timers.  The
  // timeout value is incremented once during this polling period.  Changes here
  // mean changes to the timeout values so timing remains accurate.
  //
  delay(100);  // delay 100ms

}
