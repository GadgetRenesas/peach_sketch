/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified July 2nd 2014 by Yuuki Okamiya for GR-SAKURA
  Modified July 13rd 2014 by Nozomu Fujita for GR-SAKURA
  Modified June 22nd 2016 by Nozomu Fujita for GR-PEACH

*/

#ifndef Arduino_h
#define Arduino_h

#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(GRLYCHEE) || defined(GRPEACH)
#include <PinNames.h>
//#include <gpio_api.h>
//#include <PinNames.h>
//#include "pins_arduino.h"
#endif /*GRLYCHEE*/

#include "avr/pgmspace.h"
#include "binary.h"

#if !defined(GRLYCHEE) && !defined(GRPEACH)
#include <avr/interrupt.h>
#include <avr/io.h>
#else /*GRLYCHEE GR-PEACH*/
#ifdef GRPEACH
#define F_CPU (400 * 1000 * 1000L)
#define PCLK  (200 * 1000 * 1000 / 3.0L)
#else GRLYCHEE
#define F_CPU (384 * 1000 * 1000L)
#define PCLK  (192 * 1000 * 1000 / 3.0L)
#endif
#define _BV(bit) (1 << (bit))
#endif/*GRLYCHEE GR-PEACH*/

#ifdef __cplusplus
extern "C"{
#endif

void yield(void);

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2
#if defined(GRLYCHEE) || defined(GRPEACH)
#define OUTPUT_OPENDRAIN 0x3
#endif/*GRLYCHEE GRPEACH*/

#if !defined(GRLYCHEE) && !defined(GRPEACH)
#define true 0x1
#define false 0x0
#else /*GRLYCHEE GRPEACH*/
#include <stdbool.h>
#endif/*GRLYCHEE GRPEACH*/

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER 2.718281828459045235360287471352

#define SERIAL  0x0
#define DISPLAY 0x1

#define LSBFIRST 0
#define MSBFIRST 1

#define CHANGE 1
#define FALLING 2
#define RISING 3

#if !defined(GRLYCHEE) && !defined(GRPEACH)
#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define DEFAULT 0
#define EXTERNAL 1
#define INTERNAL 2
#else  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
#define INTERNAL1V1 2
#define INTERNAL2V56 3
#else
#define INTERNAL 3
#endif
#define DEFAULT 1
#define EXTERNAL 0
#endif
#else /*GRLYCHEE GRPEACH*/
#define DEFAULT_0 0 //conflict with USBDevice_Types.h so added _0
#define INTERNAL 1
#define EXTERNAL 2
#define RAW12BIT 3
#endif/*GRLYCHEE GRPEACH*/

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

#if !defined(GRLYCHEE) && !defined(GRPEACH)
#define interrupts() sei()
#define noInterrupts() cli()
#else
#include <core_caFunc.h>
#define interrupts() __enable_irq()
#define noInterrupts() __disable_irq()
#define isNoInterrupts() \
({ \
    uint32_t cpsr; \
    __asm __volatile( \
        "mrs %0, cpsr" \
        : "=r"(cpsr) \
    ); \
    cpsr & 0x80 ? 1 : 0; \
})
#endif

#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

// avr-libc defines _NOP() since 1.6.2
#ifndef _NOP
#define _NOP() do { __asm__ volatile ("nop"); } while (0)
#endif

typedef unsigned int word;

#define bit(b) (1UL << (b))

typedef uint8_t boolean;
typedef uint8_t byte;
#if defined(GRLYCHEE) || defined(GRPEACH)
typedef void (*fInterruptFunc_t)(void);
typedef void (*fITInterruptFunc_t)(unsigned long u32timer_millis);
#endif

void init(void);

void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int digitalRead(uint8_t);
int analogRead(uint8_t);
void analogReference(uint8_t mode);
void analogWrite(uint8_t, int);
#if defined(GRLYCHEE) || defined(GRPEACH)
void analogWriteDAC(int port, int val);
void analogReadClock(uint8_t clock);
#endif/*GRLYCHEE GRPEACH*/

unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long);
void delayMicroseconds(unsigned int us);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
void shiftOutEx(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, int len, uint32_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
uint32_t shiftInEx(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, int len);

void attachInterrupt(uint8_t, void (*)(void), int mode);
void detachInterrupt(uint8_t);

void setup(void);
void loop(void);

#if 0
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#endif
int min(int a,int b);
int max(int a,int b);
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))


// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.

#define analogInPinToBit(P) (P)

// On the ATmega1280, the addresses of some of the port registers are
// greater than 255, so we can't store them in uint8_t's.
#if !defined(GRLYCHEE) && !defined(GRPEACH)
extern const uint16_t PROGMEM port_to_mode_PGM[];
extern const uint16_t PROGMEM port_to_input_PGM[];
extern const uint16_t PROGMEM port_to_output_PGM[];

extern const uint8_t PROGMEM digital_pin_to_port_PGM[];
// extern const uint8_t PROGMEM digital_pin_to_bit_PGM[];
extern const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[];
extern const uint8_t PROGMEM digital_pin_to_timer_PGM[];
#endif/*GRLYCHEE GRPEACH*/

// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.
// 
// These perform slightly better as macros compared to inline functions
//
#if !defined(GRLYCHEE) && !defined(GRPEACH)
#define digitalPinToPort(P) ( pgm_read_byte( digital_pin_to_port_PGM + (P) ) )
#define digitalPinToBitMask(P) ( pgm_read_byte( digital_pin_to_bit_mask_PGM + (P) ) )
#define digitalPinToTimer(P) ( pgm_read_byte( digital_pin_to_timer_PGM + (P) ) )
#else /*GRLYCHEE GRPEACH*/
#include <gpio_addrdefine.h>
#define digitalPinToPort(P) (PINGROUP(P))
#define digitalPinToBitMask(P) (1<<(PINNO(P)))
#endif/*GRLYCHEE GRPEACH*/
#define analogInPinToBit(P) (P)
#if !defined(GRLYCHEE) && !defined(GRPEACH)
#define portOutputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_output_PGM + (P))) )
#define portInputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_input_PGM + (P))) )
#define portModeRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_mode_PGM + (P))) )
#else /*GRLYCHEE GRPEACH*/
#define portOutputRegister(P) (PORT(P))
#define portSetResetRegister(P) (PSR(n))
#define portInputRegister(P) (PPR(P))
#define portModeRegister(P) (PM(P))
#define portModeControlRegister(P) (PMC(P))
#define portFunctinControlRegister(P) (PFC(P))
#define portFunctinControlExpandRegister(P) (PFCE(P))
#define portNotRegister(P) (PNOT(P))
#define portModeSetResetRegister(P) (PMSR(P))
#define portModeControlSetResetRegister(P) (PMCSR(P))
#define portFunctinControlAppendExpandRegister(P) (PFCAE(P))
#define portInputBufferControlRegister(P) (PIBC(P))
#define portBiDirectionControlRegister(P) (PBDC(P))
#define portIPControlRegister(P) (PIPC(P))
#endif/*GRLYCHEE GRPEACH*/

#if !defined(GRLYCHEE) && !defined(GRPEACH)
#define NOT_A_PIN 0
#define NOT_A_PORT 0
#else /*GRLYCHEE GRPEACH*/
#define NOT_A_PIN 0xff
#define NOT_A_PORT 0xff
#endif/*GRLYCHEE GRPEACH*/

#define NOT_AN_INTERRUPT -1

#ifdef ARDUINO_MAIN
#if !defined(GRLYCHEE) && !defined(GRPEACH)
#define PA 1
#define PB 2
#define PC 3
#define PD 4
#define PE 5
#define PF 6
#define PG 7
#define PH 8
#define PJ 10
#define PK 11
#define PL 12
#endif/*GRLYCHEE GRPEACH*/
#endif

#if !defined(GRLYCHEE) && !defined(GRPEACH)
#define NOT_ON_TIMER 0
#define TIMER0A 1
#define TIMER0B 2
#define TIMER1A 3
#define TIMER1B 4
#define TIMER2  5
#define TIMER2A 6
#define TIMER2B 7

#define TIMER3A 8
#define TIMER3B 9
#define TIMER3C 10
#define TIMER4A 11
#define TIMER4B 12
#define TIMER4C 13
#define TIMER4D 14
#define TIMER5A 15
#define TIMER5B 16
#define TIMER5C 17
#else /*GRLYCHEE GRPEACH*/
#define NOT_ON_TIMER 0
#endif/*GRLYCHEE GRPEACH*/

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
#include "WCharacter.h"
#include "WString.h"
#include "HardwareSerial.h"
#if !defined(GRLYCHEE) && !defined(GRPEACH)
#include "USBAPI.h"
#endif/*GRLYCHEE GRPEACH*/
#if defined(HAVE_HWSERIAL0) && defined(HAVE_CDCSERIAL)
#error "Targets with both UART0 and CDC serial not supported"
#endif

uint16_t makeWord(uint16_t w);
uint16_t makeWord(uint8_t h, uint8_t l);

#define word(...) makeWord(__VA_ARGS__)

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

void tone(uint8_t pin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t pin);

#if defined(GRLYCHEE) || defined(GRPEACH)
void pinMode(PinName pinName, uint8_t mode);
void digitalWrite(PinName pinName, uint8_t);
int digitalRead(PinName pinName);
int analogRead(PinName pinName);
void analogWrite(PinName pinName, int);
void analogWriteDAC(uint8_t port, int val);
void analogWriteDAC(PinName pinName, int val);
void analogReadClock(uint8_t clock);
void shiftOut(PinName dataPinName, PinName clockPinName, uint8_t bitOrder, int len, uint8_t val);
void shiftOutEx(PinName dataPinName, PinName clockPinName, uint8_t bitOrder, int len, uint32_t val);
uint8_t shiftIn(PinName dataPinName, PinName clockPinName, uint8_t bitOrder);
uint32_t shiftInEx(PinName dataPinName, PinName clockPinName, uint8_t bitOrder, int len);
unsigned long pulseIn(PinName pinName, uint8_t state, unsigned long timeout = 1000000L);
void attachInterrupt(PinName pinName, void (*userFunc)(void), int mode);
void attachInterrupt(uint8_t irq, PinName pinName, void (*userFunc)(void), int mode);
void attachInterrupt(uint8_t irq, uint8_t pin, void (*userFunc)(void), int mode);
void detachInterrupt(PinName pinName);
void detachInterrupt(uint8_t irq, PinName pinName);
void detachInterrupt(uint8_t irq, uint8_t pin);
void tone(PinName pinName, unsigned int frequency, unsigned long duration = 0);
void noTone(PinName pinName);
#endif/*GRLYCHEE GRPEACH*/

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned int);
long amap(long, long, long, long, long);

#endif

#include "pins_arduino.h"

#endif
