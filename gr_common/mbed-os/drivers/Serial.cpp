/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "drivers/Serial.h"
#include "platform/mbed_wait_api.h"

#if DEVICE_SERIAL

namespace mbed {

mSerial::mSerial(PinName tx, PinName rx, const char *name, int baud) : SerialBase(tx, rx, baud), mStream(name) {
}

mSerial::mSerial(PinName tx, PinName rx, int baud): SerialBase(tx, rx, baud), mStream(NULL) {
}

int mSerial::_getc() {
    // Mutex is already held
    return _base_getc();
}

int mSerial::_putc(int c) {
    // Mutex is already held
    return _base_putc(c);
}

void mSerial::lock() {
    _mutex.lock();
}

void mSerial::unlock() {
    _mutex.unlock();
}

} // namespace mbed

#endif
