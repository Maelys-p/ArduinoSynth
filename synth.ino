

  #include <PS2KeyAdvanced.h>
  #include <avr/pgmspace.h>

  #define PS2_DATA_PIN  3
  #define PS2_CLOCK_PIN 2
  #define AUDIO_PIN     9

  PS2KeyAdvanced keyboard;

  #define INSTRUMENT_COUNT 16
  #define VOICE_COUNT 4

  const char instrumentNames[INSTRUMENT_COUNT][9] PROGMEM = {
    "Piano   ", "Xylophon", "Guitar  ", "Cymbal  ", 
    "Bell    ", "Funky   ", "Vibrato ", "Metal   ",
    "Violin  ", "Bass    ", "Trumpet ", "Harmonic",
    "WubWub  ", "Alien   ", "Whistle ", "Chaos   "
  };

  unsigned int loudness[INSTRUMENT_COUNT]     = {64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  80,  72,  48,  96};
  unsigned int pitchOffset[INSTRUMENT_COUNT]  = {12,  12,  12,  12,  24,  24,   0,  12,  24,  12,  12,  24,   0,  12,  36,  12};
  unsigned int attackRate[INSTRUMENT_COUNT]   = {4096,8192,8192,8192,4096, 512, 512,8192, 128, 128, 256, 256,2048, 512,1024,8192};
  unsigned int decayRate[INSTRUMENT_COUNT]    = {   8,  32,  16,  16,   8,  16,  16,   8,  16,  16,  64,  32,  12,  24,   8,  64};
  unsigned int sustainLevel[INSTRUMENT_COUNT] = {   0,   0,   0,   0,   0,   0,   0,   0, 240, 240, 192, 192,  64,   0, 128,   0};
  unsigned int releaseRate[INSTRUMENT_COUNT]  = {  64, 128,  32,  32,  16,  32,  32,  32,  32,  32,  64,  64,  48,  16,  64,   8};
  unsigned int fmFreqRatio[INSTRUMENT_COUNT]  = { 256, 512, 768, 400, 200,  96, 528, 244, 256, 128,  64, 160, 384, 147, 256, 666};
  unsigned int fmAmplStart[INSTRUMENT_COUNT]  = { 128, 512, 512,1024, 512,   0,1024,2048, 256, 256, 384, 256, 768,1536,  64,2048};
  unsigned int fmAmplEnd[INSTRUMENT_COUNT]    = {  64,   0, 128, 128, 128, 512, 768, 512, 128, 128, 256, 128, 256,  64,  32,1024};
  unsigned int fmDecayRate[INSTRUMENT_COUNT]  = {  64, 128, 128, 128,  32, 128, 128, 128, 128, 128,  64,  64, 200,  96,  32, 255};

  const byte upperOctaveScanCodes[14] = {
    0x5C,  // Additional C4 (same as Q)
    0x51, // Q  = C4
    0x32, // 2  = C#4
    0x45, // E  = D4
    0x34, // 4  = D#4
    0x54, // T  = E4
    0x59, // Y  = F4
    0x37, // 7  = F#4
    0x49, // I  = G4
    0x39, // 9  = G#4
    0x50, // P  = A4
    0x3C, // <  = A#4
    0x5E, // ^  = B4
    0x1E  // \ = C5 (high C, octave up)
  };

  const byte lowerOctaveScanCodes[12] = {
    0x5A, // Z  = C
    0x53, // S  = C#
    0x43, // C  = D
    0x46, // F  = D#
    0x42, // B  = E
    0x4E, // N  = F
    0x4A, // J  = F#
    0x3B, // ;  = G
    0x4C, // L  = G#
    0x3E, // >  = A
    0x3A, // :  = A#
    0x07  // LSHIFT = B
  };

  const byte instrumentScanCodes[16] = {
    0x19, 
    0x1A, 
    0x2B, 
    0x12, 
    0x18, 
    0x14, 
    0x15, 
    0x16,
    0x11,
    0x17, 
    0x13, 
    0x2C, 
    0x2F, 
    0x2E, 
    0x2D, 
    0x00  
  };

  #define SCAN_LEFT_ARROW  0x15
  #define SCAN_RIGHT_ARROW 0x16
  #define SCAN_UP_ARROW    0x17
  #define SCAN_DOWN_ARROW  0x18


  byte currentInstrument = 0;
  int transposeOffset = 0;  
  char sineTable[256];
  unsigned int toneIncrement[48];

  unsigned int phase[VOICE_COUNT] = {0, 0, 0, 0};
  int increment[VOICE_COUNT] = {0, 0, 0, 0};
  byte amplitude[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int fmPhase[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int fmIncrement[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int fmAmplitude[VOICE_COUNT] = {0, 0, 0, 0};

  byte envelopeStage[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int envelopeLevel[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int adsrAttack[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int adsrDecay[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int adsrSustain[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int adsrRelease[VOICE_COUNT] = {0, 0, 0, 0};
  byte baseAmplitude[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int baseIncrement[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int fmStartAmpl[VOICE_COUNT] = {0, 0, 0, 0};
  int fmDeltaAmpl[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int fmBaseIncrement[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int fmDecayParam[VOICE_COUNT] = {0, 0, 0, 0};
  unsigned int fmExponential[VOICE_COUNT] = {0, 0, 0, 0};
  byte keyOfVoice[VOICE_COUNT] = {255, 255, 255, 255};
  unsigned int voiceAge[VOICE_COUNT] = {0, 0, 0, 0};

  bool keyStates[25] = {false};  
  ещт
  void InitSineTable() {
    for (int i = 0; i < 256; i++) {
      sineTable[i] = (sin(2 * 3.14159265 * (i + 0.5) / 256)) * 128;
    }
  }

  void InitToneTable() {
    for (byte i = 0; i < 48; i++) {
      toneIncrement[i] = 440.0 * pow(2.0, ((i - 21) / 12.0)) * 65536.0 / (16000000.0 / 512) + 0.5;
    }
  }

  void PrintInstrumentInfo() {
    Serial.print(F("\nInstrument "));
    if (currentInstrument < 10) Serial.print('0');
    Serial.print(currentInstrument);
    Serial.print(F(": "));
    
    char name[9];
    for (int i = 0; i < 8; i++) {
      name[i] = pgm_read_byte(&instrumentNames[currentInstrument][i]);
    }
    name[8] = '\0';
    Serial.println(name);
    
    Serial.print(F("  FM Ratio: "));
    Serial.print(fmFreqRatio[currentInstrument] / 256.0, 1);
    Serial.print(F(", FM Depth: "));
    Serial.print(fmAmplStart[currentInstrument]);
    Serial.print(F("->"));
    Serial.println(fmAmplEnd[currentInstrument]);
    
    Serial.print(F("  ADSR: A="));
    Serial.print(attackRate[currentInstrument]);
    Serial.print(F(" D="));
    Serial.print(decayRate[currentInstrument]);
    Serial.print(F(" S="));
    Serial.print(sustainLevel[currentInstrument]);
    Serial.print(F(" R="));
    Serial.println(releaseRate[currentInstrument]);
  }

  void setup() {
    Serial.begin(115200);
    Serial.println(F("\nFM Synthesizer v2.0"));
    Serial.println(F("===================="));
    
    keyboard.begin(PS2_DATA_PIN, PS2_CLOCK_PIN);
    keyboard.setNoBreak(0);     
    keyboard.setNoRepeat(1);    
    
    noInterrupts();
    
    InitSineTable();
    InitToneTable();
    
    // Setup PWM on pin 9
    pinMode(AUDIO_PIN, OUTPUT);
    TCCR1A = 0B10000010;  // 9-bit fast PWM
    TCCR1B = 0B00001001;
    
    interrupts();
    
    Serial.println(F("\nKeyboard Layout:"));
    Serial.println(F("Lower: Z S C F B N J ; L > : LSHIFT (C3-B3)"));
    Serial.println(F("Upper: Q 2 E 4 T Y 7 I 9 P < ^ (C4-B4)"));
    Serial.println(F("Extra: 0x1E = C4 (same as Q), \\ = C5"));
    Serial.println(F("Left/Right: Change instrument"));
    Serial.println(F("Up/Down: Transpose +/- semitone"));
    Serial.println(F("Direct select: Various keys for instruments 0-14"));
    
    PrintInstrumentInfo();
  }

  inline void UpdatePWM() __attribute__((always_inline));
  inline void UpdatePWM() {
    while ((TIFR1 & 0B00000001) == 0);
    
    TIFR1 |= 0B00000001;
    
    fmPhase[0] += fmIncrement[0];
    fmPhase[1] += fmIncrement[1];
    fmPhase[2] += fmIncrement[2];
    fmPhase[3] += fmIncrement[3];
    
    phase[0] += increment[0];
    phase[1] += increment[1];
    phase[2] += increment[2];
    phase[3] += increment[3];
    
    int val = sineTable[(phase[0] + sineTable[fmPhase[0] >> 8] * fmAmplitude[0]) >> 8] * amplitude[0];
    val += sineTable[(phase[1] + sineTable[fmPhase[1] >> 8] * fmAmplitude[1]) >> 8] * amplitude[1];
    val += sineTable[(phase[2] + sineTable[fmPhase[2] >> 8] * fmAmplitude[2]) >> 8] * amplitude[2];
    val += sineTable[(phase[3] + sineTable[fmPhase[3] >> 8] * fmAmplitude[3]) >> 8] * amplitude[3];
    
    OCR1A = val / 128 + 256;
  }

  void ProcessKeyboard() {
    static uint32_t lastScan = 0;
    
    if (millis() - lastScan < 2) return;
    lastScan = millis();
    
    if (!keyboard.available()) return;
    
    uint16_t code = keyboard.read();
    if (code == 0) return;
    
    bool isBreak = (code & PS2_BREAK) != 0;
    uint8_t scanCode = code & 0xFF;
    
    if (scanCode == SCAN_LEFT_ARROW && !isBreak) {
      if (currentInstrument == 0) 
        currentInstrument = INSTRUMENT_COUNT - 1;
      else 
        currentInstrument--;
      PrintInstrumentInfo();
      return;
    }
    else if (scanCode == SCAN_RIGHT_ARROW && !isBreak) {
      currentInstrument++;
      if (currentInstrument >= INSTRUMENT_COUNT) 
        currentInstrument = 0;
      PrintInstrumentInfo();
      return;
    }
    else if (scanCode == SCAN_UP_ARROW && !isBreak) {
      if (transposeOffset < 12) {
        transposeOffset++;
        Serial.print(F("\nTranspose: "));
        if (transposeOffset > 0) Serial.print(F("+"));
        Serial.print(transposeOffset);
        Serial.println(F(" semitones"));
      }
      return;
    }
    else if (scanCode == SCAN_DOWN_ARROW && !isBreak) {
      if (transposeOffset > -12) {
        transposeOffset--;
        Serial.print(F("\nTranspose: "));
        if (transposeOffset > 0) Serial.print(F("+"));
        Serial.print(transposeOffset);
        Serial.println(F(" semitones"));
      }
      return;
    }
    
    for (byte i = 0; i < 15; i++) {  
      if (scanCode == instrumentScanCodes[i] && !isBreak) {
        currentInstrument = i;
        PrintInstrumentInfo();
        return;
      }
    }
    
    for (byte i = 0; i < 14; i++) {
      if (scanCode == upperOctaveScanCodes[i]) {
        byte note;
        if (i == 0 || i == 1) {
          note = 12; 
        } else if (i < 13) {
          note = i + 11;  
        } else {
          note = 24;  
        }
        
        if (!isBreak && !keyStates[note]) {
          keyStates[note] = true;
          StartNote(note);
        } else if (isBreak && keyStates[note]) {
          keyStates[note] = false;
          StopNote(note);
        }
        return;
      }
    }
    
    for (byte i = 0; i < 12; i++) {
      if (scanCode == lowerOctaveScanCodes[i]) {
        byte note = i;
        if (!isBreak && !keyStates[note]) {
          keyStates[note] = true;
          StartNote(note);
        } else if (isBreak && keyStates[note]) {
          keyStates[note] = false;
          StopNote(note);
        }
        return;
      }
    }
  }

  void StartNote(byte note) {
    int transposedNote = (int)note + transposeOffset;
    
    if (transposedNote < 0 || transposedNote >= 48) {
      return;  
    }
    
    byte channel = 255;
    
    for (byte i = 0; i < VOICE_COUNT; i++) {
      if (envelopeStage[i] == 0) {
        channel = i;
        break;
      }
    }
    
    if (channel == 255) {
      channel = 0;
      for (byte i = 1; i < VOICE_COUNT; i++) {
        if (voiceAge[i] > voiceAge[channel]) channel = i;
      }
    }
    
    phase[channel] = 0;
    baseAmplitude[channel] = loudness[currentInstrument];
    baseIncrement[channel] = toneIncrement[pitchOffset[currentInstrument] + transposedNote];
    adsrAttack[channel] = attackRate[currentInstrument];
    adsrDecay[channel] = decayRate[currentInstrument];
    adsrSustain[channel] = sustainLevel[currentInstrument] << 8;
    adsrRelease[channel] = releaseRate[currentInstrument];
    envelopeStage[channel] = 1;
    fmPhase[channel] = 0;
    fmBaseIncrement[channel] = ((long)baseIncrement[channel] * fmFreqRatio[currentInstrument]) / 256;
    fmStartAmpl[channel] = fmAmplEnd[currentInstrument];
    fmDeltaAmpl[channel] = fmAmplStart[currentInstrument] - fmAmplEnd[currentInstrument];
    fmExponential[channel] = 0xFFFF;
    fmDecayParam[channel] = fmDecayRate[currentInstrument];
    keyOfVoice[channel] = note;  
    voiceAge[channel] = 0;
  }

  void StopNote(byte note) {
    for (byte i = 0; i < VOICE_COUNT; i++) {
      if (keyOfVoice[i] == note && envelopeStage[i] > 0 && envelopeStage[i] < 4) {
        envelopeStage[i] = 4; 
        return;
      }
    }
  }

  void loop() {
    ProcessKeyboard();
    
    UpdatePWM();  // #1
    UpdatePWM();  // #2
    UpdatePWM();  // #3
    UpdatePWM();  // #4
    UpdatePWM();  // #5
    
    fmExponential[0] -= (long)fmExponential[0] * fmDecayParam[0] >> 16;
    fmExponential[1] -= (long)fmExponential[1] * fmDecayParam[1] >> 16;
    fmExponential[2] -= (long)fmExponential[2] * fmDecayParam[2] >> 16;
    fmExponential[3] -= (long)fmExponential[3] * fmDecayParam[3] >> 16;
    
    UpdatePWM();  // #6
    
    for (byte i = 0; i < VOICE_COUNT; i++) {
      if (envelopeStage[i] == 4) {  
        if (envelopeLevel[i] <= adsrRelease[i]) {
          envelopeLevel[i] = 0;
          envelopeStage[i] = 0;
          keyOfVoice[i] = 255;
        }
        else envelopeLevel[i] -= adsrRelease[i];
      }
      if (envelopeStage[i] == 2) {  
        if (envelopeLevel[i] <= (adsrSustain[i] + adsrDecay[i])) {
          envelopeLevel[i] = adsrSustain[i];
          envelopeStage[i] = 3;
        }
        else envelopeLevel[i] -= adsrDecay[i];
      }
      if (envelopeStage[i] == 1) {  
        if ((0xFFFF - envelopeLevel[i]) <= adsrAttack[i]) {
          envelopeLevel[i] = 0xFFFF;
          envelopeStage[i] = 2;
        }
        else envelopeLevel[i] += adsrAttack[i];
      }
      voiceAge[i]++;
      UpdatePWM();  // #7-10
    }
    
    amplitude[0] = (baseAmplitude[0] * (envelopeLevel[0] >> 8)) >> 8;
    increment[0] = baseIncrement[0];
    fmAmplitude[0] = fmStartAmpl[0] + ((long)fmDeltaAmpl[0] * fmExponential[0] >> 16);
    fmIncrement[0] = fmBaseIncrement[0];
    UpdatePWM();  // #11
    
    amplitude[1] = (baseAmplitude[1] * (envelopeLevel[1] >> 8)) >> 8;
    increment[1] = baseIncrement[1];
    fmAmplitude[1] = fmStartAmpl[1] + ((long)fmDeltaAmpl[1] * fmExponential[1] >> 16);
    fmIncrement[1] = fmBaseIncrement[1];
    UpdatePWM();  // #12
    
    amplitude[2] = (baseAmplitude[2] * (envelopeLevel[2] >> 8)) >> 8;
    increment[2] = baseIncrement[2];
    fmAmplitude[2] = fmStartAmpl[2] + ((long)fmDeltaAmpl[2] * fmExponential[2] >> 16);
    fmIncrement[2] = fmBaseIncrement[2];
    UpdatePWM();  // #13
    
    amplitude[3] = (baseAmplitude[3] * (envelopeLevel[3] >> 8)) >> 8;
    increment[3] = baseIncrement[3];
    fmAmplitude[3] = fmStartAmpl[3] + ((long)fmDeltaAmpl[3] * fmExponential[3] >> 16);
    fmIncrement[3] = fmBaseIncrement[3];
    UpdatePWM();  // #14
    
    UpdatePWM();  // #15
  }