#ifndef _ARDUINO_H
#define _ARDUINO_H

#include "bsp/include/nm_bsp_mk82fn256.h"
#include "board.h"

#ifdef __cplusplus
extern "C"{
#endif

#define BIN 	2
#define OCTAL	8
#define DEC		10
#define HEX		16

#define delay	_delay_ms
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))
//#define interrupts() sei()
//#define noInterrupts() cli()
#define clockCyclesPerMicrosecond() USEC_TO_COUNT(1, CLOCK_GetFreq(kCLOCK_BusClk))
#define clockCyclesToMicroseconds(a) COUNT_TO_USEC(a, CLOCK_GetFreq(kCLOCK_BusClk))
#define microsecondsToClockCycles(a) USEC_TO_COUNT(a, CLOCK_GetFreq(kCLOCK_BusClk))
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

typedef unsigned int word;

#define bit(b) (1UL << (b))

typedef uint8_t boolean;
typedef uint8_t byte;


// Not exactly arduino...
inline const char * const BoolToString(bool b)
{
  return b ? "true" : "false";
}

#ifdef __cplusplus
} // extern "C"
#endif


#endif
