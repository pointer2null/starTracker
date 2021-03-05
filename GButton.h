/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

   (c) 2020 Gary Butler grbutler@gmail.com

    Provides a class that handles buttons - debounce, press, hold, log hold etc

*/
#ifndef GButton_h
#define GButton_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <inttypes.h>

enum buttonmode
{
  MODE0, // normal
  MODE1  // extended function
};

class GButton
{
public:
  GButton(int pin, int pinmode, void (*handler)(uint8_t button_flags), uint16_t firstThresholdMs, uint16_t secondThreasholdMs);
  GButton(int pin, int pinmode, void (*handler)(uint8_t button_flags));
  GButton(int pin, int pinmode);
  void process();

private:
  /* Button flags
BOFF:    Button is currently off
BWAIT:   Button is currently pressed 
BCLICK:  Button has been pressed and released 
BHOLD:   Button has has been held beyond first threshold 
BLONG:   Button has been held beyond second threshold  
BDOUBLE: Button double click
*/
  static const uint8_t BOFF = 0b00000001;
  static const uint8_t BWAIT = 0b00000010;
  static const uint8_t BPRESS = 0b00000100;
  static const uint8_t BHOLD = 0b00000100;
  static const uint8_t BLONG = 0b00001000;
  uint16_t pressStart;
  uint16_t firstThresholdMs;
  uint16_t secondThreasholdMs;
  buttonmode mode;
  int attachedPin;
  int pinMode;
  void *callbackHandler;
};

#endif