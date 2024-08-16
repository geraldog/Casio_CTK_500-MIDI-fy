/*
 * Casio_CTK_500-MIDI-fy.ino
 *
 * Created: 22/7/2023 01:30:55 AM
 * Author: Geraldo Nascimento
 * Copyright 2023-2024
 */ 

#include <bitHelpers.h>

#include <MIDIUSB.h>
#include <MIDIUSB_Defs.h>
#include <frequencyToNote.h>
#include <pitchToFrequency.h>
#include <pitchToNote.h>


uint8_t baseNote = 24;
const int numRows = 11;
const int numCols = 6;

bool noteState[numRows][numCols] = { false };

unsigned long matrix_up[numRows][numCols] = { 0 };
unsigned long matrix_down[numRows][numCols] = { 0 };
unsigned long nowTime = 0;

uint8_t matrixReps[numRows][numCols] = { 0 };
uint8_t channelValue = 0;

uint16_t allKO = B1;
uint16_t pastKO = 0;

#define KO00 64   // PC6 is high
#define KO01 16   // PF4 is high
#define KO02 64   // PE6 is high
#define KO03 16   // PB4 is high
#define KO04 32   // PB5 is high
#define KO05 64   // PB6 is high
#define KO06 128  // PB7 is high
#define KO07 2    // PF1 is high
#define KO08 128  // PF7 is high
#define KO09 64   // PF6 is high
#define KO10 32   // PF5 is high

#define KI0 4   // PD2 is high
#define KI1 8   // PD3 is high
#define KI2 2   // PD1 is high
#define KI3 1   // PD0 is high
#define KI4 16  // PD4 is high
#define KI5 64  // PD6 is high

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = { 0x08, 0x80 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOff);
}


int calculateMidiNote(int kiIndex, int koState) {
  return baseNote + kiIndex + (koState * numCols);
}

void handleNoteOn(int kiIndex, int koState) {
  // Check if the note is already turned on
  if (noteState[koState][kiIndex] == true) {
    return;
    //Serial.println("Note already on");
  }

  else {
    int midiNote = calculateMidiNote(kiIndex, koState);
    if (midiNote >= 0 && midiNote <= 127) {
      Serial.write("NOTE ON ");
      Serial.println(midiNote);
      // Send the MIDI note-on event with velocity 127 through Channel 1
      noteOn(channelValue, midiNote, 127);
    }
    else
    {
      Serial.println("NOTE OUT OF RANGE");
    }
    // Mark the note as turned on in the array
    noteState[koState][kiIndex] = true;
  }
}

void handleNoteOff(int kiIndex, int koState) {
  // Check if the note is already turned on
  if (noteState[koState][kiIndex]) {
    int midiNote = calculateMidiNote(kiIndex, koState);
    if (midiNote >= 0 && midiNote <= 127) {
      Serial.write("NOTE OFF ");
      Serial.println(midiNote);
      // Send the MIDI note-off event with velocity 127
      noteOff(channelValue, midiNote, 127);
      // Mark the note as turned off in the array
      noteState[koState][kiIndex] = false;
    }
  }

  else {
    return;
  }  //Serial.println("Note already off"); }
}

void handleChannelOn(int kiIndex, int koState) {
  // Check if the note is already turned on
  if (noteState[koState][kiIndex] == true) {
    return;
    //Serial.println("Note already on");
  }

  else {
    if (channelValue >= 15) {
      channelValue = 255;
    }
    channelValue++;
    Serial.write("Switch to Channel ");
    Serial.println(channelValue + 1);
    // Mark the note as turned on in the array
    noteState[koState][kiIndex] = true;
  }
}

void handleChannelOff(int kiIndex, int koState) {
  if (noteState[koState][kiIndex]) {
    noteState[koState][kiIndex] = false;
  }
  else {
    return;
  }
}

void handleBaseOctaveOn(int kiIndex, int koState) {
  if (noteState[koState][kiIndex]) {
    return;
  }
  else {
    if (baseNote >= 120)
    {
      baseNote = 244;
    }
    // Check if the note is already turned on
    baseNote += 12;
    Serial.write("Switch Base Octave to MIDI note ");
    Serial.println(baseNote);
    noteState[koState][kiIndex] = true;
  }
}

void handleBaseOctaveOff(int kiIndex, int koState) {
  if (noteState[koState][kiIndex]) {
    noteState[koState][kiIndex] = false;
  }
  else {
    return;
  }
}

void KIagainstKO_up(uint8_t kiIndex, uint8_t koState, uint16_t frequency, uint8_t ceil) {
  unsigned long lastTime = matrix_up[koState][kiIndex];

  if (lastTime == 0) {
    matrix_up[koState][kiIndex] = nowTime;
  }

  if ((nowTime - lastTime) > frequency) {
    debugKI_KO(kiIndex, koState, frequency, lastTime, nowTime, true, false);
    matrix_up[koState][kiIndex] = nowTime;

  }

  else if ((nowTime - lastTime) <= frequency) {
    debugKI_KO(kiIndex, koState, frequency, lastTime, nowTime, true, true);
    matrix_up[koState][kiIndex] = nowTime;

    if (matrixReps[koState][kiIndex] < ceil) { matrixReps[koState][kiIndex]++; }

    if (matrixReps[koState][kiIndex] == ceil && (kiIndex == 0 || koState != 10)) {
      handleNoteOn(kiIndex, koState);
    }
    else if (matrixReps[koState][kiIndex] == ceil && (kiIndex == 4 && koState == 10)) {
      handleChannelOn(kiIndex, koState);
    }
    else if (matrixReps[koState][kiIndex] == ceil && (kiIndex == 5 && koState == 10)) {
      handleBaseOctaveOn(kiIndex, koState);
    }
  }
}

void KIagainstKO_down(uint8_t kiIndex, uint8_t koState, uint8_t frequency) {
  unsigned long lastTime = matrix_down[koState][kiIndex];

  if (lastTime == 0) {
    matrix_down[koState][kiIndex] = nowTime;
  }

  if ((nowTime - lastTime) > frequency) {
    debugKI_KO(kiIndex, koState, frequency, lastTime, nowTime, false, false);
    matrix_down[koState][kiIndex] = nowTime;
  }

  else if ((nowTime - lastTime) <= frequency) {
    debugKI_KO(kiIndex, koState, frequency, lastTime, nowTime, false, true);
    matrix_down[koState][kiIndex] = nowTime;

    if (matrixReps[koState][kiIndex] > 0) { matrixReps[koState][kiIndex]--; }

    if (matrixReps[koState][kiIndex] == 0 && (kiIndex == 0 || koState != 10)) {
      handleNoteOff(kiIndex, koState);
    }
    else if (matrixReps[koState][kiIndex] == 0 && (kiIndex == 4 && koState == 10)) {
      handleChannelOff(kiIndex, koState);
    }
    else if (matrixReps[koState][kiIndex] == 0 && (kiIndex == 5 && koState == 10)) {
      handleBaseOctaveOff(kiIndex, koState);
    }
  }
}

void debugKI_KO(uint8_t kiIndex, uint8_t koState, uint16_t frequency, unsigned long lastTime, unsigned long nowTime, bool ON_OFF, bool lessOrMore) {
  // Any value above 10 for koState will disable debug printout
  if (koState == 11) {
    if (ON_OFF) {
      Serial.print("ON");
    }
    else {
      Serial.print("OFF");
    }

    Serial.print(" | ");

    if (lessOrMore) {
      Serial.print("<");
    }
    else {
      Serial.print(">");
    }

    Serial.print(frequency);
    Serial.print("its | KI");
    Serial.print(kiIndex);
    Serial.print(" | KO");
    Serial.print(koState);
    Serial.print(" | lastTime | ");
    Serial.print(lastTime);
    Serial.print(" | nowTime | ");
    Serial.print(nowTime);
    Serial.print(" | REPS ");
    Serial.println(matrixReps[koState][kiIndex]);
  }
}

void setup() {
  Serial.end();
  // KI key signal
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(12, INPUT);

  // KO scan signal
  pinMode(5, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);

  return;
}

void loop() {

  unsigned long time = millis();
  if (nowTime == time)
    return;

  // Check if KO00 is scanning
  if (allKO == B1) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 0, 12, 2);
    }
    else {
      KIagainstKO_down(0, 0, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 0, 12, 2);
    }
    else {
      KIagainstKO_down(1, 0, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 0, 12, 2);
    }
    else {
      KIagainstKO_down(2, 0, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 0, 12, 2);
    }
    else {
      KIagainstKO_down(3, 0, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 0, 12, 2);
    }
    else {
      KIagainstKO_down(4, 0, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 0, 12, 2);
    }
    else {
      KIagainstKO_down(5, 0, 12);
    }
  }
  // Check if KO01 is scanning
  else if ((allKO & B10) == B10) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 1, 12, 2);
    }
    else {
      KIagainstKO_down(0, 1, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 1, 12, 2);
    }
    else {
      KIagainstKO_down(1, 1, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 1, 12, 2);
    }
    else {
      KIagainstKO_down(2, 1, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 1, 12, 2);
    }
    else {
      KIagainstKO_down(3, 1, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 1, 12, 2);
    }
    else {
      KIagainstKO_down(4, 1, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 1, 12, 2);
    }
    else {
      KIagainstKO_down(5, 1, 12);
    }
  }
  // Check if KO02 is scanning
  else if ((allKO & B100) == B100) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 2, 12, 2);
    }
    else {
      KIagainstKO_down(0, 2, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 2, 12, 2);
    }
    else {
      KIagainstKO_down(1, 2, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 2, 12, 2);
    }
    else {
      KIagainstKO_down(2, 2, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 2, 12, 2);
    }
    else {
      KIagainstKO_down(3, 2, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 2, 12, 2);
    }
    else {
      KIagainstKO_down(4, 2, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 2, 12, 2);
    }
    else {
      KIagainstKO_down(5, 2, 12);
    }
  }
  // Check if KO03 is scanning
  else if ((allKO & B1000) == B1000) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 3, 12, 2);
    }
    else {
      KIagainstKO_down(0, 3, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 3, 12, 2);
    }
    else {
      KIagainstKO_down(1, 3, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 3, 12, 2);
    }
    else {
      KIagainstKO_down(2, 3, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 3, 12, 2);
    }
    else {
      KIagainstKO_down(3, 3, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 3, 12, 2);
    }
    else {
      KIagainstKO_down(4, 3, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 3, 12, 2);
    }
    else {
      KIagainstKO_down(5, 3, 12);
    }
  }
  // Check if KO04 is scanning
  else if ((allKO & B10000) == B10000) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 4, 12, 2);
    }
    else {
      KIagainstKO_down(0, 4, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 4, 12, 2);
    }
    else {
      KIagainstKO_down(1, 4, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 4, 12, 2);
    }
    else {
      KIagainstKO_down(2, 4, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 4, 12, 2);
    }
    else {
      KIagainstKO_down(3, 4, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 4, 12, 2);
    }
    else {
      KIagainstKO_down(4, 4, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 4, 12, 2);
    }
    else {
      KIagainstKO_down(5, 4, 12);
    }
  }
  // Check if KO05 is scanning
  else if ((allKO & B100000) == B100000) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 5, 12, 2);
    }
    else {
      KIagainstKO_down(0, 5, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 5, 12, 2);
    }
    else {
      KIagainstKO_down(1, 5, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 5, 12, 2);
    }
    else {
      KIagainstKO_down(2, 5, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 5, 12, 2);
    }
    else {
      KIagainstKO_down(3, 5, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 5, 12, 2);
    }
    else {
      KIagainstKO_down(4, 5, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 5, 12, 2);
    }
    else {
      KIagainstKO_down(5, 5, 12);
    }
  }
  // Check if KO06 is scanning
  else if ((allKO & B1000000) == B1000000) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 6, 12, 2);
    }
    else {
      KIagainstKO_down(0, 6, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 6, 12, 2);
    }
    else {
      KIagainstKO_down(1, 6, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 6, 12, 2);
    }
    else {
      KIagainstKO_down(2, 6, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 6, 12, 2);
    }
    else {
      KIagainstKO_down(3, 6, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 6, 12, 2);
    }
    else {
      KIagainstKO_down(4, 6, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 6, 12, 2);
    }
    else {
      KIagainstKO_down(5, 6, 12);
    }
  }
  // Check if KO07 is scanning
  else if ((allKO & B10000000) == B10000000) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 7, 12, 2);
    }
    else {
      KIagainstKO_down(0, 7, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 7, 12, 2);
    }
    else {
      KIagainstKO_down(1, 7, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 7, 12, 2);
    }
    else {
      KIagainstKO_down(2, 7, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 7, 12, 2);
    }
    else {
      KIagainstKO_down(3, 7, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 7, 12, 2);
    }
    else {
      KIagainstKO_down(4, 7, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 7, 12, 2);
    }
    else {
      KIagainstKO_down(5, 7, 12);
    }
  }
  // Check if KO08 is scanning
  else if ((allKO & 0b100000000) == 0b100000000) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 8, 12, 2);
    }
    else {
      KIagainstKO_down(0, 8, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 8, 12, 2);
    }
    else {
      KIagainstKO_down(1, 8, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 8, 12, 2);
    }
    else {
      KIagainstKO_down(2, 8, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 8, 12, 2);
    }
    else {
      KIagainstKO_down(3, 8, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 8, 12, 2);
    }
    else {
      KIagainstKO_down(4, 8, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 8, 12, 2);
    }
    else {
      KIagainstKO_down(5, 8, 12);
    }
  }
  // Check if KO09 is scanning
  else if ((allKO & 0b1000000000) == 0b1000000000) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 9, 12, 2);
    }
    else {
      KIagainstKO_down(0, 9, 12);
    }

    if ((PIND & KI1) == KI1) {
      KIagainstKO_up(1, 9, 12, 2);
    }
    else {
      KIagainstKO_down(1, 9, 12);
    }

    if ((PIND & KI2) == KI2) {
      KIagainstKO_up(2, 9, 12, 2);
    }
    else {
      KIagainstKO_down(2, 9, 12);
    }

    if ((PIND & KI3) == KI3) {
      KIagainstKO_up(3, 9, 12, 2);
    }
    else {
      KIagainstKO_down(3, 9, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 9, 12, 2);
    }
    else {
      KIagainstKO_down(4, 9, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 9, 12, 2);
    }
    else {
      KIagainstKO_down(5, 9, 12);
    }
  }
  // Check if KO10 is scanning
  else if ((allKO & 0b10000000000) == 0b10000000000) {
    if ((PIND & KI0) == KI0) {
      KIagainstKO_up(0, 10, 12, 2);
    }
    else {
      KIagainstKO_down(0, 10, 12);
    }

    if ((PIND & KI4) == KI4) {
      KIagainstKO_up(4, 10, 12, 2);
    }
    else {
      KIagainstKO_down(4, 10, 12);
    }

    if ((PIND & KI5) == KI5) {
      KIagainstKO_up(5, 10, 12, 2);
    }
    else {
      KIagainstKO_down(5, 10, 12);
    }
  }

flush_and_exit:
  MidiUSB.flush();  // send any pending events
  pastKO = allKO;

  if (allKO != 0b10000000000) {
    allKO = allKO << 1;
  }
  else {
    allKO = B1;
  }
  // We are ready for one whole iteration of the loop
  // Increase the frame count for comparison in
  // KIagainstKO_ functions
  nowTime = millis();

  PORTC &= ~KO00;
  PORTF &= ~KO01;
  PORTE &= ~KO02;
  PORTB &= ~KO03;
  PORTB &= ~KO04;
  PORTB &= ~KO05;
  PORTB &= ~KO06;
  PORTF &= ~KO07;
  PORTF &= ~KO08;
  PORTF &= ~KO09;
  PORTF &= ~KO10;

  if (allKO == B1)
    PORTC |= KO00;
  if (allKO == B10)
    PORTF |= KO01;
  if (allKO == B100)
    PORTE |= KO02;
  if (allKO == B1000)
    PORTB |= KO03;
  if (allKO == B10000)
    PORTB |= KO04;
  if (allKO == B100000)
    PORTB |= KO05;
  if (allKO == B1000000)
    PORTB |= KO06;
  if (allKO == B10000000)
    PORTF |= KO07;
  if (allKO == 0b100000000)
    PORTF |= KO08;
  if (allKO == 0b1000000000)
    PORTF |= KO09;
  if (allKO == 0b10000000000)
    PORTF |= KO10;
  return;
}
