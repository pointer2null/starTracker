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

#include <Bounce2.h>

enum buttonmode {MODE0, MODE1};

/*
BOFF:   Button is currently up
BPRESS: Button has been pressed and released before the first threshold
BHOLD:  Button has has been held beyond first threshold and released
BLONG:  Button has been held beyond second threshold  
BWAIT:  Button is currently pressed but not released or passed first threshold
*/
enum button_op {BOFF, BPRESS, BHOLD, BLONG, BWAIT};

class GButton
{
  public:
    GButton(Bounce *b, void (*handler)(button_op press), unsigned long firstThresholdMs, unsigned long secondThreasholdMs);
    void process();
  private:
    unsigned long pressStart;
    buttonmode mode;
};

#endif