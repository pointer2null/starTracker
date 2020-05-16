/*
 *  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * (c) 2020 Gary Butler grbutler@gmail.com 
 * 
 * StarTracker control. When a Nema 17 or equivelent stepper is connected to a compatable controller
 * and the output is geared as shown, then the final output shaft should revolve once per siderial day.
 * 
 * Hardware used:
 * Nema 17 stepper with 99.0506:1 planetory gearbox
 * final drive 3:1 toothed belt
 * TB6600 stepper driver.
 * Arduino Uno R3
 * 0.91 INCH OLED Display: Resolution is 128*32
 * 8 x push buttons
 * 
 * Siderial day = 86164.1 seconds
 * gearbox 99.0506:1
 * pully 3:1
 * 200 steps 16 microsteps
 *
 * each microstep = 0.0906145625seconds = 90.6145625ms
 *
**/

#include <U8g2lib.h>
#include <Bounce2.h>

// Outputs
# define PULSE       13                   // pin 8 is the step    
# define DIRECTION   12                   // pin 9 is DIRECTION
# define ENABLE      11                   // pin 10 is the enable

// Inputs
# define BTN_START   10                   // start
# define BTN_RST     9                    // reset
# define BTN_UP      8                    // speed + 
# define BTN_FFWD    7                    // FFWD
# define BTN_RWD     6                    // REW
# define BTN_DWN     5                    // speed -
# define BTN_REVERSE 4                    // Reverse DIRECTION
# define BTN_STOP    3                    // stop

// Constants
# define ON               true
# define OFF              false
# define FORWARD          true
# define REVERSE          false
# define messageShowTime  3000            // count time for a message to display before status returns

# define SLOW             80              // switch count before we go to hold
# define FAST             160             // switch count before we go to long hold

# define DEFAULT_H        11315           // default high drive period
# define DEFAULT_L        22635           // default low drive period (must be <65536)
# define H_SPEED_L        1000            // default low drive for high speed
# define SPEED_STEP_S     2               // speed inc/dec step amount (inc/red low period)
# define SPEED_STEP_F     20              // speed inc/dec fast step amount

# define BOUNCE_WITH_PROMPT_DETECTION
# define FONT u8g2_font_logisoso16_tr     // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall

enum button_op {BON, BOFF, BHOLD, BLONG, BWAIT};

// Control variables
bool          direction      = FORWARD;   // currect direction : FORWARD = CCW
bool          lastDirection  = FORWARD;
bool          enabled        = OFF;       // Drive on/off
bool          ffwd           = OFF;       // in ffwd mode
bool          rwd            = OFF;       // in rwd mode
int           high_period    = DEFAULT_H; // current drive high period
int           low_period     = DEFAULT_L; // current drive low period

const char    *dispMessage;               // current message
unsigned long msgSetTime     = 0;         // timestamp for new message

// counters for button press
unsigned long rstPressCount  = 0;
unsigned long upPressCount   = 0;
unsigned long dwnPressCount  = 0;
unsigned long rwdPressCount  = 0;
unsigned long revPressCount  = 0;
unsigned long ffwdPressCount = 0;

// debouncers
Bounce rev_b   = Bounce();
Bounce rwd_b   = Bounce();
Bounce up_b    = Bounce();
Bounce dwn_b   = Bounce();
Bounce ffwd_b  = Bounce();
Bounce reset_b = Bounce();

// display voodoo
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

// TCNTx  actual timer value
// OCRx   output compare register
// TIMSKx timer/counter interrupt mask - to enabled/dsable interupts
// TIFRx  timer/counter interupt flag register - indicated pending interrupt

void setup() {
  u8g2.begin(); // Display
  showMsg("WELCOME");
  displayMessage();

  // initialize timer1
  cli();             // stop interrupts
  TCCR1A = 0;        // set entire TCCR1A register to 0
  TCCR1B = 0;        // same for TCCR1B
  TCNT1  = 0;        // initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = DEFAULT_L;
  OCR1B = DEFAULT_H;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 and CS10 bits for 64X prescaler
  TCCR1B |= (1 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A) | (1 << OCIE1B);
  //allow interrupts
  sei();

  pinMode(ENABLE,            OUTPUT);
  pinMode(DIRECTION,         OUTPUT);
  pinMode(PULSE,             OUTPUT);

  pinMode(BTN_START,         INPUT_PULLUP);
  pinMode(BTN_STOP,          INPUT_PULLUP);

  reset_b.attach(BTN_RST,    INPUT_PULLUP);
  ffwd_b.attach(BTN_FFWD,    INPUT_PULLUP);
  up_b.attach  (BTN_UP,      INPUT_PULLUP);
  rwd_b.attach (BTN_RWD,     INPUT_PULLUP);
  dwn_b.attach (BTN_DWN,     INPUT_PULLUP);
  rev_b.attach (BTN_REVERSE, INPUT_PULLUP);

  // set initial direction and enable pins
  digitalWrite(DIRECTION, direction);
  digitalWrite(ENABLE,    enabled);
}


ISR(TIMER1_COMPA_vect) { //timer1 interrupt
  if (enabled) {
    digitalWrite(PULSE, HIGH);
    digitalWrite(DIRECTION, direction);
  }
  // we've reached the max so reset the count
  TCNT1  = 0;
}

ISR(TIMER1_COMPB_vect) { //timer1 interrupt
  digitalWrite(13, LOW);
}


void loop() {
  readButtons();
  displayMessage();
}

void readButtons() {
  if (!digitalRead(BTN_START) && !enabled) {
    showMsg("START");
    enabled = ON;
    setCurrentSpeed();
  }
  if (!digitalRead(BTN_STOP) && enabled) {
    showMsg("STOP");
    enabled = OFF;
    digitalWrite(PULSE, OFF);
  }

  processButton(&reset_b, &rstPressCount,  processRstButton, "reset");
  processButton(&ffwd_b,  &ffwdPressCount, processFfwdButton, "ffwd");
  processButton(&rwd_b,   &rwdPressCount,  processRwdButton, "rwd");
  processButton(&rev_b,   &revPressCount,  processRevButton, "rev");
  processButton(&up_b,    &upPressCount,   processUpButton, "up");
  processButton(&dwn_b,   &dwnPressCount,  processDownButton, "down");
}

void setDefaultSpeed() {
  cli();//stop interrupts
  OCR1B = DEFAULT_H;
  OCR1A = DEFAULT_L;
  sei();//allow interrupts
}

void setCurrentSpeed() {
  cli();//stop interrupts
  OCR1B = (int) low_period / 2;
  OCR1A = low_period;
  sei();//allow interrupts
}


void setHighSpeed() {
  cli();//stop interrupts
  OCR1B = (int) H_SPEED_L / 2;
  OCR1A = H_SPEED_L;
  sei();//allow interrupts
}

void processFfwdButton(button_op press) {
  if (!enabled) return;
  switch (press) {
    case BON: {
        if (ffwd) {
          showMsg("FFWD OFF");
          ffwd = OFF;
          direction = lastDirection;
          setCurrentSpeed();
        } else {
          showMsg("FFWD");
          ffwd = ON;
          setHighSpeed();
        }
        break;
      }
    case DEFAULT: {
        break;
      }
  }
}

void processRwdButton(button_op press) {
  if (!enabled) return;
  switch (press) {
    case BON: {
        if (rwd) {
          showMsg("REWIND OFF");
          rwd = OFF;
          direction = lastDirection;
          setCurrentSpeed();
        } else {
          showMsg("REWIND");
          lastDirection = direction;
          ffwd = OFF;
          rwd = ON;
          setHighSpeed();
          direction = !direction;
        }
        break;
      }
    case DEFAULT: {
        break;
      }

  }
}

void processRstButton(button_op press) {
  switch (press) {
    case BON: {
        showMsg("RESET");
        ffwd = OFF;
        rwd = OFF;
        enabled = OFF;
        setCurrentSpeed();
        break;
      }
    case BHOLD:
    case BLONG: {
        showMsg("FULL RESET");
        rstPressCount = 0;
        ffwd = OFF;
        rwd = OFF;
        enabled = OFF;
        high_period    = DEFAULT_H; // current drive high period
        low_period     = DEFAULT_L; // current drive low period
        setDefaultSpeed();
        break;
      }
    case DEFAULT: {
        break;
      }
  }
}

void processRevButton(button_op press) {
  switch (press) {
    case BON: {
        ffwd = OFF;
        rwd = OFF;
        if (direction == FORWARD) {
          showMsg("CW >");
          direction = REVERSE;
        } else {
          showMsg("< CCW");
          direction = FORWARD;
        }
        break;
      }
    case DEFAULT: {
        break;
      }
  }
}

void processUpButton(button_op press) {
  if (!enabled) return;
  if (low_period <= DEFAULT_H) {
    showMsg("MAX SPEED");
    return;
  }
  switch (press) {
    case BWAIT: {
        showMsg("INCREASE");
        break;
      }
    case BON: {
        low_period -= SPEED_STEP_S;
        showMsg("INCREASE");
        break;
      }
    case BLONG:
    case BHOLD: {
        showMsg("INCREASE++");
        low_period -= SPEED_STEP_F;
        break;
      }
    case DEFAULT: {
        break;
      }
  }
  setCurrentSpeed();
}

void processDownButton(button_op press) {
  if (!enabled) return;
  if (low_period >= 65534) {
    showMsg("MIN SPEED");
    return;
  }
  switch (press) {
    case BWAIT: {
        showMsg("DECREASE");
        break;
      }
    case BON: {
        low_period += SPEED_STEP_S;
        showMsg("DECREASE");
        break;
      }
    case BLONG:
    case BHOLD: {
        showMsg("DECREASE++");
        low_period += SPEED_STEP_F;
        break;
      }
    case DEFAULT: {
        break;
      }
  }
  setCurrentSpeed();
}

void processButton(Bounce *b, unsigned long *counter, void (*op)(button_op press), String name) {
  b->update();
  if (b->fell()) {
    (*counter) = 0;
    op(BON);
  } else if (b->rose()) {
    op(BOFF);
  } else if (!b->read()) {
    (*counter)++;
    if (*counter > FAST) {
      op(BLONG);
    } else if (*counter > SLOW) {
      op(BHOLD);
    } else {
      op(BWAIT); // when you hold the button but before hold is triggered
    }
  }
}


void showMsg(const char *message) {
  msgSetTime = millis();
  dispMessage = message;
}

void displayMessage() {
  u8g2.clearBuffer();
  u8g2.setFont(FONT);
  if (millis() > msgSetTime + messageShowTime) {
    // clear message
    if (direction) {
      u8g2.drawStr(4, 30, (enabled ? (ffwd || rwd ? "<<<< RUN CCW" : "< RUN CCW") : "STOPPED"));
    } else {
      u8g2.drawStr(4, 30, (enabled ? (ffwd || rwd ? "RUN CW >>>>" : "RUN CW >") : "STOPPED"));
    }
  } else {
    u8g2.drawStr(4, 30, dispMessage);
  }
  u8g2.sendBuffer();
}
