/*
  HardwareSerial_private.h - Hardware serial library for Wiring
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
  Modified 30 Aug 2016 by Yuuki Okamiya
*/

#include "wiring_private.h"

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(PinName tx, PinName rx)
{
	_tx = tx;
	_rx = rx;
	_serial = NULL;
}

// Actual interrupt handlers //////////////////////////////////////////////////////////////
#if 0
void HardwareSerial::_rx_complete_irq(void)
{
    while (serial_readable(&_serial)) {
        rx_buffer_index_t i = (rx_buffer_index_t)((_rx_buffer_head + 1) % SERIAL_RX_BUFFER_SIZE);
        int c = serial_getc(&_serial);
        if (i != _rx_buffer_tail) {
            _rx_buffer[_rx_buffer_head] = c;
            _rx_buffer_head = i;
        }
    }
}
#endif
