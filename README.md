Forbis-Stokes Arduino Project
=============================

I have this directory managed as a git repository for version control of the
circuit schematics and Arduino code.  Will have more time to really look at
this in December, but might get to chip away at it sooner.

Problems
--------
What I am trying to do is operate two systems. One system runs a pump using an
input for a level sensor. If below the minimum, the pump remains off. If above
the maximum, the pump remains on. If in between, the pump runs on a timer. The
other system controls the temperature in a heating tank using temperature
probes. If the temperature is below the minimum, a gas valve should open and an
ignitor should go off on a timer. One the temperature goes above the maximum,
the valve should close and ignitor stop.

Here is a list of issues I am facing:

* I can only use 3 thermistors as analog inputs instead of the 5 I want. I am
  not sure why.
* The analog reading of the level sensor (output is variable current that I run
  through a resistor for voltage reading) is not being read by the Arduino,
  though it can be read by a multimeter.
* Running the pump and ignitor on separate timers that will not conflict has
  been challenging.
* Enough current flowing through some of the digital pins to switch my relay
  even when the setting is LOW.

Some notes from Aaron on the code
---------------------------------

Attached are two sets of Arduino code. The "ADPL_labsetup" is the more complete
file. This was made using the level sensor as a digital output, only sending a
signal when the minimum or maximum levels were exceeded. The "Pump_analog" was
created to test the level sensor output when used as an analog output ranging
from 4-20 mA, depending on depth.

To explain what I am trying to do (as simply as possible):

* Operate pump - When the level in a collection tank is between a minimum and
  maximum level, the pump should be on for 3 minutes every hour. When the level
  is below minimum, pump should be off. When the level is above maximum, the
  pump should remain on. If I can run the level sensor on analog output, I want
  to record the readings to know the depth.

* Record temperatures at 5 points.

* Operate burner - If temperature 3 is below 68C, a gas valve should open and
  an ignitor should spark for 5 seconds. Until the temperature reaches 72C, the
  valve should remain open, and the ignitor should spark for 5 seconds, turn
  off for 15 minutes, and spark again for 5 seconds. Once the temperature is
  72C, the valve should shut and ignitor remain off.
