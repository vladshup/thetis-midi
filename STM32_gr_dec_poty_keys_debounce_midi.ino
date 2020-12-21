#include <ButtonDebounce.h>
/* Rotary encoder grey code read lib only for PowerSDR(Thetis)*/
#include <MidiEncoder.h>
#include <USBComposite.h>
#define STM32DUINO_CORE

//Buttons debounce delay ms
#define DEBOUNCE 100

//OOP Wheel encoders 5V-GND
const unsigned int enc0_command = 40; //encoder 0 ID
MidiEncoder enc0(PC14, PC15); //Arg's - STM32 pins
/*
const unsigned int enc1_command = 41; //encoder 1 ID
MidiEncoder enc1(PB03, PB04);//Arg's - STM32 pins
*/

//Main encoder speed divider
#define SDIV 4

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

#define POT_0 PA0 //ADC pins: PA0-PA7,PB0,PB1
const unsigned int pot0_command = 50; // poty 0 ID
unsigned int pot0_old_value = 0;
unsigned int pot0_new_value = 0;

/*
#define POT_1 PA1 //ADC pins: PA0-PA7,PB0,PB1
const unsigned int pot1_command = 51; // poty 1 ID
unsigned int pot1_old_value = 0;
unsigned int pot1_new_value = 0;
*/

//Buttons
const unsigned int key0_command = 60; // button 0
#define KEY_0 PB11 //Any digital pin
unsigned int key00 = 0;
ButtonDebounce key0(KEY_0, DEBOUNCE);

const unsigned int key1_command = 61; // button 1
#define KEY_1 PB10 //Any digital pin
unsigned int key01 = 0;
ButtonDebounce key1(KEY_1, DEBOUNCE);

void setup()
{
    //product id taken from library example
    USBComposite.setProductId(0x0030);
    pinMode(SPEAKER_PIN, OUTPUT);
    midiin.registerComponent();
    USBComposite.begin();
  
    pinMode(POT_0, INPUT);
    midi.begin();
    delay(1000);
}//End setup


void loop()
{
  midiin.poll(); //Not used but need for PowerSDR(Thetis)

    //Encoder wheel utility     
       static int8_t pulse;
       pulse++;
       int8_t tmpdata = enc0.read();
       
           if( tmpdata ) {

            if ( pulse % SDIV == 0) {
             midi.sendControlChange(midi_channel, enc0_command, tmpdata);
             pulse = 0;
            }
           }

    //Buttons utility
    key0.update();
    int8_t old_key0;
    old_key0 = key00;
    key00 = key0.state();
    if (key00 == old_key0) {  
    } else {
      midi.sendControlChange(midi_channel, key0_command, key00*127);  
    }
    
    key1.update();
    int8_t old_key1;
    old_key1 = key01;
    key01 = key1.state();
    if (key01 == old_key1) {  
    } else {
      midi.sendControlChange(midi_channel, key1_command, key01*127);  
    }
    
    //Poty utility
      int temp = analogRead(POT_0); // a value between 0-4095
      pot0_new_value = temp / 32;          // convert to a value between 0-127
    
      // If difference between new_value and old_value is grater than threshold
      if ((pot0_new_value > pot0_old_value && pot0_new_value - pot0_old_value > threshold) ||
          (pot0_new_value < pot0_old_value && pot0_old_value - pot0_new_value > threshold)) {
    
        midi.sendControlChange(midi_channel, pot0_command, pot0_new_value);
    
        // Update old_value
        pot0_old_value = pot0_new_value;
      }
 
} //End loop

/* returns change in encoder state (-1,0,1) */
/*
int8_t read_encoder(int8_t enc_a, int8_t enc_b)
{
 //int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
 int8_t enc_states[] = {0,127,1,0,1,0,0,127,127,0,0,1,0,1,127,0}; //PowerSDR mrx (Thetis) need only rotary direction 1 (CCW) and 127 (CW)
 static uint8_t old_AB = 0;

 old_AB <<= 2;                   //remember previous state
 //old_AB |= ( ENC_PORT & 0x03 );  //add current state
 old_AB |= ((digitalRead (enc_b) << 1) | digitalRead (enc_a));
 return ( enc_states[( old_AB & 0x0f )]);  
} //End read_encoder()
*/
