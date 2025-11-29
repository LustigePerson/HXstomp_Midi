#include <Wire.h>
#include "OneButton.h"
#include <EEPROM.h>
#include "config.h"            // Include the new configuration file
#include <Adafruit_NeoPixel.h> // Moved after config.h to ensure LEDPIN and maxLedBrighntess are defined

// ##################### LED ####################
// LEDPIN and maxLedBrighntess are now defined in config.h
Adafruit_NeoPixel LED(1, LEDPIN, NEO_RGB + NEO_KHZ800);

// ##################### Power and GND for Midi In and Midi OUT ####################
int gndPin = 9;   // set to LOW and connect GND to the midi circuit here. (no need to disconnect midi devices for sketch upload)
int powerPin = 8; // set to HIGH and connect power to the midi circuit here. (no need to disconnect midi devices for sketch upload)

// ##################### Pins for Buttons ####################
OneButton SwitchA(10, true); // Setup a new OneButton
OneButton SwitchB(11, true); // Setup a new OneButton
OneButton SwitchC(12, true); // Setup a new OneButton
#define pressTime 300
#define debounceTime 25

// ##################### General Variables ####################
unsigned long blinkPreviousMillis = 0;
int blinkInterval = 0;

bool bypassActive = false;
bool boostActive = false;   // CC 100 for Toggle
bool FSextraActive = false; // CC 101 for Toggle

int looperCurrent = 0;     // 0=stopped, 1=playing, 2=recording
bool recModeRecord = true; // rec or overdup

#define MIDICHAN 1

// ##################### Banks ####################
const int n_banks = N_BANKS; // Use N_BANKS from config.h
int currentBank = 0;
bool bankEnabled[n_banks];

// Removed hardcoded bank configurations, now in config.h

// Helper function to execute an action based on its type
void executeAction(Action action)
{
  switch (action.type)
  {
  case ACTION_MIDI_CC:
    SendCC(action.midi_cc.cc_number, action.midi_cc.cc_value);
    break;
  case ACTION_INTERNAL_FUNCTION:
    switch (action.internal_func)
    {
    case FUNC_TOGGLE_TUNER:
      ToggleTuner();
      break;
    case FUNC_TAP_TEMPO:
      TapTempo();
      break;
    case FUNC_HX_BYPASS:
      HXbypass();
      break;
    case FUNC_FS4:
      FS4();
      break;
    case FUNC_FS5:
      FS5();
      break;
    case FUNC_TOGGLE_BOOST:
      ToggleBoost();
      break;
    case FUNC_TOGGLE_FS_EXTRA:
      ToggleFSextra();
      break;
    case FUNC_LOOP_REC_OVERDUB:
      LoopRecOverdub();
      break;
    case FUNC_LOOP_PLAY_STOP:
      LoopPlayStop();
      break;
    case FUNC_LOOP_UNDO_REDO:
      LoopUndoRedo();
      break;
    case FUNC_BANK_UP:
      BankUp();
      break;
    case FUNC_MODE_UP:
      ModeUp();
      break;
    case FUNC_BANK_DOWN:
      BankDown();
      break;
    case FUNC_MODE_DOWN:
      ModeDown();
      break;
    case FUNC_NONE:
    default:
      // Do nothing for FUNC_NONE or unknown functions
      break;
    }
    break;
  case ACTION_SET_SNAPSHOT:
    SetSnapshot(action.snapshot_number);
    break;
  case ACTION_NONE:
  default:
    // Do nothing for ACTION_NONE or unknown action types
    break;
  }
}

// Wrapper functions for long press events
void handleLongPressA()
{
  executeAction(bankConfigs[currentBank].buttonA.long_press_action);
}

void handleLongPressB()
{
  executeAction(bankConfigs[currentBank].buttonB.long_press_action);
}

void handleLongPressC()
{
  executeAction(bankConfigs[currentBank].buttonC.long_press_action);
}

void setup()
{
  Serial.begin(31250); // MIDI:31250 // SERIAL MONITOR:9600 OR 115200

  // pins for Midi (including power supply pins, so midi is not screwed up when uploading a new sketch)
  pinMode(powerPin, OUTPUT);
  pinMode(gndPin, OUTPUT);
  digitalWrite(powerPin, HIGH); // Power up
  digitalWrite(gndPin, LOW);    // Power up

  // set timings
  // longpress
  SwitchA.setPressTicks(pressTime);
  SwitchB.setPressTicks(pressTime);
  SwitchC.setPressTicks(pressTime);

  // debounce
  SwitchA.setDebounceTicks(debounceTime);
  SwitchB.setDebounceTicks(debounceTime);
  SwitchC.setDebounceTicks(debounceTime);

  // Initialize bankEnabled array to all enable for first startup
  for (int i = 0; i < n_banks; ++i)
  {
    bankEnabled[i] = true;
  }
  // Read bank enable/disable states from EEPROM
  for (int i = 0; i < n_banks; ++i)
  {
    byte storedValue = EEPROM.read(i);
    if (storedValue != 0 && storedValue != 1)
    {
      // If the stored value is not 0 or 1, overwrite it with the default setting from bankEnabled
      EEPROM.update(i, bankEnabled[i]);
    }
    else
    {
      // Update the state only if the stored value is 0 (false) or 1 (true)
      bankEnabled[i] = storedValue;
    }
  }

  startupLED();

  // switch bank state if button A/B is held
  unsigned long startupStart = millis();
  while (millis() - startupStart <= 1000)
  {
    SwitchA.tick();
    SwitchB.tick();
    SwitchC.tick();

    if (SwitchA.isLongPressed() || SwitchB.isLongPressed() || SwitchC.isLongPressed())
    {
      if (SwitchA.isLongPressed())
      {
        bankEnabled[1] = !bankEnabled[1];
        EEPROM.update(1, bankEnabled[1]);
      }
      if (SwitchB.isLongPressed())
      {
        bankEnabled[2] = !bankEnabled[2];
        EEPROM.update(2, bankEnabled[2]);
      }
      if (SwitchC.isLongPressed())
      {
        bankEnabled[3] = !bankEnabled[3];
        EEPROM.update(3, bankEnabled[3]);
      }
      break;
    }
  }

  showInitializedBanks();

  // link the button functions
  // Button A
  SwitchA.attachClick(SwitchAclick);
  SwitchA.attachLongPressStart(handleLongPressA); // Updated to use wrapper

  // Button B
  SwitchB.attachClick(SwitchBclick);
  SwitchB.attachLongPressStart(handleLongPressB); // Updated to use wrapper

  // Button C
  SwitchC.attachClick(SwitchCclick);
  SwitchC.attachLongPressStart(handleLongPressC); // Updated to use wrapper

  enterBank();
}

void loop()
{
  // ********************Monitor the Buttons***************
  SwitchA.tick();
  SwitchB.tick();
  SwitchC.tick();

  if (blinkInterval != 0)
  {
    blinkLoopLED();
  }

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
  executeAction(bankConfigs[currentBank].buttonA.click_action);
}

void SwitchBclick()
{
  executeAction(bankConfigs[currentBank].buttonB.click_action);
}

void SwitchCclick()
{
  executeAction(bankConfigs[currentBank].buttonC.click_action);
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

void LoopRecOverdub()
{
  if (recModeRecord == true)
  {
    SendCC(60, 127);       // Record
    recModeRecord = false; // recModeRecord press now overdups
  }
  else if (recModeRecord == false)
  {
    SendCC(60, 0); // Overdup
  }
  else
  {
    LED.setPixelColor(0, LED.Color(255, 255, 255));
    LED.show();
  }
  looperCurrent = 2;
  blinkInterval = 100;
}

void LoopPlayStop()
{
  if (looperCurrent == 0 || looperCurrent == 2)
  {
    SendCC(61, 127); // start playing
    looperCurrent = 1;
    blinkInterval = 250;
    recModeRecord = false; // recModeRecord press now overdups
  }
  else if (looperCurrent == 1)
  {
    SendCC(61, 0); // stop playing
    looperCurrent = 0;
    blinkInterval = 0;
    SwitchLED();
    recModeRecord = true; // recModeRecord press now records new loop
  }
}

void LoopUndoRedo()
{
  SendCC(63, 127);
}

void BankUp()
{
  do
  {
    currentBank = (currentBank + 1) % n_banks;
  } while (!bankEnabled[currentBank]);
  enterBank();
}

void BankDown()
{
  do
  {
    currentBank = (currentBank - 1 + n_banks) % n_banks;
  } while (!bankEnabled[currentBank]);
  enterBank();
}

void enterBank()
{
  executeAction(bankConfigs[currentBank].enter_action);
  SwitchLED();
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
  delay(250);
  LED.setPixelColor(0, LED.Color(255, 255, 255));
  LED.show();
  delay(500);
}

void showInitializedBanks()
{
  LED.clear();
  LED.show();
  delay(250);
  int originalBank = currentBank;
  for (int i = 0; i < n_banks; ++i)
  {
    if (bankEnabled[i])
    {
      currentBank = i;
      SwitchLED();
      delay(550);
      LED.clear();
      LED.show();
      delay(250);
    }
  }
  currentBank = originalBank;
  delay(500);
}

void SwitchLED()
{
  LED.setPixelColor(0, bankConfigs[currentBank].led_color);
  LED.show();
}

void blinkLED(int times, int delay_ms, int red, int green, int blue)
{
  int i;
  for (i = 0; i < times; i++)
  {
    LED.clear();
    LED.show();
    delay(delay_ms);
    LED.setPixelColor(0, LED.Color(red, green, blue));
    LED.show();
    delay(delay_ms);
  }
}

void rainbow()
{
  int i, j;
  for (i = 0; i < 1; i++)
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

void blinkLoopLED()
{
  unsigned long currentMillis = millis();

  if (currentMillis - blinkPreviousMillis >= blinkInterval)
  {
    // save the last time you blinked the LED
    blinkPreviousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (LED.getPixelColor(0) == 0)
    {
      SwitchLED();
    }
    else
    {
      LED.clear();
      LED.show();
    }
  }
}
