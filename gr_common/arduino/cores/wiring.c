/* wiring.c */
/* Copyright (C) 2016 Nozomu Fujita, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* May 25th 2017: Modified by Yuuki Okamiya. */

#include <wiring_private.h>

#if defined(GRLYCHEE) || defined(GRPEACH)
#include <mbed_wait_api.h>
#endif //GRLYCHEE GRPEACH

uint32_t micros(void)
{
    return (uint32_t)get_micros_arduino_system_timer();
}

uint32_t millis(void)
{
    return (uint32_t)get_millis_arduino_system_timer();
}

void delayMicroseconds(unsigned int us)
{
	wait_us(us);
}

void delay(unsigned long ms)
{
	wait_ms(ms);
}

int min(int a, int b){
	return ((a)<(b)?(a):(b));
}

int max(int a, int b){
	return ((a)>(b)?(a):(b));
}
