#ifndef __AudioItem_h
#define __AudioItem_h
#include <stdint.h>
#include "Arduino.h"


class AudioItem {
  public:
  uint32_t timelinePosition=0;
  uint32_t soundHeadPos=0;
  uint32_t loopPos=0;
  uint32_t length=0;
  boolean remove=false;
  boolean removeAtEnd=false;
  boolean enabled=true;
  boolean error=false;
  boolean loop=false;
  boolean errorTxt="";
  boolean playing=true;
  
  AudioItem();
  AudioItem(uint32_t myPos);
  void init(uint32_t myPos);
  ~AudioItem();
  virtual void dispose();
  virtual void stop();
  virtual void start();
  virtual void onEnded();
  virtual void onLoop();
  virtual uint16_t getSampleAt(uint32_t pos);
  virtual uint16_t getNextSample();
  virtual boolean loadIf();

  static uint16_t eightBitsToUnsigned16(unsigned char b);
  static int16_t eightBitsToSigned16(unsigned char b);
  
  static uint16_t eightBitsToUnsigned12(unsigned char b);
  static int16_t eightBitsToSigned12(unsigned char b);

};




#endif // __AudioItem_h
