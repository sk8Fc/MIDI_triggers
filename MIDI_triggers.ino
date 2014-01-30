/*    
 * File: MIDI_triggers.ino
 * The code is for the Sparkfun MP3 Player shield and VS1053 breakout board and the Arduino MEGA ADK.
 *
 * Reference :
 *            File:     MP3_Shield_RealtimeMIDI.ino 
 *            Author:   Matthias Neeracher
 *
 * This code is in the public domain, with the exception of the contents of sVS1053b_Realtime_MIDI_Plugin.
 *
 *
 * http://www.sparkfun.com/Code/MIDI_Example.pde
 * http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Dev/Arduino/Shields/VS_Shield_Example.zip
 *
 *      Spark Fun Electronics 2011
 *      Nathan Seidle
 *
 */ 

#include <SoftwareSerial.h>
SoftwareSerial midiSerial(2,3); // Soft TX on 3, RX not used (2 is an input anyway, for VS_DREQ)



#define VS_GPIO1  4  // Mode selection (0 = file / 1 = real time MIDI)
#define VS_RESET  8 //Reset is active low



int instrument = 0;
int pinNext = A1;
int pinPrev = A0;
const int pinKey[10] = {
  A2, A3, A4, A5, A6, A7, A8, A9, A10, A11
};

const int pinDrum[4] = {
  22, 24, 26, 28
};

// http://soundprogramming.net/file_formats/general_midi_drum_note_numbers
const int noteDrums[4] = {
  35, 42, 47, 38
};
  
// http://www.phys.unsw.edu.au/jw/graphics/notes.GIF
const int noteKeys[10] = {
  60, 62, 64, 65, 67, 69, 71, 72, 74, 76
};

boolean booKeys[10] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

boolean booDrums[4] = {
  0, 0, 0, 0
};

const char* keys[10] = {
  "do|C4 ", "re|D4 ", "mi|E4 ", "fa|F4 ", "so|G4 ", "la|A4 ", "si|B4 ", "do|C5 ", "re|D5 ",
  "mi|E5 "
};

void setup() {
  midiSerial.begin(31250);

  pinMode(VS_RESET, OUTPUT);

  pinMode(pinNext, INPUT);
  pinMode(pinPrev, INPUT);
  for(int i = 0 ; i < 7 ; i++) {
    pinMode(pinKey[i], INPUT);
  }

  Serial.begin(57600); //Use serial for debugging 
  Serial.println("\n******\n");
  Serial.println("MP3 Shield Example");

  //Initialize VS1053 chip 
  digitalWrite(VS_RESET, LOW); //Put VS1053 into hardware reset


  delayMicroseconds(1);
  digitalWrite(VS_RESET, HIGH); //Bring up VS1053
  
  pinMode(VS_GPIO1, OUTPUT);
  digitalWrite(VS_GPIO1, HIGH);  // Enable real time MIDI mode
}

void sendMIDI(byte data) {
  midiSerial.write(data);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  sendMIDI(cmd);
  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0 || (cmd & 0xF0) >= 0xE0) {
    sendMIDI(data1);
    sendMIDI(data2);
  } else {
    sendMIDI(data1);
  }

}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}



void loop() {
  
  talkMIDI(0xB0, 0x07, 120); //0xB0 is channel message, set channel volume to near max (127)

  //control midi
  if (digitalRead(pinNext) == HIGH){
    instrument++;
    if(instrument == 127)
      instrument = 0;
    Serial.print(" Instrument No: ");
    Serial.println(instrument, DEC);
    delay(200);
  }

  if (digitalRead(pinPrev) == HIGH){
    instrument--;
    if(instrument == -1)
      instrument = 127;
    Serial.print(" Instrument No: ");
    Serial.println(instrument, DEC);
    delay(200);
  }

  //Demo Basic MIDI instruments, GM1
  //=================================================================
  talkMIDI(0xB0, 0, 0x00); //Default bank GM1

  //Change to different instrument
  
  talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command

  //keyboards
  for(int i = 0 ; i < 7 ; i++) {
    if (digitalRead(pinKey[i]) == HIGH && !booKeys[i]) {
      Serial.print("note:");
      Serial.print(keys[i]);
      Serial.println(noteKeys[i], DEC);

      noteOn(0, noteKeys[i], 127);
      booKeys[i] = true;
    } else if (digitalRead(pinKey[i]) == LOW) {
      noteOff(0, noteKeys[i], 127);
      booKeys[i] = false;
    }
  }


  talkMIDI(0xB0, 0, 0x78); //Bank select drums

  talkMIDI(0xC0, 30, 0); //Set instrument number. 0xC0 is a 1 data byte command

  for(int i = 0 ; i < 4 ; i++){
    if(digitalRead(pinDrum[i]) == HIGH && !booDrums[i]) {
      Serial.print("note:");
      
      Serial.println(noteDrums[i], DEC);

      noteOn(0, noteDrums[i], 127);
      booDrums[i] = true;
    } else if (digitalRead(pinDrum[i]) == LOW) {
      noteOff(0, noteDrums[i], 127);
      booDrums[i] = false;
    }
  }
}
