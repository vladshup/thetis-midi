#include <ButtonDebounce.h>
/* Rotary encoder read example */
#include <USBComposite.h>

// test a KY-040 style rotary encoder module on a blue pill board
// uncomment either LIBMAPLE_CORE or STM32DUINO_CORE 
//#define LIBMAPLE_CORE
#define STM32DUINO_CORE
// for STM32DUINO_CORE be sure to select a serial port in the tools menu

//Debounce delay ms
#define DEBOUNCE 100 

USBMIDI midi;
const unsigned int midi_channel = 4; // this might show up as channel 1 depending on start index

//Wheels encoders 5V-GND
const unsigned int enc0_command = 48; //encoder 0
#define ENC_A PC14 //Digital pin. Best with 5V tolerance
#define ENC_B PC15 //Digital pin. Best with 5V tolerance

//Potentiometrs 3.3V-GND
const unsigned int pot0_command = 50; // poty 0
const uint8 threshold = 1;
unsigned int old_value = 0;
unsigned int new_value = 0;
#define POT_0 PA0 //ADC pins: PA0-PA7,PB0,PB1


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
  USBComposite.setProductId(0x0031);
  pinMode(POT_0, INPUT);
  midi.begin();
  delay(1000);
    
 //Setup encoder pins as inputs
 pinMode(ENC_A, INPUT_PULLUP);
 digitalWrite(ENC_A, HIGH);
 pinMode(ENC_B, INPUT_PULLUP);
 digitalWrite(ENC_B, HIGH);

 //Buttons setup in debounce library
}//End setup

void loop()
{

    //Encoder wheel utility     
    int8_t tmpdata;
     tmpdata = read_encoder();
     if( tmpdata ) {
      midi.sendControlChange(midi_channel, enc0_command, tmpdata);
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
      new_value = temp / 32;          // convert to a value between 0-127
    
      // If difference between new_value and old_value is grater than threshold
      if ((new_value > old_value && new_value - old_value > threshold) ||
          (new_value < old_value && old_value - new_value > threshold)) {
    
        midi.sendControlChange(midi_channel, pot0_command, new_value);
    
        // Update old_value
        old_value = new_value;
      }
 
} //End loop

/* returns change in encoder state (-1,0,1) */
int8_t read_encoder()
{
 //int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
 int8_t enc_states[] = {0,127,1,0,1,0,0,127,127,0,0,1,0,1,127,0}; //PowerSDR mrx (Thetis) need only rotary direction 1 (CCW) and 127 (CW)
 static uint8_t old_AB = 0;
 /**/
 old_AB <<= 2;                   //remember previous state
 //old_AB |= ( ENC_PORT & 0x03 );  //add current state
 old_AB |= ((digitalRead (ENC_B) << 1) | digitalRead (ENC_A));
 return ( enc_states[( old_AB & 0x0f )]);  
} //End read_encoder()
