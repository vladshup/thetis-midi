
/* Rotary encoder grey code read lib only for PowerSDR(Thetis)*/
#define STM32DUINO_CORE
#include <USBComposite.h>
#include <MidiEncoder.h>
#include <MidiPot.h>
#include <AceButton.h>
using namespace ace_button;

// The pin numbers attached to the buttons.
const int RIT_BUTTON_PIN = PB4;
const int SPLIT_BUTTON_PIN = PB11;
const int AB_BUTTON_PIN = PB10;

// Buttons, automatically sharing the default System ButtonConfig.
AceButton ritButton(RIT_BUTTON_PIN);
AceButton splitButton(SPLIT_BUTTON_PIN);
AceButton abButton(AB_BUTTON_PIN);

//Encoders 5V-GND
const unsigned int vfoA_id = 40; //encoder 0 ID
MidiEncoder vfoA(PC14, PC15); //Arg's - STM32 pins
#define VFO_A_DIV 2

const unsigned int rit_id = 41; //encoder 1 ID
MidiEncoder rit(PB6, PB7);//Arg's - STM32 pins


//RIT/XIT encoder speed mult
#define MULT 1

USBMIDI midi;
const unsigned int midi_channel = 4; // this might show up as channel 1 depending on start index

//Speaker
#define SPEAKER_PIN PC13 //Fake pin for midi input emulation

class myMidi : public USBMIDI {
 virtual void handleNoteOff(unsigned int channel, unsigned int note, unsigned int velocity) {
  noTone(SPEAKER_PIN);
 }
 virtual void handleNoteOn(unsigned int channel, unsigned int note, unsigned int velocity) {
   tone(SPEAKER_PIN, (midiNoteFrequency_10ths[note]+5)/10);
  }
  
};

myMidi midiin; //Fake midi input emulation

//Potentiometrs 3.3V-GND
const uint8 threshold = 1;

//ADC pins: PA0-PA7,PB0,PB1
const unsigned int pot1_id = 50; // poty 1 ID
MidiPot pot1(PA0);

/*
const unsigned int pot2_id = 51; // poty 2 ID
MidiPot pot2(PA1);
*/


// The event handler for the buttons.
void handleEvent(AceButton* button, uint8_t eventType,
    uint8_t /*buttonState*/) {
  uint8_t pin = button->getPin();

  if (pin == RIT_BUTTON_PIN) {
    switch (eventType) {
      case AceButton::kEventPressed:
        midi.sendControlChange(midi_channel, 60, 127);
      break;
            
      case AceButton::kEventReleased:
        midi.sendControlChange(midi_channel, 60, 0);
      break;

      // LongPressed goes in and out of edit mode.
      case AceButton::kEventLongPressed:
      midi.sendControlChange(midi_channel, 61, 127);
      break;
    }
  } else if (pin == SPLIT_BUTTON_PIN) {
    switch (eventType) {
      case AceButton::kEventReleased:
        midi.sendControlChange(midi_channel, 62, 0);
        break;

      case AceButton::kEventPressed:
        midi.sendControlChange(midi_channel, 62, 127);
      break;
    }
  } else if (pin == AB_BUTTON_PIN) {

    switch (eventType) {
      case AceButton::kEventReleased:
        midi.sendControlChange(midi_channel, 63, 0);
        break;

      case AceButton::kEventPressed:
        midi.sendControlChange(midi_channel, 63, 127);
      break;
    }    
  }
  
}// End the event handler for the buttons.



void setup()
{
    //product id taken from library example
    USBComposite.setProductId(0x0030);
    pinMode(SPEAKER_PIN, OUTPUT);
    midiin.registerComponent();
    USBComposite.begin();  
    midi.begin();

  // Button uses the built-in pull up register.
  pinMode(RIT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SPLIT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(AB_BUTTON_PIN, INPUT_PULLUP);

  // Configure the ButtonConfig with the event handler.
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
    
  delay(1000);
}//End setup


void loop()
{
//Input channel  
  midiin.poll(); //Not used but need for PowerSDR(Thetis)
  
//Buttons
//Don't remember add new buttons to event handler
  ritButton.check();
  splitButton.check();
  abButton.check();
  
//Encoders  
    //VFO_A encoder
       static int8_t pulse;
       pulse++;
       int8_t tmpdata = vfoA.read();       
           if( tmpdata ) {
            if ( pulse % VFO_A_DIV == 0) {
             midi.sendControlChange(midi_channel, vfoA_id, tmpdata);
             pulse = 0;
            }
           }

    //RIT encoder   
       int8_t tmpdata1 = rit.read();       
           if( tmpdata1 ) {
              for(int i = 1; i <= MULT; i++) {
                midi.sendControlChange(midi_channel, rit_id, tmpdata1);
              }
           }
   
//Potentiometers
    if (pot1.read()) midi.sendControlChange(midi_channel, pot1_id, pot1.value);
    //if (pot2.read()) midi.sendControlChange(midi_channel, pot2_id, pot2.value);
  
} //End loop
