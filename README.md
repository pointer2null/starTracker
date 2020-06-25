# starTracker

To take a long exposure of the stars you need to comensate for the earths rotation. This is my star tracker.

StarTracker control. When a Nema 17 or equivelent stepper is connected to a compatable controller and the output is geared as shown, then the final output shaft should revolve once per siderial day.
   
Hardware used:
 * Nema 17 stepper with 99.0506:1 planetory gearbox
 * final drive 3:1 toothed belt
 * TB6600 stepper driver.
 * Arduino Uno R3
 * 0.91 INCH OLED Display: Resolution is 128*32
 * 8 x push buttons
 * Siderial day = 86164.1 seconds
 * gearbox 99.0506:1
 * pully 3:1
 
200 steps 16 microsteps each microstep = 0.0906145625seconds = 90.6145625ms
