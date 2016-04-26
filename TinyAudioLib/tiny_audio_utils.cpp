#include "tiny_audio_utils.h"
#include "tiny_audio_setup.h"



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
  uint_fast16_t prop=HALF_OF_DAC_MINUS/fq;
  return HALF_OF_DAC+(x%fq)*prop;
}


// Freq is multiplied by 10x
uint16_t playSineFreq(uint_fast16_t f,uint_fast8_t a,uint32_t x) { // use table lookup and interpolation for sin instead
  return (int16_t)((uint32_t)getSineFromMem((x*f/10*0xFF/11025)%0xFF)*((uint32_t)HALF_OF_DAC_MINUS)/255L*a/100);
  // return (int16_t)( 256+round(256*sin((double)x*f/11025.0*2*PI)*a/100) );
}
// find a nice instrument play function (organ is the bast candidate 440Hz+660Hz*.2 http://music.stackexchange.com/questions/8214/how-can-i-replicate-the-sound-of-an-instrument )
// Freq is multiplied by 10x
uint16_t playOrganFreq(uint_fast16_t f,uint_fast8_t a,uint32_t x) { // use table lookup and interpolation for sin instead
  return 
  	(int16_t)((uint32_t)getSineFromMem((x*f/10*0xFF/11025)%0xFF)*((uint32_t)HALF_OF_DAC_MINUS)/255L*a/100)+
  	(int16_t)((uint32_t)getSineFromMem((x*(2*f/3)/10*0xFF/11025)%0xFF)*((uint32_t)HALF_OF_DAC_MINUS)/255L*a/100);
  // return (int16_t)( 256+round(256*sin((double)x*f/11025.0*2*PI)*a/100) );
}



// Middle C is -9
// Freq is multiplied by 10x
int frequencyFromNote(int halfStepsFromA4) { // save it to a table 
  return 4400*pow(1.059463094359295, halfStepsFromA4); // 1.059463094359295 is 2^(1/12)
}



