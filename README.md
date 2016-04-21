# TinyAudio

This is a trial of an audio library for the TinyArcade (Arduino) platform

The idea is to be able to play various forms of audio together in a transparent way

- sampled stream from SD file
- sampled effects in C source code or memory data
- generated waves to play midi like notes

This is still at an early stage, but this sample it can play together an audio track saved as music.raw (unsigned 8bit 11025Hz) inside a Tinytris folder on the SD card
(some other somples are included)
(Audacity can export as 8bit unsigned PCM "Raw (headerless)" under "other uncompressed data", remove dithering in quality preferences to have a better sample)

At the same time it plays and loops a bouncing effect saved as C++ source code (brb.cpp/brb.h)

And, also at the same time, it also plays an ugly sounding Sawtooth Wave of a 440Hz (note A4)

This is very bad to hear, but it, well, sort of, works.

In the setup() in TinyAudio.ino you can attach sounds to the timeline

The timeline idea is nice, but it has problems, like that after 108,212.. hours it resets itself (max 32bit int/11025hz/60/60)

To do:

  - loop the timeline, give it a finite end, remembering to take care of sounds that start inside the timeline (before the loop end) but end outside
  - Handle play of instant sound effects, appending them at the playhead position and removing them automatically (handling two ore more instances of the same sound in parallel)
  - allow conditions like "if big file available play that otherwide play midi"
  - add midi like playing in form of a AudioItemSong class that takes notes as C++ data and returns the correct wave for the correct pitch of the current note
  
  
