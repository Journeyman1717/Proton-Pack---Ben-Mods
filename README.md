I have moved the Cyclotron to Pin 10 and created a second object in the NeoPatterns Class caleed CycLEDs, the first Object was PackLEDs.
  CycLEDs are RGB LEDs on Pin 10
  PackLEDs are on Pin 2 and are all RGBW with additional lights in the Booster Tube, HGA and NFilter.

ISSUE
Only the first object in the class is being called. If I the order in which the objects are declaired, the LEDs connected to the first object are functional and the LEDs connected to the second object are not.

Additional Libraries needed:
DFPlayerMini_Fast
FastLED
