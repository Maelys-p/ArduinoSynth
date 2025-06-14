# ArduinoSynth
## What It Be
An arduino uno based 4-voice polyphony synth controlled by a keyboard. 
31.25 kHz sample rate, 16 instruments (one of which is just for fun, implying others are of much use).
Sound synthesized with ADSR and FM (cryptic words).

## Credits
Code adapted (shamelessly stolen) from https://www.instructables.com/Arduino-Synthesizer-With-FM/
All I did was change the controls from a bunch of buttons to a PS/2 keyboard (a bunch of buttons).
I guess I added some stuff for more convinient controlling and serial reporting. That code lacked that because of the inconvinient input method.  


## Keyboard Layout
The layout is very human and is very convinient for humans, that is, if you rip out half the keys.

Lower octave: Z S C F B N J ; L > : LSHIFT
              C C# D D# E F F# G G# A A# B

              
Upper octave: Q 2 E 4 T Y 7 I 9 P < ^
              C C# D D# E F F# G G# A A# B

You can switch instruments with the arrow keys (left and right), or by pressing some random binds on the numpad.

Up/Down Arrows transpose the thing.

Also of note is that there is a good chance that this only works this way with the particular old ass keyboard I have, because as I found out there seem to be different standards for the scancodes. 

## Hardware Required
As for hardware you only need the UNO, or a rough equivalent (which likely will not work, because this thing uses registers that might only work this way on atmega328, I'm pretty sure),
any speaker really, a ps/2 keyboard and a ps/2 input slot, as well as some way to connect four of it's important(operational) pins to the board. Honestly, why does that thing have so many holes to only use 4. 

## Build & Upload
1. Install PS2KeyAdvanced library
2. Upload the sketch
3. Pray to Gods

## Notes
The code is likely final and will not be maintained, the entire thing runs on precisely timed PWM cycles, so changing almost anything breaks everything, good luck o/
