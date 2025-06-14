# ArduinoSynth
An arduino uno based 4-voice polyphony synth controlled by a keyboard. 
31.25 kHz sample rate, 16 instruments (one of which is just for fun, implying others are of much use).
Sound synthesized with ADSR and FM (cryptic words).

Code adapted (shamelessly stolen) from https://www.instructables.com/Arduino-Synthesizer-With-FM/
All I did was change the controls from a bunch of buttons to a PS/2 keyboard (a bunch of buttons).
I guess I added some stuff for controlling. 

The layout is very human and is very convinient for humans, that is, if you rip out half the keys.

As for hardware you only need the UNO, or a rough equivalent (which likely will not work, because this thing uses registers that might only work this way on atmega328, I'm pretty sure),
, any speaker really, a ps/2 keyboard and a ps/2 input slot, as well as some way to coonnect four of it's important(operational) pins to the board. Honestly, why does that thing have so many holes to only use 4. 

The code is likely final and will not be maintained, good luck forcing it to work ig, bb o/
