#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Forward declarations for functions in HXstomp_Midi.ino
void SendCC(int CC, int value);
void SetSnapshot(int n);
void ToggleTuner();
void TapTempo();
void HXbypass();
void FS4();
void FS5();
void ToggleBoost();
void ToggleFSextra();
void LoopRecOverdub();
void LoopPlayStop();
void LoopUndoRedo();
void BankUp();
void ModeUp();
void BankDown();
void ModeDown();

// Define LEDPIN and maxLedBrighntess from HXstomp_Midi.ino
#define LEDPIN 4
#define maxLedBrighntess 45
extern Adafruit_NeoPixel LED; // Declare LED as extern to be defined in HXstomp_Midi.ino

enum ActionType
{
    ACTION_NONE,
    ACTION_MIDI_CC,
    ACTION_INTERNAL_FUNCTION,
    ACTION_SET_SNAPSHOT // Special case for SetSnapshot as it's an internal function with a parameter
};

enum InternalFunction
{
    FUNC_NONE,
    FUNC_TOGGLE_TUNER,
    FUNC_TAP_TEMPO,
    FUNC_HX_BYPASS,
    FUNC_FS4,
    FUNC_FS5,
    FUNC_TOGGLE_BOOST,
    FUNC_TOGGLE_FS_EXTRA,
    FUNC_LOOP_REC_OVERDUB,
    FUNC_LOOP_PLAY_STOP,
    FUNC_LOOP_UNDO_REDO,
    FUNC_BANK_UP,
    FUNC_MODE_UP,
    FUNC_BANK_DOWN,
    FUNC_MODE_DOWN
};

struct Action
{
    ActionType type;
    union
    {
        struct
        {
            uint8_t cc_number;
            uint8_t cc_value;
        } midi_cc;
        InternalFunction internal_func;
        uint8_t snapshot_number; // For ACTION_SET_SNAPSHOT
    };

    // Constructor for MIDI CC action
    Action(uint8_t cc_num, uint8_t cc_val) : type(ACTION_MIDI_CC)
    {
        midi_cc.cc_number = cc_num;
        midi_cc.cc_value = cc_val;
    }

    // Constructor for Internal Function action
    Action(InternalFunction func) : type(ACTION_INTERNAL_FUNCTION)
    {
        internal_func = func;
    }

    // Explicit static factory method for Set Snapshot action
    static Action forSnapshot(uint8_t snapshot_num)
    {
        Action a;
        a.type = ACTION_SET_SNAPSHOT;
        a.snapshot_number = snapshot_num;
        return a;
    }

    // Default constructor for no action
    Action() : type(ACTION_NONE) {}
};

struct ButtonActions
{
    Action click_action;
    Action long_press_action;

    ButtonActions(Action click = Action(), Action long_press = Action())
        : click_action(click), long_press_action(long_press) {}
};

struct BankConfig
{
    uint32_t led_color;
    Action enter_action;
    ButtonActions buttonA;
    ButtonActions buttonB;
    ButtonActions buttonC;

    BankConfig(uint32_t color, Action enter, ButtonActions btnA, ButtonActions btnB, ButtonActions btnC)
        : led_color(color), enter_action(enter), buttonA(btnA), buttonB(btnB), buttonC(btnC) {}
};

// Define the number of banks
const int N_BANKS = 4;

// Define the bank configurations
const BankConfig bankConfigs[N_BANKS] = {
    // Bank 0: Snapshot Mode
    BankConfig(
        LED.Color(255, 0, 165),                                           // b0Color
        Action(71, 0),                                                    // enter0 SendCC(71, 0)
        ButtonActions(Action::forSnapshot(1), Action(FUNC_MODE_UP)),      // b0_Aclick SetSnapshot(1), LongPress=ModeUp
        ButtonActions(Action::forSnapshot(2), Action(FUNC_TOGGLE_BOOST)), // b0_Bclick SetSnapshot(2), LongPress=ToggleBoost
        ButtonActions(Action::forSnapshot(3), Action(FUNC_BANK_UP))       // b0_Cclick SetSnapshot(3), LongPress=BankUp
        ),
    // Bank 1: Tap / Tuner
    BankConfig(
        LED.Color(153, 255, 51),                                          // b1Color
        Action(71, 3),                                                    // enter1 SendCC(71, 3)
        ButtonActions(Action(FUNC_TOGGLE_TUNER), Action(FUNC_MODE_UP)),   // b1_Aclick ToggleTuner(), LongPress=ModeUp
        ButtonActions(Action(FUNC_TAP_TEMPO), Action(FUNC_TOGGLE_BOOST)), // b1_Bclick TapTempo(), LongPress=ToggleBoost
        ButtonActions(Action(FUNC_HX_BYPASS), Action(FUNC_BANK_UP))       // b1_Cclick HXbypass(), LongPress=BankUp
        ),
    // Bank 2: FS Mode
    BankConfig(
        LED.Color(51, 255, 255),                                          // b2Color
        Action(71, 3),                                                    // enter2 SendCC(71, 3)
        ButtonActions(Action(FUNC_FS4), Action(FUNC_MODE_UP)),            // b2_Aclick FS4(), LongPress=ModeUp
        ButtonActions(Action(FUNC_FS5), Action(FUNC_TOGGLE_BOOST)),       // b2_Bclick FS5(), LongPress=ToggleBoost
        ButtonActions(Action(FUNC_TOGGLE_FS_EXTRA), Action(FUNC_BANK_UP)) // b2_Cclick ToggleFSextra(), LongPress=BankUp
        ),
    // Bank 3: Looper Mode
    BankConfig(
        LED.Color(255, 0, 0),                                                  // b3Color
        Action(71, 3),                                                         // enter3 SendCC(71, 3)
        ButtonActions(Action(FUNC_LOOP_REC_OVERDUB), Action(FUNC_MODE_UP)),    // b3_Aclick LoopRecOverdub(), LongPress=ModeUp
        ButtonActions(Action(FUNC_LOOP_PLAY_STOP), Action(FUNC_TOGGLE_BOOST)), // b3_Bclick LoopPlayStop(), LongPress=ToggleBoost
        ButtonActions(Action(FUNC_LOOP_UNDO_REDO), Action(FUNC_BANK_UP))       // b3_Cclick LoopUndoRedo(), LongPress=BankUp
        )};

#endif // CONFIG_H
