

#include <stdint.h>

#define nullPtr 0

#include <SdFat.h>
#include <SPI.h>
#include <Wire.h>


#include "tcaudio.h"
#include "AudioItem.h"
#include "LoopingBuffer.h"
#include "AudioItemFile.h"

#include "brb.h"


SdFat sd;





int16_t playFreq(int f,uint8_t a,uint32_t x) { // use table lookup and interpolation for sin instead
int fq=11025/f;
int prop=256/fq;
  return 256+(x%fq)*prop;
  return (int16_t)( 256+round(256*sin((double)x*f/11025.0*2*PI)*a/100) );
}














class AudioItemRawData8bit: public AudioItem {
   public:
   unsigned char *data;
   
   AudioItemRawData8bit() : AudioItem() {}
   AudioItemRawData8bit(uint32_t myPos,unsigned char *mydata,uint32_t myLen) : AudioItem(myPos) {
    init(myPos,mydata,myLen);
   }
   void init(uint32_t myPos,unsigned char *mydata,uint32_t myLen) {
    AudioItem::init(myPos);
    data=mydata;
    length=myLen;
  }
  
  uint16_t getSampleAt(uint32_t pos) {
    if(!enabled) return 0;

    soundHeadPos=pos;
    uint32_t oldLoopPos=loopPos;
    loopPos=pos%length;
    
    if(soundHeadPos>=length) {
      if(loop) {
        if(loopPos!=oldLoopPos+1) {
          onLoop();
        }
      } else {
        onEnded();
        stop();
        return 0;
      }
    }
    return data[pos%length];
   }
};



typedef uint16_t (*waveGeneratorFunction)(uint32_t frame);

class AudioItemWaveFunction: public AudioItem {
  public:
  waveGeneratorFunction generator=nullPtr;
  
  AudioItemWaveFunction() : AudioItem() {
  }
  AudioItemWaveFunction(uint32_t myPos,waveGeneratorFunction myFunc) : AudioItem() {
    init(myPos,myFunc);
  }
  void init(uint32_t myPos,waveGeneratorFunction myFunc) {
    AudioItem::init(myPos);
    generator=myFunc;
  }
  
  uint16_t getSampleAt(uint32_t pos) {
    if(!enabled) return 0;
    if(generator!=nullPtr) return generator(pos);
  }
};

uint16_t waveGeneratorTest(uint32_t frame) {
  return frame%20;
}


AudioItemRawData8bit testSoundEffect; // (0,(unsigned char*)brb,4412);
AudioItemFile testStream; // (0,(char*)"/TinyTris/music.raw");

AudioItemWaveFunction testSoundWave; // (0,waveGeneratorTest);





const unsigned int MAX_ITEMS=100;

class AudioTimeline: public AudioItem { // a timeline is really an audio item (could be nested)
  public:
/* inherits  
  boolean loop=false;
  boolean enabled=true;
  boolean error=false;
  boolean errorTxt="";
*/
  AudioItem* allItems[MAX_ITEMS];
  unsigned int channels=0;

  AudioTimeline() : AudioItem(0) {
    clear();
  }
  AudioTimeline(uint32_t myPos) : AudioItem(myPos) {
    clear();
  }
  
  void clear() {
    for(unsigned char i=0;i<channels;i++) allItems[i]=nullPtr;
    channels=0;
  }

  int addItem(AudioItem * theItem) {
    if(channels>=MAX_ITEMS) {
       error=true;
       errorTxt="Head out of buffer (did you forget to load?)";
       SerialUSB.print("X");
       return -1;
    }
    allItems[channels++]=theItem;
    SerialUSB.print("+");
    SerialUSB.println(channels);
    return channels;
  }

  void removeItem(AudioItem * theItem) {
    uint_fast8_t move=0;
    for(uint_fast8_t i=0;i<channels;i++) {
      if(move) allItems[i-move]=allItems[i];
      if(allItems[i]==theItem) {
        allItems[i]=nullPtr;
        move++;
      }
    }
    channels-=move;
  }

    void dispose() { // not a distructor, just useless at the moment
      for(unsigned char i=0;i<channels;i++) {
        if(allItems[MAX_ITEMS]!=nullPtr) allItems[i]->dispose();
      }
    }
    uint16_t getSampleAt(uint32_t pos) {

      // return testSoundEffect.getSampleAt(pos);
      // return playFreq(110,50,pos);
      
      // return playFreq(880,50,pos);
      // return testStream.getSampleAt(pos);

      uint16_t sample=0;
      for(unsigned char i=0;i<channels;i++) {
        if(allItems[i]->enabled && !allItems[i]->error) sample+=allItems[i]->getSampleAt(pos);
      }
      return sample;
    }
    uint16_t getNextSample() {
      return 0;
      
      uint16_t sample=0;
      for(unsigned char i=0;i<channels;i++) {
        if(allItems[i]->enabled && !allItems[i]->error) sample+=allItems[i]->getNextSample();
      }
      return sample;
    }
    boolean loadIf() {
      testStream.loadIf();
      boolean res=false;
      for(unsigned char i=0;i<channels;i++) {
        if(allItems[i]->enabled && !allItems[i]->error) res=res || allItems[i]->loadIf() ;
      }
    }
};

class MainAudioTimeline: public AudioTimeline {
  public:
  uint32_t saveSampleIndex;
  uint32_t lastSampleIndex;
  
  MainAudioTimeline() : AudioTimeline() {
    lastSampleIndex=sampleIndex;
  }
  uint32_t nextFrame() {
    if(!loop || length==0) return soundHeadPos++;
    if(++soundHeadPos>=length) soundHeadPos=0;
  }
  void doLoop() {
    if(!playing) return;
    loadIf();
    saveSampleIndex=sampleIndex;
    if(saveSampleIndex==lastSampleIndex) return;
    if(saveSampleIndex>lastSampleIndex) {
      
      for(uint32_t i=lastSampleIndex;i<saveSampleIndex;i++) {
        audioBuffer[i]=getSampleAt(nextFrame());
      }
    } else {
      for(uint32_t i=lastSampleIndex;i<bufferSize;i++) audioBuffer[i]=getSampleAt(nextFrame());
      for(uint32_t i=0;i<saveSampleIndex;i++) audioBuffer[i]=getSampleAt(nextFrame());
    }
    lastSampleIndex=saveSampleIndex;
  }
};



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
  
  
  // audioTimeline.loop=true;
  // audioTimeline.length=testStreamlength;
  SerialUSB.println("Ready to start");
  
  audioTimeline.addItem((AudioItem*)&testStream);
  audioTimeline.addItem((AudioItem*)&testSoundEffect);
  audioTimeline.addItem((AudioItem*)&testSoundWave);
  
  SerialUSB.println("Ready to start");
  audioTimeline.start();
  SerialUSB.println("Setup done");

  // for(uint16_t i=0;i<bufferSize;i++) audioBuffer[i]=testSoundEffect.getSampleAt(i)+playFreq(440,50,i);
  // for(uint16_t i=0;i<bufferSize;i++) audioTimeline.loop();
  tcStartCounter();
}

void loop() {
  // SerialUSB.print(audioTimeline.lastSampleIndex);
  // SerialUSB.print("..");
  audioTimeline.doLoop();
  // SerialUSB.println(audioTimeline.saveSampleIndex);
}




