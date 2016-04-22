

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




PROGMEM  prog_uchar sine256[]  = {
  127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,176,178,181,184,187,190,192,195,198,200,203,205,208,210,212,215,217,219,221,223,225,227,229,231,233,234,236,238,239,240,
  242,243,244,245,247,248,249,249,250,251,252,252,253,253,253,254,254,254,254,254,254,254,253,253,253,252,252,251,250,249,249,248,247,245,244,243,242,240,239,238,236,234,233,231,229,227,225,223,
  221,219,217,215,212,210,208,205,203,200,198,195,192,190,187,184,181,178,176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,78,
  76,73,70,67,64,62,59,56,54,51,49,46,44,42,39,37,35,33,31,29,27,25,23,21,20,18,16,15,14,12,11,10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0,0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,9,10,11,12,14,15,16,18,20,21,23,25,27,29,31,
  33,35,37,39,42,44,46,49,51,54,56,59,62,64,67,70,73,76,78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124

};


volatile prog_uchar getSineFromMem(uint_fast8_t pos) {
  return pgm_read_byte_near(sine256 + pos);  
}


// Freq is multiplied by 10x
uint16_t playSawFreq(uint_fast16_t f,uint_fast8_t a,uint32_t x) {
uint_fast16_t fq=(11025/f)*10;
uint_fast16_t prop=256/fq;
  return 256+(x%fq)*prop;
}


// Freq is multiplied by 10x
uint16_t playSineFreq(uint_fast16_t f,uint_fast8_t a,uint32_t x) { // use table lookup and interpolation for sin instead
  return (int16_t)(getSineFromMem((x*f/10*0xFF/11025)%0xFF)*a/100 );
  // return (int16_t)( 256+round(256*sin((double)x*f/11025.0*2*PI)*a/100) );
}
// find a nice instrument play function (organ is the bast candidate 440Hz+660Hz*.2 http://music.stackexchange.com/questions/8214/how-can-i-replicate-the-sound-of-an-instrument )




// Middle C is -9
// Freq is multiplied by 10x
int frequencyFromNote(int halfStepsFromA4) { // save it to a table 
  return 4400*pow(1.059463094359295, halfStepsFromA4); // 1.059463094359295 is 2^(1/12)
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





class AudioItemWave: public AudioItem {
  public:
  waveGeneratorFunction generator=nullPtr;
  
  AudioItemWave() : AudioItem() {
  }
  AudioItemWave(uint32_t myPos,waveGeneratorFunction myFunc) : AudioItem() {
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






typedef uint16_t (*noteGeneratorFunction)(uint_fast16_t freq,uint_fast8_t amplitude,uint32_t frame); // Freq is multiplied by 10x



class AudioItemNote: public AudioItem {
  public:
  noteGeneratorFunction generator=nullPtr;
  int frequency=4400; // note A4 is 440 // // Freq is multiplied by 10x
  uint8_t amplitude=100;
  
  AudioItemNote() : AudioItem() {
  }
  AudioItemNote(uint32_t myPos,noteGeneratorFunction myFunc) : AudioItem() {
    init(myPos,myFunc);
  }
  AudioItemNote(uint32_t myPos,noteGeneratorFunction myFunc,int startingFreq,uint8_t startingAmplitude) : AudioItem() {
    init(myPos,myFunc,startingFreq,startingAmplitude);
  }
  void init(uint32_t myPos,noteGeneratorFunction myFunc) {
    AudioItem::init(myPos);
    generator=myFunc;
  }
  void init(uint32_t myPos,noteGeneratorFunction myFunc,int startingFreq,uint8_t startingAmplitude) {
    AudioItem::init(myPos);
    frequency=startingFreq;
    amplitude=startingAmplitude;
    generator=myFunc;
  }
  uint16_t getSampleAt(uint32_t pos) {
    if(!enabled) return 0;
    if(generator!=nullPtr) return generator(frequency,amplitude,pos); // Freq is multiplied by 10x
  }
};






AudioItemRawData8bit testSoundEffect; // (0,(unsigned char*)brb,4412);
AudioItemFile testStream; // (0,(char*)"/TinyTris/music.raw");

AudioItemWave testSoundWave; // (0,waveGeneratorTest);
AudioItemNote testSoundNote; // (0,waveGeneratorTest);





const unsigned int MAX_ITEMS=100;

class AudioTimeline: public AudioItem { // a timeline is really an audio item (could be nested)!
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
      // return playSineFreq(110,50,pos);
      
      // return playSineFreq(880,50,pos);
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
  testSoundNote.init(0,playSineFreq);
  testSoundNote.frequency=frequencyFromNote(+2);
  
  
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

  // for(uint16_t i=0;i<bufferSize;i++) audioBuffer[i]=testSoundEffect.getSampleAt(i)+playSineFreq(440,50,i);
  // for(uint16_t i=0;i<bufferSize;i++) audioTimeline.loop();
  tcStartCounter();
}

void loop() {
  // SerialUSB.print(audioTimeline.lastSampleIndex);
  // SerialUSB.println(frequencyFromNote(-9));
  audioTimeline.doLoop();
  // SerialUSB.println(audioTimeline.saveSampleIndex);
}




