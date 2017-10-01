/*
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

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

  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  Modified 3 December 2013 by Matthijs Kooijman
  Modified 22 June 2016 by Nozomu Fujita
  Modified 15 June 2017 by Yuuki Okamiya
*/

#include "HardwareSerial.h"
#include "HardwareSerial_private.h"
#if defined(GRPEACH)
HardwareSerial Serial(USBTX, USBRX);
HardwareSerial Serial0(P2_14, P2_15);
HardwareSerial Serial1(P2_5, P2_6);
HardwareSerial Serial3(P5_3, P5_4);
#if 1
HardwareSerial Serial4(P8_14, P8_15);
#else
HardwareSerial Serial4(P5_0, P5_1);
#endif
#if 1
HardwareSerial Serial5(P8_13, P8_11);
#else
HardwareSerial Serial5(P6_6, P6_7);
#endif
HardwareSerial Serial6(P5_6, P5_7);
HardwareSerial Serial7(P7_4, P7_5);
#elif defined(GRLYCHEE)
HardwareSerial Serial(USBTX, USBRX);
HardwareSerial Serial0(P7_8, P1_8);
HardwareSerial Serial2(P7_1, P0_1);
HardwareSerial Serial4(P5_15, P5_14);
#endif
// Public Methods //////////////////////////////////////////////////////////////
void HardwareSerial::begin(unsigned long baud, byte config)
{
	_serial = new BufferedSerial(_tx, _rx, SERIAL_BUFFER_SIZE, 1, NULL);
    _serial->baud((int)baud);

    int bits = 8;
    RawSerial::Parity parity = RawSerial::None;
    int stop_bits = 1;
    const uint8_t SERIAL_DM = 0b00000111;
    const uint8_t SERIAL_D8 = 0b00000110;
    const uint8_t SERIAL_D7 = 0b00000100;
#if 0
    const uint8_t SERIAL_D6 = 0b00000010;
    const uint8_t SERIAL_D5 = 0b00000000;
#endif
    const uint8_t SERIAL_PM = 0b00110000;
    const uint8_t SERIAL_PN = 0b00000000;
    const uint8_t SERIAL_PE = 0b00100000;
    const uint8_t SERIAL_PO = 0b00110000;
    const uint8_t SERIAL_SM = 0b00001000;
    const uint8_t SERIAL_S1 = 0b00000000;
    const uint8_t SERIAL_S2 = 0b00001000;

    switch (config & SERIAL_DM) {
    case SERIAL_D8:
        bits = 8;
        break;
    case SERIAL_D7:
        bits = 7;
        break;
#if 0
    case SERIAL_D6:
        bits = 6;
        break;
    case SERIAL_D5:
        bits = 5;
        break;
#endif
    default:
        break;
    }
    switch (config & SERIAL_PM) {
    case SERIAL_PN:
        parity = RawSerial::None;
        break;
    case SERIAL_PE:
        parity = RawSerial::Even;
        break;
    case SERIAL_PO:
        parity = RawSerial::Odd;
        break;
    default:
        break;
    }
    switch (config & SERIAL_SM) {
    case SERIAL_S1:
        stop_bits = 1;
        break;
    case SERIAL_S2:
        stop_bits = 2;
        break;
    default:
        break;
    }
    _serial->format(bits, parity, stop_bits);

}

void HardwareSerial::end()
{
    flush();
    delete _serial;
}

int HardwareSerial::available(void)
{
	return _serial->readable();
}

int HardwareSerial::peek(void)
{
	// no implements
    return -1;
}

int HardwareSerial::read(void)
{
	return _serial->getc();
}

void HardwareSerial::flush()
{
	while(!_serial->writeable()){
	}
}

size_t HardwareSerial::write(uint8_t c)
{
	return _serial->putc((uint8_t)c);
}

