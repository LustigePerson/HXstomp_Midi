#include <Wire.h>
#include "OneButton.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// ##################### LED ####################
#define LEDPIN 4
#define maxLedBrighntess 50
Adafruit_NeoPixel LED(1, LEDPIN, NEO_RGB + NEO_KHZ800);

// ##################### Power and GND for Midi In and Midi OUT ####################
int gndPin = 9;   // set to LOW and connect GND to the midi circuit here. (no need to disconnect midi devices for sketch upload)
int powerPin = 8; // set to HIGH and connect power to the midi circuit here. (no need to disconnect midi devices for sketch upload)

// ##################### Pins for Buttons ####################
OneButton SwitchA(10, true); // Setup a new OneButton
OneButton SwitchB(11, true); // Setup a new OneButton
OneButton SwitchC(12, true); // Setup a new OneButton
#define pressTime 250
#define debounceTime 20

// ##################### General Variables ####################
int n_banks = 3;
int currentBank = 0;
bool bypassActive = false;
bool boostActive = false;   // CC 100 for Toggle
bool FSextraActive = false; // CC 101 for Toggle

#define MIDICHAN 1

// ##################### Bank 0 ####################
// ################# Snapshot Mode ################
#define b0Color 255, 0, 165
#define enter0 SendCC(71, 0)
#define b0_Aclick SetSnapshot(1)
#define b0_Bclick SetSnapshot(2)
#define b0_Cclick SetSnapshot(3)

// ##################### Bank 1 ####################
// ################## Tap / Tuner #################
#define b1Color 153, 255, 51
#define enter1 SendCC(71, 3)
#define b1_Aclick ToggleTuner()
#define b1_Bclick TapTempo()
#define b1_Cclick HXbypass()

// ##################### Bank 2 ####################
// #################### FS Mode ###################
#define b2Color 51, 255, 255
#define enter2 SendCC(71, 3)
#define b2_Aclick FS4()
#define b2_Bclick FS5()
#define b2_Cclick ToggleFSextra()

void setup()
{
  Serial.begin(31250); // MIDI:31250 // SERIAL MONITOR:9600 OR 115200

  // pins for Midi (including power supply pins, so midi is not screwed up when uploading a new sketch)
  pinMode(powerPin, OUTPUT);
  pinMode(gndPin, OUTPUT);
  digitalWrite(powerPin, HIGH); // Power up
  digitalWrite(gndPin, LOW);    // Power up

  startupLED();
  enterBank();

  // set timings
  SwitchA.setPressTicks(pressTime);
  SwitchB.setPressTicks(pressTime);
  SwitchC.setPressTicks(pressTime);

  SwitchA.setDebounceTicks(debounceTime);
  SwitchB.setDebounceTicks(debounceTime);
  SwitchC.setDebounceTicks(debounceTime);

  // link the button functions
  // Button A
  SwitchA.attachClick(SwitchAclick);
  SwitchA.attachLongPressStop(ModeUp);

  // Button B
  SwitchB.attachClick(SwitchBclick);
  SwitchB.attachLongPressStop(ToggleBoost);

  // Button C
  SwitchC.attachClick(SwitchCclick);
  SwitchC.attachLongPressStop(BankUp);
}

void loop()
{
  // ********************Monitor the Buttons***************
  SwitchA.tick();
  SwitchB.tick();
  SwitchC.tick();

} // Loop

//**************** Device Specific Functions below this point (e.g. for Line6 HX Stomp) **************************
void SendCC(int CC, int value)
{
  Serial.write(176);
  Serial.write(CC);
  Serial.write(value);
}

void SetSnapshot(int n)
{
  SendCC(69, n - 1);
}

void SwitchAclick()
{
  if (currentBank == 0)
  {
    b0_Aclick;
  }
  else if (currentBank == 1)
  {
    b1_Aclick;
  }
  else if (currentBank == 2)
  {
    b2_Aclick;
  }
  else
  {
    ;
  }
}

void SwitchBclick()
{
  if (currentBank == 0)
  {
    b0_Bclick;
  }
  else if (currentBank == 1)
  {
    b1_Bclick;
  }
  else if (currentBank == 2)
  {
    b2_Bclick;
  }
  else
  {
    ;
  }
}

void SwitchCclick()
{
  if (currentBank == 0)
  {
    b0_Cclick;
  }
  else if (currentBank == 1)
  {
    b1_Cclick;
  }
  else if (currentBank == 2)
  {
    b2_Cclick;
  }
  else
  {
    ;
  }
}

void FS4()
{
  SendCC(52, 127);
}

void FS5()
{
  SendCC(53, 127);
}

void HXbypass()
{
  if (bypassActive == false)
  {
    SendCC(70, 0);
    bypassActive = true;
  }
  else if (bypassActive == true)
  {
    SendCC(70, 127);
    bypassActive = false;
  }
}

void ToggleBoost()
{
  if (boostActive == false)
  {
    SendCC(100, 0);
    boostActive = true;
  }
  else if (boostActive == true)
  {
    SendCC(100, 127);
    boostActive = false;
  }
}

void ToggleFSextra()
{
  if (FSextraActive == false)
  {
    SendCC(101, 0);
    FSextraActive = true;
  }
  else if (FSextraActive == true)
  {
    SendCC(101, 127);
    FSextraActive = false;
  }
}

void ToggleTuner()
{
  SendCC(68, 127);
}

void TapTempo()
{
  SendCC(64, 127);
}

void BankUp()
{
  currentBank = (currentBank + 1) % n_banks;
  enterBank();
}

void BankDown()
{
  currentBank = (currentBank - 1) % n_banks;
  enterBank();
}

void ModeUp()
{
  SendCC(71, 4);
}

void ModeDown()
{
  SendCC(71, 5);
}

void startupLED()
{
  LED.begin();
  LED.setBrightness(maxLedBrighntess);
  rainbow();
  LED.clear();
  LED.show();
  delay(100);
  SwitchLED();
  delay(50);
  LED.clear();
  LED.show();
  delay(100);
  SwitchLED();
}

void SwitchLED()
{
  if (currentBank == 0)
  {
    LED.setPixelColor(0, LED.Color(b0Color));
  }
  else if (currentBank == 1)
  {
    LED.setPixelColor(0, LED.Color(b1Color));
  }
  else if (currentBank == 2)
  {
    LED.setPixelColor(0, LED.Color(b2Color));
  }
  else
  {
    LED.setPixelColor(0, LED.Color(255, 255, 255));
  }
  LED.show();
}

void enterBank()
{
  if (currentBank == 0)
  {
    enter0;
  }
  else if (currentBank == 1)
  {
    enter1;
  }
  else if (currentBank == 2)
  {
    enter2;
  }
  SwitchLED();
}

void rainbow()
{
  int i, j;
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j <= 255; j++)
    {
      LED.setPixelColor(0, Wheel(j));
      LED.show();
      delay(5);
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return LED.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return LED.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return LED.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
