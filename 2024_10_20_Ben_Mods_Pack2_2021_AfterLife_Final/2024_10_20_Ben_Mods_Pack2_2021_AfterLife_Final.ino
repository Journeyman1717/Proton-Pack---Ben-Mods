/****************************************************************************************
   Michael Simone - Arduino Source for 2021 Ghostbusters Proton Pack
   LED pinout 2 / 3 using addressable LED's (NewPixels) and 1 Adafruit Jewel for the wand gun
   Pinout 2 - Cyclotron LEDs 0 - 39, Powercell, 40 - 54
   Pintout 3 - Cyclotron Vent 0 - 3, 4 Slow-Blow, 5 Wand Vent, , 6 White LED, 7 White LED2, 8, Yellow LED, , 9 Orange Hat LED, 10-16 NeoPixel Jewel
   Sound Module - DFPlayer
   Some Reference Code from Eric Banker (Wand LED write helper function)

   2021 Changes - Repurposed STATIS Game mode for Afterlife, added 40 LED ring and new cyclotron routine for Afterlife
******************************************************************************************/

/*********************************************************
    Initiate Libraries for the 28 segment bargraph
 *********************************************************/

#include <BGSequence.h>
BGSequence BarGraph;

// **** Different Bargraph sequence modes **** //
enum BarGraphSequences { START, ACTIVE, FIRE1, FIRE2, BGVENT };
BarGraphSequences BGMODE;

/*********************************************************
    Initiate Libraries for DFPlayer sound module
 *********************************************************/

#include "SoftwareSerial.h"
#include <DFPlayerMini_Fast.h>

SoftwareSerial mySoftwareSerial(12, 11);  // RX, TX (was 11, 12)
DFPlayerMini_Fast myDFPlayer;

// Audio Tracks - Note DFPlayer is sensitive to file order
unsigned long StartupTrack = 1;
unsigned long AfterlifeStartup = 37; // Added for afterlife startup track.
unsigned long AfterlifeStartupShort = 38; // Added for Afterlidfe statup after venting
unsigned long ThemeSWTrack = 3;
unsigned long SlimeSWTrack = 32;
unsigned long MesonSWTrack = 31;
unsigned long ChristmasSWTrack = 26;

unsigned long FireMovie = 11;
unsigned long FireCrossStreams = 12;
unsigned long FireStasis = 13; // Not referenced, has become FireAfterlife
unsigned long FireSlime = 14;
unsigned long FireMeson = 15;
unsigned long FireChristmas = 21;
unsigned long FireAfterlife = 34;

unsigned long MovieTail = 4;
unsigned long StasisTail = 22; // Not referenced, new reference for AfterlifeTail
unsigned long SlimeTail = 23;
unsigned long MesonTail = 24;
unsigned long ChristmasTail = 25;
unsigned long AfterlifeTail = 36;

unsigned long shutdownTrack = 10;
unsigned long IdleMovieStart = 5; // Not referenced
unsigned long IdleMovieLoop = 2;
unsigned long IdleSlimeStart = 27;
unsigned long IdleSlimeLoop = 28;
unsigned long IdleMesonStart = 29;
unsigned long IdleMesonLoop = 30;
unsigned long IdleAfterlifeLoop = 35;

unsigned long clickTrack = 8;
unsigned long chargeTrack = 7;
unsigned long ventTrack = 9;
unsigned long SlimeRefillTrack = 33;

unsigned long warnMovie = 16;
unsigned long warnStasis = 17; // Not referenced
unsigned long warnSlime = 18;
unsigned long warnMeson = 19;
unsigned long warnCrossStreams = 20;
unsigned long warnAfterlife = 16; // Same file as WarnMovie


unsigned long currenttrack = 1;

/*************************************************************************
    Initiate LED Variables
*************************************************************************/
int seq_1_current = 0;               // current led in sequence 1
const int num_led = 28;              // total number of leds in bar graph
unsigned long firing_interval = 40;  // interval at which to cycle firing lights on the bargraph. We update this in the loop to speed up the animation so must be declared here (milliseconds).

// Ben's Edits Wand LED's
const int WhiteLED1 = 0;
const int ThemeLED = 1;
const int WandVentStart = 2;  // New
const int WandVentEnd = 8;    // New
const int WandVentLEDs = 7;   // Number of LED's on Wand Vent
const int OrangeHatLED = 9;
const int WhiteLED2 = 10;
const int SloBloLED = 11;
const int GunLEDStart = 12;
const int GunLEDEnd = 18;

/*
const int ventStart = 0;
const int ventEnd = 3;
const int SloBloLED = 4;
const int WandVentLED = 5;
const int ThemeLED = 6;
const int WhiteLED1 = 7;
const int WhiteLED2 = 8;
const int OrangeHatLED = 9;
const int GunLEDStart = 10;
const int GunLEDEnd = 16;
*/
/******************************************************************************
    Initiate Adafruit LED NeoPixel libraries and custom GBLEDPattern Class
 ******************************************************************************/
#include "FastLED.h"
#include <Adafruit_NeoPixel.h>
#include "GBLEDPatterns.h"

// *** Note these constants may change if you are using different LED counts, E.g. 1 LED vs a 7 LED Neopixel Jewel *** //

// Cyclotron + PowerCell LED Count
const int NeoPixelCountPack = 48;

// Wand LED Count
const int NeoPixelCountWand = 19;

// Cyclotron LED Count
const int NeoPixelCountCyc = 50;


void PackLEDsComplete();

#define NEO_WAND 3  // for powercell
Adafruit_NeoPixel wandLights = Adafruit_NeoPixel(NeoPixelCountWand, NEO_WAND, NEO_GRBW + NEO_KHZ800);


// Call Cyclotron Patterns Class
NeoPatterns PackLEDs(NeoPixelCountPack, 2, NEO_GRBW + NEO_KHZ800, &PackLEDsComplete);

// Call Cyclotron Patterns Class
NeoPatterns CycLEDs(NeoPixelCountCyc, 10, NEO_GRB + NEO_KHZ800, &PackLEDsComplete);






// ******************* Pack Status / State Arrays ******************* //
enum PackState { OFF, BOOTING, BOOTED, POWERDOWN };
enum PackState STATUS;

enum WandState { WANDOFF, ON, ON_PACK };
enum WandState WANDSTATUS;

enum WandSLEDState { ALLOFF, WANDONLY, NORMAL, FIRING, WARNING, FASTWARNING, VENTSTATE, STREAMCROSS };
enum WandSLEDState WANDLEDSTATUS;

enum PackTheme { MOVIE, STATIS, SLIME, MESON, CHRISTMAS };
enum PackTheme THEME;

// ******************* inputs for switches and buttons ******************* //
/*
const int STARTWAND_SWITCH = 5;
const int STARTPACK_SWITCH = 8;
const int MUSIC_SWITCH = 7;
const int FIRE_BUTTON = 4;
const int FIRE_BUTTON2 = 6;
const int VENTING = 9;  // Output
const int ISPLAYING_OUT = 12;
const int RUMBLE = 13;  // Output
*/
const int STARTWAND_SWITCH = 5;  // Updated for Ben's Pack - Swapped Input with 6
const int STARTPACK_SWITCH = 8;  // Updated for Ben's Pack - Swapped Input with 7
const int MUSIC_SWITCH = 7;      // Updated for Ben's Pack - Swapped Input with 8
const int FIRE_BUTTON = 4;
const int FIRE_BUTTON2 = 6;    // Updated for Ben's Pack - Swapped Input with 5
const int VENTING = 9;         // Output
const int ISPLAYING_OUT = 13;  // Changed from 12 to 13
//const int RUMBLE = 13; // Output // Rumble not used


// ******************* Pack states or logic variables ******************* //
bool PowerOff = true;
bool PowerBooted = false;
bool IsFiring = false;
bool shouldWarn = false;
bool shuttingDown = false;
bool warning = false;
bool venting = false;
bool shouldVent = false;
bool IsPlaying = false;

// ******************* physical switch states ******************* //
bool startpack = false;
bool startwand = false;
bool packidle = false;
bool musicmode = false;
bool musicmodestart = false;
bool fire = false;
bool fireButtonState = false;
bool fire2 = false;
bool fire2ButtonState = false;

// ******************* Cyclotron / Powercell timing variables ******************* //
unsigned long prevCycMillis = 0;
unsigned long prevBGMillis = 0;
unsigned long fire_interval = 100;
unsigned long fire_intervalC = 100;
unsigned long FireRate = 0;
unsigned long FireRate2 = 0;

unsigned long cycIdleRate[5] = { 95, 5, 95, 95, 50 };

unsigned long firingStateMillis;
unsigned long VentMillis;

const unsigned long firingwarning = 5000;
const unsigned long firingalarm = 10000;
const unsigned long venting_interval = 5000;

// **** test variables *** //

unsigned long prevDFPMillis = 0;
unsigned long IntervalDFPMillis = 20;
bool DFPlayerStart = false;
unsigned int DFPTrack = 0;
unsigned int playType = 0;


/*************************************************************************
                                 SETUP Function
 *************************************************************************/
void setup() {
  Serial.begin(9600);
  // ***** Start Communication with the DFPlayer ***** //
  mySoftwareSerial.begin(9600);
  myDFPlayer.begin(mySoftwareSerial);
  myDFPlayer.normalMode();
  myDFPlayer.startDAC();

  BarGraph.BGSeq();

  myDFPlayer.volume(30);

  // ***** Assign Proton Pack Switches / Buttons ***** //
  pinMode(STARTWAND_SWITCH, INPUT);
  digitalWrite(STARTWAND_SWITCH, HIGH);
  pinMode(STARTPACK_SWITCH, INPUT);
  digitalWrite(STARTPACK_SWITCH, HIGH);
  pinMode(MUSIC_SWITCH, INPUT);
  digitalWrite(MUSIC_SWITCH, HIGH);
  pinMode(FIRE_BUTTON, INPUT);
  digitalWrite(FIRE_BUTTON, HIGH); // Using Pull Down Resistors
  pinMode(FIRE_BUTTON2, INPUT);
  digitalWrite(FIRE_BUTTON2, HIGH); // Using Pull Down Resistors
  pinMode(VENTING, OUTPUT);
  digitalWrite(VENTING, HIGH);
  //pinMode(RUMBLE, OUTPUT);
  //digitalWrite(RUMBLE, LOW);

  pinMode(ISPLAYING_OUT, INPUT);

  // ***** Configure LED's in wand lights (Including Vent LEDs) ***** //
  wandLights.begin();
  wandLights.setBrightness(100);
  wandLights.show();

  // ***** Configure Proton Pack LED's ***** //
  PackLEDs.begin();
  //PackLEDs.Color1 = PackLEDs.Wheel(255);
  PackLEDs.Color2 = PackLEDs.Wheel(170);

  CycLEDs.begin();
  CycLEDs.Color1 = PackLEDs.Wheel(255);
  //CycLEDs.Color2 = PackLEDs.Wheel(170);

  // ***** Set the intial pack status to off ***** //
  STATUS = OFF;
  THEME = STATIS;
  // ***** Start with LED's off ***** //
  clearLEDs();
  delay(500);
}


bool firestart = false;
unsigned long CuurentBGInterval = BarGraph.IntervalBG;
/*************************************************************************
                                 Main Loop Function
 *************************************************************************/
void loop() {

  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();

  // Check Wand Switch Status
  startwand = digitalRead(STARTWAND_SWITCH);
  startpack = digitalRead(STARTPACK_SWITCH);
  musicmode = digitalRead(MUSIC_SWITCH);
  fire = digitalRead(FIRE_BUTTON); 
  fire2 = digitalRead(FIRE_BUTTON2); 
  IsPlaying = digitalRead(ISPLAYING_OUT);
  //Serial.print(startwand) ;
  //Serial.println(IndexPC);
  getWandSTATUS();

  /*
     Switch logic
     Activate Pack - Switch 1
     Active Wand - Switch 2 (Turn Vent On)
     Music - Switch 3 Block Fire Logic, Fire changes song (Pack Must be active)
     Fire 1 / 2 are the same
  */

  /********************************************************************************
      Play / Change music (Fire Button 1) or Change Theme (Fire Button 2)
   ********************************************************************************/
  if (musicmode == true) {
    if (fire2 == true && fire2ButtonState == true) { // Inverted fire2 and fire2ButtonState states false was true and true was false
      if (STATUS == BOOTED) {
        switch (THEME) {
          case MOVIE:
            THEME = STATIS;
            CycLEDs.setBrightness(75);
            CycLEDs.Cyclotron(CycLEDs.Color1, cycIdleRate[1], THEME);
            myDFPlayer.play(ThemeSWTrack);
            break;
          case STATIS:
            THEME = SLIME;
            CycLEDs.setBrightness(75);
            CycLEDs.Cyclotron(CycLEDs.Color1, cycIdleRate[2], THEME);
            myDFPlayer.play(SlimeSWTrack);
            break;
          case SLIME:
            THEME = MESON;
            CycLEDs.Cyclotron(CycLEDs.Color1, cycIdleRate[3], THEME);
            myDFPlayer.play(MesonSWTrack);
            break;
          case MESON:
            THEME = CHRISTMAS;
            CycLEDs.Cyclotron(CycLEDs.Color1, cycIdleRate[4], THEME);
            myDFPlayer.play(ChristmasSWTrack);
            break;
          case CHRISTMAS:
            THEME = MOVIE;
            CycLEDs.Cyclotron(CycLEDs.Color1, cycIdleRate[0], THEME);
            myDFPlayer.play(ThemeSWTrack);
            break;
        }
      }
      fire2ButtonState = false; // Inverted fire2 and fire2ButtonState states false was true and true was false
      musicmodestart = true; // Inverted fire2 and fire2ButtonState states false was true and true was false
    } else {
      if (fire2 == false) { // Inverted fire2 and fire2ButtonState states false was true and true was false
        fire2ButtonState = true; // Inverted fire2 and fire2ButtonState states false was true and true was false
      }
    }
    if (fire == true && fireButtonState == true) { // Inverted fire and fireButtonState states false was true and true was false
      fireButtonState = false; // Inverted fire and fireButtonState states false was true and true was false
      if (musicmodestart == false) {
        myDFPlayer.playFolder(1, 1);
        musicmodestart = true;
      } else {
        myDFPlayer.playNext();
      }
    } else {
      if (fire == false) { // Inverted fire and fireButtonState states false was true and true was false
        fireButtonState = true; // Inverted fire and fireButtonState states false was true and true was false
      }
    }
  } else {
    musicmodestart = false;
  }
  /********************************************************************************
      Logic routine, Checks for the wand status to activate the pack and wand features
   ********************************************************************************/
  if (WANDSTATUS == ON_PACK)  // Turn on the Proton Pack if both the wand activate switch and Pack switch are ON
  {
    // ***** If Booting from a start start the booting routine ***** //
    if (PowerOff == true && startpack == true)  // If fresh poweron
    {
      switch (THEME) {
        case MOVIE:
          myDFPlayer.play(StartupTrack);
          break;
        case STATIS:
          myDFPlayer.play(AfterlifeStartup); //added to play the AfterlifeStartup
          break;
      }
      BarGraph.initiateVariables(ACTIVE);
      STATUS = BOOTING;
      //setVentLightState(ventStart, ventEnd, 2); // Vent LED's are on the pack
      CycLEDs.CyclotronBoot(CycLEDs.Wheel(255), 50, THEME);
      PackLEDs.PowercellBoot(PackLEDs.Wheel(170), 30);
    }
/*
    if (PowerOff == false && startpack == true) // Should plat a short startup sound for Afterlife after Veniting
    {
      switch (THEME){
        case STATIS:
          myDFPlayer.play(AfterlifeStartupShort); //added to play the AfterlifeStartup
          break;
      }
    }

*/
    switch (STATUS) {
      case OFF:
        // Logic if wand is on but pack is off? /////////////
        /*
           Create a function for wand LED's? Turn on Vent, ?BarGraph? Intensify?( Create Intensify Function?
        */
        clearLEDs(); // Turn Off all LED's prior to booting, Clears NFilter and Booster Tube
        break;
      // ***** Additional logic and LED routines for booting ***** //
      case BOOTING:
        PowerOff = false;
        BarGraph.sequencePackOn(currentMillis);
        setWandLightState(SloBloLED, 7, currentMillis);  //set sloblow red blinking // Changed LED 4 to SloBloLED
        setWandLightState(WhiteLED2, 1, 0);      //  set Wand ON LED white
        break;
      /********************************************************************************
         Logic routine for when the pack is fully booted, gun features work
      ********************************************************************************/
      case BOOTED:
        // ***** Based on the pack status and wand status run LED routine ***** //
        WandLightState(currentMillis);
        //Serial.println(WANDLEDSTATUS);
        if (WANDLEDSTATUS == NORMAL) {
          BarGraph.sequencePackOn(currentMillis);
        }
        if (IsPlaying == true && musicmode == false && WANDLEDSTATUS == NORMAL) {
          //          DFPlayerStart = true;
          //          DFPTrack = IdleMovieLoop;
          //          playType = 1;
          switch (THEME)  // MOVIE, STATIS, SLIME, MESON, CHRISTMAS
          {
            case MOVIE:
              myDFPlayer.loop(IdleMovieLoop);
              break;
            case STATIS:
              myDFPlayer.loop(IdleAfterlifeLoop); // AfterlifeLoop #35
              break;
            case SLIME:
              myDFPlayer.loop(IdleSlimeLoop);
              break;
            case MESON:
              myDFPlayer.loop(IdleMesonLoop);
              break;
            case CHRISTMAS:
              myDFPlayer.loop(IdleMovieLoop);
              break;
          }
        }
        /********************************************************************************
                        Firing button is pushed, start firing sequence
        ********************************************************************************/
        fire = digitalRead(FIRE_BUTTON);
        fire2 = digitalRead(FIRE_BUTTON2);
        if ((fire == true && musicmode == false) || (fire2 == true && musicmode == false)) { // Inverted fire and fire2 states false was true and true was false
          if (WANDLEDSTATUS != WARNING && WANDLEDSTATUS != FASTWARNING) {
            WANDLEDSTATUS = FIRING;
          }
          if (IsFiring == false) {
            if (fire2 == true && fire == false && WANDLEDSTATUS != WARNING)  // ***** Are the streams crossed? ***** // // Inverted fire and fire2 states false was true and true was false
            {
              if (THEME != MOVIE) {
                PlaySoundTrack(FIRING);
              } else {
                PlaySoundTrack(STREAMCROSS);
              }
              //########### CHange to crossing the streams sound
              BarGraph.initiateVariables(FIRE2);
              BarGraph.clearLEDs();
            } else if ((fire2 == false || fire == true) && WANDLEDSTATUS != WARNING) { // Inverted fire and fire2 states false was true and true was false
              PlaySoundTrack(FIRING);
              BarGraph.initiateVariables(FIRE1);
              BarGraph.clearLEDs();
            }

            firingStateMillis = currentMillis;
            IsFiring = true;
            PackLEDs.AL_Fire(true);
            CycLEDs.AL_Fire(true);
          }
          if (fire2 == true && fire == false) { // Inverted fire and fire2 states false was true and true was false
            setWandLightState(OrangeHatLED, 1, 2);  // Set gun button 2 LED white //Changed 0 to 2 to turn LED orange
            BarGraph.sequenceFire2(currentMillis);
            //digitalWrite(RUMBLE, HIGH);
          } else if (fire == true && fire2 == false) { // Inverted fire and fire2 states false was true and true was false
            BarGraph.sequenceFire1(currentMillis);
            //digitalWrite(RUMBLE, HIGH);
          }
          WandFire(currentMillis);  // Runs routine to speed up Cyclotron LEDs

          // Start Firing timing and at 10 seconds vent pack
          // After 10 seconds start the warning routing on the wand
          if ((millis() - firingStateMillis) >= firingalarm) {
            if (WANDLEDSTATUS == WARNING) {
              PlaySoundTrack(FASTWARNING);
              //BarGraph.changeInterval(10);
            }
            WANDLEDSTATUS = FASTWARNING;
            shouldVent = true;
          }
          // After 5 seconds start the alarm routing on the wand
          else if ((millis() - firingStateMillis) >= firingwarning) {
            WANDLEDSTATUS = WARNING;
            //BarGraph.changeInterval(20);
          }

        } else  // ***** Fire has stopped, Check if venting is required, reset ***** //
        {
          /*
              If you were just firing, reset things back to normal
          */

          // *** Only do this once after firing *** //
          if (IsFiring == true) {
            // ** Play tail end firing sound ** //
            if (shouldVent != true) {
              PlaySoundTrack(NORMAL);
            }
            // ** Reset Pack LED intervals ** //
            CycLEDs.CyclotronInterval(cycIdleRate[THEME]);
            PackLEDs.PowercellInterval(75);

            // ** Turn off firing rumble ** //
            //digitalWrite(RUMBLE, LOW);
          }
          IsFiring = false;
          FireRate = 0;
          FireRate2 = 0;

          // ** If you should be venting start the ventine routine ** //
          if (shouldVent == true) {
            shouldVent = false;
            venting = true;
            VentMillis = currentMillis;
          } else if (venting == true) {
            /* Venting Routine

               Play the venting track, start the bargraph venting animation, turn on N-Filter LED's.
            */
            // Turn on Smoke
            digitalWrite(VENTING, LOW);

            if (WANDLEDSTATUS == FASTWARNING) {
              myDFPlayer.play(ventTrack);
              BarGraph.initiateVariables(BGVENT);
            }
            WANDLEDSTATUS = VENTSTATE;
            PackLEDs.VentPack();
            BarGraph.sequenceVent(currentMillis);



            // ** Clear venting routine after the venting interval ** //
            if ((currentMillis - VentMillis) >= venting_interval) {
              venting = false;
              PowerOff = true;
              WANDLEDSTATUS = NORMAL;
              // Turn off Smoke
              digitalWrite(VENTING, HIGH);
              clearLEDs();
              BarGraph.initiateVariables(ACTIVE);
              if (THEME == 1) {
                PackLEDs.AL_Fire(false);
                PackLEDs.setBrightness(10);
                CycLEDs.AL_Fire(false);
                CycLEDs.setBrightness(75);
              }
            }
          } else {
            // ** If you are not in a normal state, set everything back to normal.
            if (WANDLEDSTATUS != NORMAL && venting == false) {
              WANDLEDSTATUS = NORMAL;
              BarGraph.initiateVariables(ACTIVE);
              if (THEME == 1) {
                PackLEDs.AL_Fire(false);
                PackLEDs.setBrightness(10);
                CycLEDs.AL_Fire(false);
                CycLEDs.setBrightness(75);
              }
            }
          }
        }
        break;
      case POWERDOWN:
        break;
    }
  } else if (WANDSTATUS == ON)  // run this if only the wand is ON
  {
    if (STATUS != OFF) {
      if (shuttingDown == false) {
        BarGraph.clearLEDs();
        BarGraph.initiateVariables(START);
        myDFPlayer.play(shutdownTrack);
        musicmodestart = false;
        STATUS = POWERDOWN;
        shuttingDown = true;
        PackLEDs.PowerDown(100);
        CycLEDs.PowerDown(100);
      }

    } else  // ***** If turing wand on from off State play CLICK ***** //
    {
      if (WANDLEDSTATUS != WANDONLY) {
        myDFPlayer.play(clickTrack);
        BarGraph.initiateVariables(START);
        musicmodestart = false;
      }
    }
    WANDLEDSTATUS = WANDONLY;
    WandLightState(currentMillis);
    BarGraph.sequenceStart(currentMillis);
  } else  // IF both the wand switch and pack switch are off
  {
    switch (STATUS) {
      case OFF:
        WANDLEDSTATUS = ALLOFF;
        BarGraph.sequenceShutdown(currentMillis);
        WandLightState(currentMillis);
        PackLEDs.PowerDown(100);
        CycLEDs.PowerDown(100);
        break;
      case BOOTED:
        if (shuttingDown == false) {
          BarGraph.IntervalBG = 80;
          myDFPlayer.play(shutdownTrack);
          musicmodestart = false;
          STATUS = POWERDOWN;
          WANDLEDSTATUS = ALLOFF;
          WANDSTATUS = WANDOFF;
          WandLightState(currentMillis);
          shuttingDown = true;
          PackLEDs.PowerDown(100);
          CycLEDs.PowerDown(100);
        }
        BarGraph.sequenceShutdown(currentMillis);
        break;
      case BOOTING:
        if (shuttingDown == false) {
          BarGraph.IntervalBG = 80;
          myDFPlayer.play(shutdownTrack);
          musicmodestart = false;
          STATUS = POWERDOWN;
          WANDLEDSTATUS = ALLOFF;
          WANDSTATUS = WANDOFF;
          WandLightState(currentMillis);
          shuttingDown = true;
          PackLEDs.PowerDown(100);
          CycLEDs.PowerDown(100);
        }
        BarGraph.sequenceShutdown(currentMillis);
        break;
      case POWERDOWN:
        BarGraph.sequenceShutdown(currentMillis);
        break;
    }
  }
  CycLEDs.Update();
  PackLEDs.Update();
  wandLights.show();
  //delay(1);
}

void WandFire(unsigned long currentMillis) {
  if ((currentMillis - prevCycMillis) >= fire_interval) {
    FireRate++;
    prevCycMillis = currentMillis;
    if (FireRate > 95) {
      CycLEDs.CyclotronInterval(95 - 94);
      PackLEDs.PowercellInterval(95 - 94);
    } else {
      CycLEDs.CyclotronInterval(95 - FireRate);
      PackLEDs.PowercellInterval(95 - FireRate);
    }
  }
  //
  if ((currentMillis - prevBGMillis) >= (CuurentBGInterval * 4.75)) {
    prevBGMillis = currentMillis;
    if (BarGraph.IntervalBG >= 1) {
      BarGraph.changeInterval(BarGraph.IntervalBG - 1);
    }
  }
}

// Powercell Boot Sequence Completion Callback
void PackLEDsComplete() {
  if (STATUS == BOOTING) {
    STATUS = BOOTED;
    WANDLEDSTATUS = NORMAL;
    PowerBooted = true;
    CycLEDs.IndexCLEDArray = 0;
    CycLEDs.IndexCyc = 0;
    CycLEDs.Cyclotron(PackLEDs.Color1, cycIdleRate[THEME], THEME);
    PackLEDs.Powercell(PackLEDs.Color2, 75);
    PackLEDs.PowercellClear();
  } else if (STATUS == POWERDOWN) {
    PowerOff = true;
    shuttingDown = false;
    PowerBooted = false;
    CycLEDs.Color1 = PackLEDs.Wheel(0);
    PackLEDs.Color2 = PackLEDs.Wheel(0);
    CycLEDs.ActivePatternCyc = NONE;
    PackLEDs.ActivePatternPC = NONE;
    clearLEDs();
    STATUS = OFF;
    if (WANDSTATUS == WANDOFF) {
      THEME = MOVIE;
    }
  }
}

/* 

// Ben's Edits Wand LED's
const int WhiteLED1 = 0;
const int ThemeLED = 1;
const int WandVentStart = 2;  // New
const int WandVentEnd = 8;    // New
const int WandVentLEDs = 7;   // Number of LED's on Wand Vent
const int OrangeHatLED = 9;
const int WhiteLED2 = 10;
const int SloBloLED = 11;
const int GunLEDStart = 12;
const int GunLEDEnd = 18;


Original

const int ventStart = 0;
const int ventEnd = 3;
const int SloBloLED = 4;
const int WandVentLED = 5;
const int ThemeLED = 6;
const int WhiteLED1 = 7;
const int WhiteLED2 = 8;
const int OrangeHatLED = 9;
const int GunLEDStart = 10;
const int GunLEDEnd = 16;
*/

void WandLightState(unsigned long currentMillis) {
  switch (WANDLEDSTATUS)  // enum WandSLEDState { ALLOFF, WANDONLY, NORMAL, FIRING, WARNING, FASTWARNING, VENTSTATE };
  {
    case ALLOFF:
      clearLEDs();
      break;
    case WANDONLY:
      setWandLightState(WhiteLED2, 1, 0);    //  set back light white
      setWandLightState(WandVentStart, 4, 0);  //  set wand vent led OFF
      setWandLightState(SloBloLED, 4, 0);    //  set sloblo  led OFF
      //setVentLightState(ventStart, ventEnd, 2);
      setWandLightState(OrangeHatLED, 4, 0); // Orange Hat LED Off
      break;
    case NORMAL:                              // Wand is ON and Pack is ON
      setWandLightState(WhiteLED2, 1, 0);     //  set back light white
      setWandLightState(WandVentStart, 1, 0);   //  set wand vent led white
      setWandLightState(SloBloLED, 0, 0);     //  set sloblo  led OFF
      setWandLightState(WhiteLED1, 4, 0);     // Top LED off
      setWandLightState(OrangeHatLED, 2, 0);  // Set gun button 2 LED OFF
      //setVentLightState(ventStart, ventEnd, 2);
      clearGunLEDs();  //Turn off Gun LED's
      break;
    case FIRING:
      setWandLightState(WhiteLED1, 10, currentMillis);  //set white led slow blinking
      fireStrobe(currentMillis);
      break;
    case WARNING:
      setWandLightState(WhiteLED2, 6, currentMillis);   //set white led slow blinking
      setWandLightState(WhiteLED1, 10, currentMillis);  //set white led slow blinking
      fireStrobe(currentMillis);
      break;
    case FASTWARNING:
      setWandLightState(WhiteLED2, 5, currentMillis);   //set white led fast blinking
      setWandLightState(WhiteLED1, 10, currentMillis);  //set white led slow blinking
      fireStrobe(currentMillis);
      break;
    case VENTSTATE:
      //setVentLightState(ventStart, ventEnd, 0);
      setWandLightState(OrangeHatLED, 4, 0);  // Set gun button 2 LED OFF
      setWandLightState(SloBloLED, 4, 0);     //  set sloblo  led OFF
      clearGunLEDs();                         //Turn off Gun LED's
      break;
    case STREAMCROSS:
      break;
  }

  if (STATUS != OFF && STATUS != POWERDOWN) {
    switch (THEME) {
      case MOVIE:  // Set the theme Red
        setWandLightState(ThemeLED, 0, 0);
        break;
      case STATIS:  // Set the theme blue
        setWandLightState(ThemeLED, 3, 0);
        break;
      case SLIME:  // Set the theme green
        setWandLightState(ThemeLED, 9, 0);
        break;
      case MESON:  // Set the theme yellow
        setWandLightState(ThemeLED, 12, 0);
        break;
      case CHRISTMAS:  // Set the theme flashing red / green
        setWandLightState(ThemeLED, 11, currentMillis);
        break;
    }
  } else {
    setWandLightState(ThemeLED, 4, 0);
  }
  wandLights.show();
}

/*************** Wand Light Helpers *********************
    Modified from Eric Bankers source
*/
unsigned long prevFlashMillis = 0;   // Last time we changed wand sequence 1
unsigned long prevFlashMillis2 = 0;  // Last time we changed wand sequence 2
unsigned long prevFlashMillis3 = 0;  // Last time we changed wand sequence 3
unsigned long prevFlashMillis4 = 0;  // Last time we changed wand sequence 4
unsigned long prevFlashMillis5 = 0;  // Last time we changed wand sequence 4
unsigned long prevFlashMillis6 = 0;  // Last time we changed wand sequence 4

bool flashState1 = false;
bool flashState2 = false;
bool flashState3 = false;
bool flashState4 = false;
bool flashState5 = false;
bool flashState6 = false;
const unsigned long wandFastFlashInterval = 100;    // interval at which we flash the top led on the wand
const unsigned long wandMediumFlashInterval = 500;  // interval at which we flash the top led on the wand

void setWandLightState(int lednum, int state, unsigned long currentMillis) {
  switch (state) {
    case 0:  // set led red
      wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
      break;
    case 1:  // set led white
      wandLights.setPixelColor(lednum, wandLights.Color(0, 0, 0, 255));
      break;
    case 2:  // set led orange
      wandLights.setPixelColor(lednum, wandLights.Color(255, 89, 11)); // changed (255, 127, 0) to (255, 61, 4)
      break;
    case 3:  // set led blue
      wandLights.setPixelColor(lednum, wandLights.Color(0, 0, 255));
      break;
    case 4:  // set led off
      wandLights.setPixelColor(lednum, 0);
      break;
    case 5:  // fast white flashing
      if ((unsigned long)(currentMillis - prevFlashMillis) >= wandFastFlashInterval) {
        prevFlashMillis = currentMillis;
        if (flashState1 == false) {
          wandLights.setPixelColor(lednum, wandLights.Color(0, 0, 0, 255)); // Removed the colours and let the white LED turn on.
          flashState1 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState1 = false;
        }
      }
      break;
    case 6:  // slower orange flashing
      if ((unsigned long)(currentMillis - prevFlashMillis2) >= wandMediumFlashInterval) {
        prevFlashMillis2 = currentMillis;
        if (flashState2 == false) {
          wandLights.setPixelColor(lednum, wandLights.Color(0, 0, 0, 255)); // Removed the colours and let the white LED turn on.
          //wandLights.setPixelColor(lednum, wandLights.Color(255, 127, 0));
          flashState2 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState2 = false;
        }
      }
      break;
    case 7:  // medium red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis3) >= wandMediumFlashInterval) {
        prevFlashMillis3 = currentMillis;
        if (flashState3 == false) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState3 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState3 = false;
        }
      }
      break;
    case 8:  // fast red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis4) >= wandFastFlashInterval) {
        prevFlashMillis4 = currentMillis;
        if (flashState4 == false) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState4 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState4 = false;
        }
      }
      break;
    case 9:  // set LED green
      wandLights.setPixelColor(lednum, wandLights.Color(0, 255, 0));
      break;
    case 10:  // slower orange flashing
      if ((unsigned long)(currentMillis - prevFlashMillis5) >= wandMediumFlashInterval) {
        prevFlashMillis5 = currentMillis;
        if (flashState5 == false) {
          wandLights.setPixelColor(lednum, 0);
          flashState5 = true;
        } else {
          wandLights.setPixelColor(lednum, wandLights.Color(0, 0, 0, 255)); // Removed the colours and let the white LED turn on.
          flashState5 = false;
        }
      }
      break;
    case 11:  // slower orange flashing
      if ((unsigned long)(currentMillis - prevFlashMillis6) >= wandMediumFlashInterval) {
        prevFlashMillis6 = currentMillis;
        if (flashState6 == false) {
          wandLights.setPixelColor(lednum, wandLights.Color(0, 255, 0));
          flashState6 = true;
        } else {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState6 = false;
        }
      }
      break;
    case 12:  // set led yellow
      wandLights.setPixelColor(lednum, wandLights.Color(129, 126, 0));
      break;
  }
  wandLights.show();
}




/*************** Vent Light *************************/
/*
void setVentLightState(int startLed, int endLed, int state) {
  switch (state) {
    case 0:  // set all leds to white
      for (int i = startLed; i <= endLed; i++) {
        wandLights.setPixelColor(i, wandLights.Color(255, 255, 255));
      }
      // Set the relay to on while venting. If relay is off set the pin LOW
      digitalWrite(VENTING, HIGH);
      break;
    case 1:  // set all leds to blue
      for (int i = startLed; i <= endLed; i++) {
        wandLights.setPixelColor(i, wandLights.Color(0, 0, 255));
      }
      // Set the relay to on while venting. If relay is off set the pin LOW
      digitalWrite(VENTING, HIGH);
      break;
    case 2:  // set all leds off
      for (int i = startLed; i <= endLed; i++) {
        wandLights.setPixelColor(i, 0);
      }
      // Set the relay to OFF while not venting. If relay is onf set the pin HIGH
      digitalWrite(VENTING, LOW);
      break;
  }
}
*/

// ** Testing a new function to optimize the DFPlayer to improve conflicts ** // Doesn't seem to get called.

void PlayDFPLayerHelper(unsigned long currentMillis) {
  if (DFPTrack == IdleMovieLoop) {
    IntervalDFPMillis = 5000;
  } else {
    IntervalDFPMillis = 20;
  }
  if ((currentMillis - prevDFPMillis) >= IntervalDFPMillis) {
    if (prevDFPMillis == 0) {
      prevDFPMillis = currentMillis;
    } else {
      prevDFPMillis = currentMillis;
      if (playType == 0) {
        myDFPlayer.play(DFPTrack);
      } else if (playType == 1) {
        myDFPlayer.loop(DFPTrack);
      }
      DFPlayerStart = false;
      prevDFPMillis = 0;
    }
  }
}


void PlaySoundTrack(int Track)  //enum WandSLEDState { ALLOFF, WANDONLY, NORMAL, FIRING, WARNING, FASTWARNING, VENTSTATE };
{
  switch (THEME)  // MOVIE, STATIS, SLIME, MESON, CHRISTMAS
  {
    case MOVIE:
      if (Track == FIRING) {
        myDFPlayer.loop(FireMovie);
      } else if (Track == FASTWARNING) {
        if (fire == false && fire2 == true) { // Inverted fire and fire2 states false was true and true was false
          myDFPlayer.loop(warnCrossStreams);
        } else {
          myDFPlayer.loop(warnMovie);
        }

      } else if (Track == NORMAL) {
        //        DFPlayerStart = true;
        //        DFPTrack = MovieTail;
        //        playType = 0;
        myDFPlayer.play(MovieTail); 
      } else if (Track == STREAMCROSS) {
        myDFPlayer.play(FireCrossStreams);
      }
      break;
    case STATIS:
      if (Track == FIRING) {
        myDFPlayer.loop(FireAfterlife);
      } else if (Track == FASTWARNING) {
        if (fire == false && fire2 == true) { // Inverted fire and fire2 states false was true and true was false
          myDFPlayer.loop(warnCrossStreams);
        } else {
          myDFPlayer.loop(warnAfterlife);
        }

      } else if (Track == NORMAL) {
        myDFPlayer.play(AfterlifeTail); //Changed MovieTail to AfterlifeTail
      } else if (Track == STREAMCROSS) {
        myDFPlayer.play(FireAfterlife); // AfterlifeStreamCross
      }
      break;
    case SLIME:
      if (Track == FIRING) {
        myDFPlayer.loop(FireSlime);
      } else if (Track == FASTWARNING) {
        myDFPlayer.loop(warnSlime);
      } else if (Track == NORMAL) {
        myDFPlayer.play(SlimeTail);
      }
      break;
    case MESON:
      if (Track == FIRING) {
        myDFPlayer.loop(FireMeson);
      } else if (Track == FASTWARNING) {
        myDFPlayer.loop(warnMeson);
      } else if (Track == NORMAL) {
        myDFPlayer.play(MesonTail);
      }
      break;
    case CHRISTMAS:
      if (Track == FIRING) {
        myDFPlayer.loop(FireChristmas);
      } else if (Track == FASTWARNING) {
        myDFPlayer.loop(warnMovie);
      } else if (Track == NORMAL) {
        myDFPlayer.play(ChristmasTail);
      }
      break;
  }
}

void getWandSTATUS() {
  if (startwand == true and startpack == true) {
    WANDSTATUS = ON_PACK;
  } else if (startwand == true and startpack == false) {
    WANDSTATUS = ON;
  } else {
    WANDSTATUS = WANDOFF;
  }
}

// Startup / Shutdown functions that clear all the LEDs
void clearLEDs() {
  for (int i = 0; i <= NeoPixelCountPack-1; i++) {
    PackLEDs.setPixelColor(i, 0);
  }
  for (int i = 0; i <= NeoPixelCountWand; i++) {
    wandLights.setPixelColor(i, 0);
  }
  CycLEDs.Update(); // Added an Update call to the LEDclear for the PackLEDs. Vent lights weren't always turining off after venting
  PackLEDs.Update();
}
void clearGunLEDs() {
  for (int i = GunLEDStart; i <= GunLEDEnd; i++) {
    wandLights.setPixelColor(i, 0);
  }
}


/*
   THEME color schemes
   enum PackTheme { MOVIE = 0, STATIS = 1, SLIME = 2, MESON = 3, CHRISTMAS = 4 };
*/

unsigned long ThemeGunColors1[5] = { 255, 255, 0, 255, 255 };
unsigned long ThemeGunColors2[5] = { 255, 255, 255, 255, 0 };
unsigned long ThemeGunColors3[5] = { 255, 255, 0, 0, 0 };

unsigned long ThemeGunColors12[5] = { 255, 255, 0, 255, 0 };
unsigned long ThemeGunColors22[5] = { 0, 0, 255, 255, 255 };
unsigned long ThemeGunColors32[5] = { 0, 0, 0, 0, 0 };

unsigned long ThemeGunColors13[5] = { 0, 0, 0, 0, 0 };
unsigned long ThemeGunColors23[5] = { 0, 0, 0, 0, 0 };
unsigned long ThemeGunColors33[5] = { 255, 255, 0, 0, 0 };

unsigned long ThemeGunColors14[5] = { 255, 255, 0, 0, 0 };
unsigned long ThemeGunColors24[5] = { 0, 0, 0, 0, 0 };
unsigned long ThemeGunColors34[5] = { 255, 255, 0, 0, 0 };

unsigned long ThemeGunColors15[5] = { 0, 0, 0, 0, 0 };
unsigned long ThemeGunColors25[5] = { 255, 255, 0, 0, 0 };
unsigned long ThemeGunColors35[5] = { 0, 0, 0, 0, 0 };

/*************** Firing Animations *********************/
unsigned long prevFireMillis = 0;
const unsigned long fire_interval2 = 50;  // interval at which to cycle lights (milliseconds).
int fireSeqNum = 0;
int fireSeqTotal = 5;

void fireStrobe(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - prevFireMillis) >= fire_interval2) {
    prevFireMillis = currentMillis;

    /*
    const int GunLEDStart = 12;
    const int GunLEDEnd = 18;
    
    Updated Pin numbers to be 12 - 18 from 10 - 16
    */

    switch (fireSeqNum) {
      case 0:
        wandLights.setPixelColor(12, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(13, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(14, 0);
        wandLights.setPixelColor(15, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(16, 0);
        wandLights.setPixelColor(17, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(18, 0);
        break;
      case 1:
        wandLights.setPixelColor(12, wandLights.Color(ThemeGunColors13[THEME], ThemeGunColors23[THEME], ThemeGunColors33[THEME]));
        wandLights.setPixelColor(13, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(14, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(15, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(16, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(17, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(18, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        break;
      case 2:
        wandLights.setPixelColor(12, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(13, 0);
        wandLights.setPixelColor(14, wandLights.Color(ThemeGunColors13[THEME], ThemeGunColors23[THEME], ThemeGunColors33[THEME]));
        wandLights.setPixelColor(15, 0);
        wandLights.setPixelColor(16, wandLights.Color(ThemeGunColors13[THEME], ThemeGunColors23[THEME], ThemeGunColors33[THEME]));
        wandLights.setPixelColor(17, 0);
        wandLights.setPixelColor(18, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        break;
      case 3:
        wandLights.setPixelColor(12, wandLights.Color(ThemeGunColors13[THEME], ThemeGunColors23[THEME], ThemeGunColors33[THEME]));
        wandLights.setPixelColor(13, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(14, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(15, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(16, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(17, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(18, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        break;
      case 4:
        wandLights.setPixelColor(12, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(13, 0);
        wandLights.setPixelColor(14, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(15, 0);
        wandLights.setPixelColor(16, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(17, 0);
        wandLights.setPixelColor(18, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));

        break;
      case 5:
        wandLights.setPixelColor(12, wandLights.Color(ThemeGunColors14[THEME], ThemeGunColors24[THEME], ThemeGunColors34[THEME]));
        wandLights.setPixelColor(13, wandLights.Color(ThemeGunColors15[THEME], ThemeGunColors25[THEME], ThemeGunColors35[THEME]));
        wandLights.setPixelColor(14, wandLights.Color(ThemeGunColors12[THEME], ThemeGunColors22[THEME], ThemeGunColors32[THEME]));
        wandLights.setPixelColor(15, wandLights.Color(ThemeGunColors13[THEME], ThemeGunColors23[THEME], ThemeGunColors33[THEME]));
        wandLights.setPixelColor(16, wandLights.Color(ThemeGunColors14[THEME], ThemeGunColors24[THEME], ThemeGunColors34[THEME]));
        wandLights.setPixelColor(17, wandLights.Color(ThemeGunColors1[THEME], ThemeGunColors2[THEME], ThemeGunColors3[THEME]));
        wandLights.setPixelColor(18, wandLights.Color(ThemeGunColors13[THEME], ThemeGunColors23[THEME], ThemeGunColors33[THEME]));
        break;
    }

    wandLights.show();

    fireSeqNum++;
    if (fireSeqNum > fireSeqTotal) {
      fireSeqNum = 0;
    }
  }
}
