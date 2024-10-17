/****************************************************************************************
   Michael Simone - Arduino Source for 2020 Ghostbusters Proton Pack
   This is a modified Adafruit library to handle the routine operations for both the Cyclotron and Powercell.
   The following are the pattern types supported:


*/
enum  pattern { NONE, POWERCELL, POWERCELL_BOOT, CYCLOTRON, CYCLOTRON_BOOT, POWERDOWNPACK, VENT };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
  public:

    // *** Pattern variables for both the Cyclotron and Powercell
    pattern  ActivePatternCyc;     // which Cycltron pattern is running // ActivePattern1 changed to ActivePatternCyc
    pattern  ActivePatternPC;      // which Powercell pattern is running // ActivePattern2 changed to ActivePatternPC
    pattern  ActivePatternHGA;     // which HGA pattern is running // new
    pattern  ActivePatternBT;      // which Booster Tube pattern is running // new
    pattern  ActivePatternNF;      // which N-Filter pattern is running // new
 
    direction Direction;            // direction to run the pattern

    /*
       The following are interval variables to manage timing and the change of timing during firing
    */

    unsigned long Interval;         // Cyclotron milliseconds between updates (These are set in the main program)
    unsigned long IntervalChange;
    unsigned long Interval2;        // Cyclotron milliseconds between updates (These are set in the main program)
    unsigned long lastUpdateCyc;    // last update of position // laterUpdateC changed to lastUpdateCyc
    unsigned long lastUpdatePC;     // last update of position
    unsigned long lastUpdateHGA;    // last update of position // new
    unsigned long lastUpdateBT;     // last update of position // new
    unsigned long lastUpdateNF;     // last update of position // new

    uint8_t themeMode;

    // Added these variables to handle the change in fading speeds when firing
    bool fadeComplete = false;
    bool IntervalChangeFlag = false;
    bool AfterLifeFiring = false;
    uint8_t mytime;

    /*
       The following provide an index position of the LED's to determine which one you want to change in the loop sequence
    */

    uint16_t IndexCyc;              // current step within the pattern (Cyclotron)
    uint8_t  IndexPC;               // current step within the pattern (PowerCell) (0 - 15)
    uint8_t  IndexHGA;              // current step within the pattern (HGA) // new
    uint8_t  IndexBT;               // current step within the pattern (Booster Tube) // new
    uint8_t  IndexNF;               // current step within the pattern (N-Filter) // new

    uint16_t TotalStepsCyc;         // total number of steps in the Cyclotron pattern
    uint16_t TotalStepsPC;          // total number of steps in the PowerCell pattern (16 Steps) start (LED 23) end (LED 38)
    uint16_t TotalStepsHGA;         // total number of steps in the HGA pattern
    uint16_t TotalStepsBT;          // total number of steps in the Booster Tube pattern
    uint16_t TotalStepsNF;          // total number of steps in the N-Filter pattern


    uint32_t Color1, Color2;        // What colors are in use
    int fadeStep = 0;               // state variable for fade function
    uint16_t PCSegIndex;            //

    /*
       Cyclotron & Powercell references
    */

    //const uint8_t CyclotronLEDs = 50; // Changed for Ben's Pack (0-49) Pin 10

    const uint8_t HGALEDs = 7; // Added for Ben's Pack (0 - 6) // new
    const uint8_t HGAStart = 0; // added
    const uint8_t HGAEnd = 6; // added
    const uint8_t BoosterLEDs = 16; // Added for Ben's Pack (7 - 22) // new
    const uint8_t BoosterStart = 7; // added
    const uint8_t BoosterEnd = 23; // added
    const uint8_t PowerCellLEDs = 16; // CONFIRMED
    const uint8_t PowerCellStart = 24; // added CONFIRMED
    const uint8_t PowerCellEnd = 39;   // added CONFIRMED
    const uint8_t NFilterLEDs = 9; // Added for Ben's Pack (39 - 47) // new
    const uint8_t NFilterStart = 40; // added
    const uint8_t NFilterEnd = 48; // added
    const uint8_t CycLEDs = 50;  // added
    const uint8_t CycStart = 0;  // added
    const uint8_t CycEnd = 49;  // added

    
    uint16_t IndexCycLEDArray;
    //const uint8_t   CycLEDs = 50; // Changed for Ben's Pack
    const uint8_t  CyclotronLEDCount = 50; // Changed for Ben's Pack
    const int CyclotronLEDArray[50] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    

    /*
        CYCLOTRON LED Pattern (Note: LED reference will still be 0-19, timing will reference 31 spots)
        0 - 4 LED (Lower right Cyclotron Lens)
        5 BLANK (No LED's)
        6-10 LED  (Lower left Cyclotron Lens)
        11-13 BLANK (No LED's)
        14 - 18 LED (Upper left Cyclotron Lens)
        19 - 22 BLANK (No LED's)
        23 - 27 LED (Upper right Cyclotron Lens)
        28-30 BLANK (No LED's)
    */

    bool cycNormalStart = false;      // Used on the first routine to clear all LEDs before you start the sequence

    /*
       Call back functions to let you know when a routine is complete E.g. let you know when the pack is powered up or down.
    */

    void (*OnCompleteC)();  // Callback on completion of pattern
    void (*OnCompletePC)();  // Callback on completion of pattern

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns (uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
      : Adafruit_NeoPixel(pixels, pin, type)
    {
      OnCompleteC = callback;
      OnCompletePC = callback;
    }

    /*
        Main Update that gets called to refresh all your LED's, this checks the routine
        and calls the appropriate updates.
    */
    void Update()
    {
      //CyclotronUpdate();
      
      if ((millis() - lastUpdateCyc) > Interval) // Time to update Cyclotron ?
      {
        lastUpdateCyc = millis();
        switch (ActivePatternCyc)
        {
          case CYCLOTRON:
            CyclotronUpdate();
            break;
          case CYCLOTRON_BOOT:
            CyclotronBootUpdate();
            break;
          case VENT:
            break;
          case POWERDOWNPACK:
            break;
          default:
            break;
        }
      }

      if ((millis() - lastUpdatePC) > Interval2) // Time to update Cyclotron PowerCell?
      {
        lastUpdatePC = millis();
        switch (ActivePatternPC)
        {
          case POWERCELL:
            PowercellUpdate();
            break;
          case POWERCELL_BOOT:
            PowercellBootUpdate();
            break;
          case POWERDOWNPACK:
            PowerDownUpdate();
            break;
          case VENT:
            VentPackUpdate();
          default:
            break;
        }
      }
    }

    /*
       Increment the cyclotron state (4 states, called after every interval)
    */

    void IncrementC()
    {
      if (ActivePatternCyc == CYCLOTRON)
      {
        if (CyclotronLEDArray[IndexCyc] == 1)
        {
          IndexCycLEDArray++;
        }
        IndexCyc++;
        if (IndexCyc >= TotalStepsCyc)
        {
          IndexCyc = 0;
          IndexCycLEDArray = 0;
          if (themeMode == 4)
          {
            if (Color1 == Wheel(255))
            {
              Color1 = Wheel(85);
            }
            else
            {
              Color1 = Wheel(255);
            }
          }
          if (OnCompleteC != NULL)
          {

            OnCompleteC(); // call the completion callback
          }
        }
      }
    }

    /*
       Increment the Powercell LED state
    */
    void IncrementPC()
    {
      if (ActivePatternPC != POWERDOWNPACK)      // *** dont continue if in the powerdown state *** //
      {
        if (Direction == FORWARD)
        {
          IndexPC++;
          if (IndexPC >= TotalStepsPC) // Changed PowerCellLEDs from TotalStepsPC
          {
            IndexPC = PowerCellStart - 1; // Chaged to be PowerCellStart - 1 previously Cyclotron End
            if (OnCompletePC != NULL)
            {
              OnCompletePC(); // call the completion callback
            }
          }
        }
        else // Direction == REVERSE
        {
          --IndexPC;
          if (ActivePatternPC == POWERCELL_BOOT)
          {
            if (IndexPC - PCSegIndex < PowerCellStart - 1)  // Changed to be the start of the PowerCell - 1
            {
              IndexPC = PowerCellEnd - 1;               // Changed to be the PowerCell End Does this need to be -1?
              PCSegIndex++;                             // -1 to account for the zero position
              if (PCSegIndex == 15)
              {
                if (OnCompletePC != NULL)
                {
                  OnCompletePC(); // call the completion callback
                }
              }
            }
          }

          /*
             This should not be called, Reverse Powercell is used during boot, forward in the normal state. This could be cleaned up
          */
          if (IndexPC <= 0)
          {
            IndexPC = PowerCellEnd - 1;  // Changed to be PowerCellEnd
            if (OnCompletePC != NULL)
            {
              ActivePatternCyc = NONE;
              ActivePatternPC = NONE;
              OnCompletePC(); // call the completion callback
            }
          }
        }
      }
      else
      {
        /*
           This would be normally called during a power down but the segment system is not used
        */
        IndexPC++;
        if (IndexPC >= PowerCellEnd) // Changed to be PowerCellEnd
        {
          IndexPC = 0;
          if (OnCompletePC != NULL)
          {
            OnCompletePC(); // call the completion callback
          }
        }
      }
    }

    /*
        Initialize for the Powercell Boot
    */
    void PowercellBoot(uint32_t color1, uint32_t interval, direction dir = REVERSE)
    {
      // ** Reverse direction, reset the powercell index variables ** //
      ActivePatternPC = POWERCELL_BOOT;
      Interval2 = interval;
      PCSegIndex = 0;
      TotalStepsPC = PowerCellEnd; // This should be the end LED (38)
      Color2 = color1;
      IndexPC = TotalStepsPC;
      Direction = dir;
    }

    /*
         Initialize for the Powercell Normal State
    */
    void Powercell(uint32_t color1, uint32_t interval, direction dir = FORWARD)
    {
      // ** Forward direction, reset the powercell index variables ** //
      ActivePatternPC = POWERCELL;
      Interval2 = interval;
      TotalStepsPC = PowerCellEnd; // This should be the end LED (38)
      Color2 = color1;
      IndexPC = PowerCellStart - 1; // Changed to be reference to itsself.
      Direction = dir;
    }

    /*
        Initialize Cyclotron Boot Cycle
    */

    void CyclotronBoot(uint32_t color1, uint32_t interval, uint8_t tmode)
    {
      // ** reset the cyclotron index variables ** //
      ActivePatternCyc = CYCLOTRON_BOOT;
      themeMode = tmode;
      Interval = interval;
      TotalStepsCyc = 4;
      Color1 = color1;
      IndexCyc = 0;
      fadeStep = 0;
    }

    /*
       Initialize Normal Cyclotron Cycle
    */

    void Cyclotron(uint32_t color1, uint32_t interval, uint8_t tmode)  //enum PackTheme { MOVIE, STATIS, SLIME, MESON, CHRISTMAS };
    {
      // ** reset the cyclotron index variables ** //
      themeMode = tmode;
      ActivePatternCyc = CYCLOTRON;
      TotalStepsCyc = CycLEDs;
      if (themeMode == 1)
      {
        Interval = 5;
      }
      else
      {
        Interval = interval;
      }


      fadeStep = 0;
      cycNormalStart = false;           // has the routine started yet? ( used to clear LED's and start the cycle again )
      fadeComplete = false;

      /*
        Changes the cyclotron to the THEME colors
      */
  
      switch (themeMode)
      {
        case 0:
          Color1 = Wheel(255);
          break;
        case 1:
          Color1 = Wheel(255);
          break;
        case 2:
          Color1 = Wheel(85);
          for (uint8_t i = 0; i < CyclotronLEDCount; i++)
          {
            setPixelColor(i, Color1);
          }
          break;
        case 3:
          Color1 = Wheel(42);
          break;
        case 4:
          Color1 = Wheel(85);
          break;
      }
    }

    /*
      Initialize Venting
    */
    void VentPack()
    {
      ActivePatternCyc = VENT;
      ActivePatternPC = VENT;
    }

    /*
      Initialize Powerdown Sequence
    */
    void PowerDown(uint8_t interval)
    {
      ActivePatternCyc = POWERDOWNPACK;
      Interval = interval;
      TotalStepsCyc = CycLEDs;
      IndexCyc = 0;
      ActivePatternPC = POWERDOWNPACK;
      Interval2 = 100;
      TotalStepsPC = 35;
      IndexPC = 0;
      Direction = REVERSE;
    }

    /*
      Update the PowerCell LEDs using the Boot Sequence
    */
    void PowercellBootUpdate()
    {
      for (uint8_t i = PowerCellStart - 1; i < PowerCellEnd; i++)
      {
        if (i == IndexPC)  // Scan Pixel to the right
        {
          setPixelColor(i, Color2);
        }
        else
        {
          for (uint8_t j = PowerCellStart - 1 + PCSegIndex; j < PowerCellEnd; j++)
          {
            if (j != IndexPC )
            {
              setPixelColor(j, Color(0, 0, 0));
            }
          }
        }
      }
      show();
      IncrementPC();
    }

    /*
      Update the PowerCell using the Normal Sequence
    */
    void PowercellUpdate()
    {
      for (uint8_t i = PowerCellStart - 1; i < PowerCellEnd; i++)
      {

        if (i == IndexPC)  // Scan Pixel to the right
        {
          setPixelColor(i, Color2);
        }
        if (IndexPC == (PowerCellEnd - 1)) // removed -1
        {
          for (uint8_t j = PowerCellStart - 1; j < PowerCellEnd; j++)
          {
            setPixelColor(j, Color(0, 0, 0));
          }
        }

      }
      show();
      IncrementPC();
    }

    /*
      Update the Cyclotron using the Boot Sequence
    */
    

    void CyclotronBootUpdate()
    {
      // ** Fade in the correct THEME color ** //
      switch (themeMode)
      {
        case 0:
          fade(0, 255, 0, 0, 0, 0, 200, 0, CyclotronLEDCount - 1);
          break;
        case 1:
          fade(0, 255, 0, 0, 0, 0, 200, 0, CyclotronLEDCount - 1);
          break;
        case 2:
          fade(0, 0, 0, 255, 0, 0, 200, 0, CyclotronLEDCount - 1);
          break;
        case 3:
          fade(0, 129, 0, 126, 0, 0, 200, 0, CyclotronLEDCount - 1);
          break;
        case 4:
          fade(0, 0, 0, 255, 0, 0, 200, 0, CyclotronLEDCount - 1);
          break;
      }
      show();
      IncrementC();
    }

    /*
      Update the Cyclotron using the Normal Sequence
    */

    void CyclotronUpdate()
    {
      if (themeMode == 1)
      {
        circring();
      }
      else
      {
        for (uint8_t i = 0; i < CycLEDs; i++)
        {
          //Serial.print(i);
          if (i == IndexCyc)  // Scan Pixel to the right
          {
            if (CyclotronLEDArray[IndexCyc] == 1)
            {
              //Serial.println(IndexCycLEDArray);
              if (themeMode == 2)
              {
                //Serial.println(IndexCycLEDArray);
                setPixelColor(IndexCycLEDArray, DimColor(getPixelColor(IndexCycLEDArray)));
                setPixelColor(IndexCycLEDArray, DimColor(getPixelColor(IndexCycLEDArray)));
              }
              else
              {
                setPixelColor(IndexCycLEDArray, Color1);
              }
            }
          }
          else // Fading tail
          {
            if (CyclotronLEDArray[i] == 1)
            {
              if (i < CyclotronLEDCount)
              {
                if (themeMode != 4)
                {
                  if (themeMode == 2)
                  {
                    if (IndexCycLEDArray != i)
                    {
                      setPixelColor(IndexCycLEDArray - 1, Color1);
                    }

                  }
                }
              }
            }
            if (i < CyclotronLEDCount)
            {
              if (themeMode != 4 && themeMode != 2)
              {
                {
                  setPixelColor(i, DimColor(getPixelColor(i)));
                }
              }
            }
          }
        }
        IncrementC();
      }
      show();
    }

    /*
       Venting Pack LED Update Function
    */
    void VentPackUpdate()
    {
      for (uint8_t i = 0; i < CycLEDs; i++)
      {
        setPixelColor(i, Color1);
      }
      for (uint8_t i = PowerCellStart - 1; i < PowerCellEnd; i++)
      {
        setPixelColor(i, Color2);
      }
      show();
    }

    /*
       Power down LED Update Function
    */
    void PowerDownUpdate()
    {
      for (uint8_t i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, DimColor(getPixelColor(i)));
      }
      show();
      IncrementPC();
    }

    /*
       Clear Powercell Update Function
    */
    void PowercellClear()
    {
      for (uint8_t i = PowerCellStart - 1; i < PowerCellEnd; i++)
      {
        setPixelColor(i, Color(0, 0, 0));
      }
      show();
    }

    /*
       Change the time interval of the powercell during firing
    */
    void PowercellInterval(uint32_t interval)
    {
      Interval2 = interval;
    }

    
    // Reverse pattern direction
    void Reverse()
    {
      if (Direction == FORWARD)
      {
        Direction = REVERSE;
        IndexCyc = TotalStepsCyc - 1;
      }
      else
      {
        Direction = FORWARD;
        IndexCyc = 0;
      }
    }

    /*
       Change the time interval of the cyclotron during firing
    */

    void CyclotronInterval(uint8_t interval)
    {
      if (themeMode == 1)
      {
        setBrightness(240);
      }
      else
      {
        Interval = interval;
      }
    }

    /*
       Change the cyclotron intervals
    */
    void IntervalChangeCall()
    {
      Interval = IntervalChange;
      //FadeCyc = FadeCycChange;
    }

    /*
       The following are support functions from Adafruit to help with setting colors, diming.
    */
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
      // Shift R, G and B components one bit to the right
      uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
      return dimColor;
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
      return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
      return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
      return color & 0xFF;
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85)
      {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
      }
      else if (WheelPos < 170)
      {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      else
      {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
      }
    }

    /*
       This is a fading function that is used to help with the cyclotron fading and power up.

       Its sligntly modified to help fade only selected LED's that are called.
    */
    void fade(byte redStartValue, byte redEndValue, byte greenStartValue, byte greenEndValue, byte blueStartValue, byte blueEndValue, int totalSteps, int startLED, int endLED)
    {
      static float redIncrement, greenIncrement, blueIncrement;
      static float red, green, blue;
      static bool fadeUp = false;

      if (fadeStep == 0) // first step is to initialise the initial colour and increments
      {
        red = redStartValue;
        green = greenStartValue;
        blue = blueStartValue;
        fadeUp = false;

        redIncrement = (float)(redEndValue - redStartValue) / (float)totalSteps;
        greenIncrement = (float)(greenEndValue - greenStartValue) / (float)totalSteps;
        blueIncrement = (float)(blueEndValue - blueStartValue) / (float)totalSteps;
        fadeStep = 1; // next time the function is called start the fade

      }
      else  // all other steps make a new colour and display it
      {
        // make new colour
        red += redIncrement;
        green +=  greenIncrement;
        blue += blueIncrement;

        // set up the pixel buffer

        for (byte i = startLED; i <= endLED; i++)
        {
          //Serial.println((byte)red);
          setPixelColor(i, Color((byte)red, (byte)green, (byte)blue));
        }

        // go on to next step
        fadeStep += 1;

        // finished fade
        if (fadeStep >= totalSteps)
        {
          fadeComplete = true;
          if ((byte)red > 0 && (byte)red < 50)
          {
            for (byte i = startLED; i <= endLED; i++)
            {
              //Serial.println((byte)red);
              setPixelColor(i, 0);
            }
          }
          if ((byte)green > 0 && (byte)green < 50)
          {
            for (byte i = startLED; i <= endLED; i++)
            {
              //Serial.println((byte)red);
              setPixelColor(i, 0);
            }
          }
          if ((byte)blue > 0 && (byte)blue < 50)
          {
            for (byte i = startLED; i <= endLED; i++)
            {
              //Serial.println((byte)red);
              setPixelColor(i, 0);
            }
          }
          if (fadeUp) // finished fade up and back
          {
            fadeStep = 0;

            return; // so next call recalabrates the increments
          }

          // now fade back
          fadeUp = true;
          redIncrement = -redIncrement;
          greenIncrement = -greenIncrement;
          blueIncrement = -blueIncrement;
          fadeStep = 1; // don't calculate the increments again but start at first change
        }
      }
    }

    // Quick routine to change the speed of the cyclotron for the Afterlife Mode.
    void AL_Fire(bool IsFire)
    {
      if (IsFire == true) {
        AfterLifeFiring = true;
      }
      else
      {
        AfterLifeFiring = false;
      }
    }


    void circring()
    {
      for (uint8_t i = 0; i < CycLEDs; i++) {  // We can't do a fill_solid as we need to use a modulus operator to wrap around the strand.
        setPixelColor(i, Color(0, 0, 0));
      }
      if (AfterLifeFiring == false)
      {
         mytime = (millis() / 3) % CycLEDs;
      }
      else
      {
        mytime = (millis() / 2) % CycLEDs;
      }
      uint8_t mylen = beatsin8(15, 3, CycLEDs / 4);

      for (uint8_t i = 0; i < mylen; i++) {  // We can't do a fill_solid as we need to use a modulus operator to wrap around the strand.
        setPixelColor(((mytime + i) % CycLEDs), Color(255, 0, 0));
      }
    } // circring()
};
