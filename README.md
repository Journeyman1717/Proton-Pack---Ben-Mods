In GBLEDPatterns.h find where the cyclotron LED's (40 or 50) are addressed. They would have been the first in the sequencee but now are the last. See notes at the top of the files.

Determine how to mix GRBW and GRB LEDs in the same dataconnection LED's in the cyclotron are GRB and the LED's elsewhere are GRBW.
  Looks like you cant mix RGB and RGBW LED's in the same "string" like i did, I will have to find a new Output pin for the Cyclotron. This may make the above change moot.
  https://forums.adafruit.com/viewtopic.php?t=110803

PIN 10 currently has no connection. I could move the Cyclotron to Pin 10 (50 LEDs)  But we would need to seperate the Power Cell Code from the Cyclotron Code.


Additional Libraries needed:
DFPlayerMini_Fast
FastLED

Optional:
  Add the additional Functions as time permits.
