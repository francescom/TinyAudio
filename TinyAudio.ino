

#include <stdint.h>

#define nullPtr 0

#include <SdFat.h>
#include <SPI.h>
#include <Wire.h>


#include "TinyAudio.h"

#include "brb.h"


SdFat sd;





uint16_t waveGeneratorTest(uint32_t frame) {
  return frame%20;
}




AudioItemRawData8bit testSoundEffect; // (0,(unsigned char*)brb,4412);
AudioItemFile testStream; // (0,(char*)"/TinyTris/music.raw");

AudioItemWave testSoundWave; // (0,waveGeneratorTest);
AudioItemNote testSoundNote; // (0,waveGeneratorTest);




MainAudioTimeline audioTimeline;




void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  SPI.begin();

  delay(3);
  
  SerialUSB.println("Setup.");
  if (!sd.begin(10, SPI_FULL_SPEED)) {
    SerialUSB.println("Card failed");
  //  while (1);
  }
  SPI.setClockDivider(0);

  //audio output config
  analogWrite(A0, 0);
  tcConfigure(11025);

  testSoundEffect.init(0,(unsigned char*)brb,4412);
  testSoundEffect.loop=true;

  testStream.init(0,(char*)"/TinyTris/music.raw");
  testStream.loop=true;
  testSoundWave.init(0,waveGeneratorTest);
  testSoundNote.init(0,playSineFreq);
  testSoundNote.frequency=4400;


  testStream.volumePerc=90;
  testSoundNote.volumePerc=10;
  
  // audioTimeline.loop=true;
  // audioTimeline.length=testStreamlength;
  SerialUSB.println("Ready to start");
  
  audioTimeline.addItem((AudioItem*)&testStream);
  audioTimeline.addItem((AudioItem*)&testSoundEffect);
  // audioTimeline.addItem((AudioItem*)&testSoundWave);
  audioTimeline.addItem((AudioItem*)&testSoundNote);
  
  SerialUSB.println("Ready to start");
  audioTimeline.start();
  SerialUSB.println("Setup done");

  tcStartCounter();
}

void loop() {
  // SerialUSB.print(audioTimeline.lastSampleIndex);
  // SerialUSB.println(frequencyFromNote(-9));
  audioTimeline.doLoop();
  // SerialUSB.println(audioTimeline.saveSampleIndex);
}




