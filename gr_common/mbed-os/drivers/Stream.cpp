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
#include "drivers/Stream.h"

namespace mbed {

mStream::mStream(const char *name) : FileLike(name), _file(NULL) {
    // No lock needed in constructor
    /* open ourselves */
    char buf[12]; /* :0x12345678 + null byte */
    std::sprintf(buf, ":%p", this);
    _file = std::fopen(buf, "w+");
    mbed_set_unbuffered_stream(_file);
}

mStream::~mStream() {
    // No lock can be used in destructor
    fclose(_file);
}

int mStream::putc(int c) {
    lock();
    fflush(_file);
    int ret = std::fputc(c, _file);
    unlock();
    return ret;
}
int mStream::puts(const char *s) {
    lock();
    fflush(_file);
    int ret = std::fputs(s, _file);
    unlock();
    return ret;
}
int mStream::getc() {
    lock();
    fflush(_file);
    int ret = mbed_getc(_file);
    unlock();
    return ret;
}
char* mStream::gets(char *s, int size) {
    lock();
    fflush(_file);
    char *ret = mbed_gets(s,size,_file);
    unlock();
    return ret;
}

int mStream::close() {
    return 0;
}

ssize_t mStream::write(const void* buffer, size_t length) {
    const char* ptr = (const char*)buffer;
    const char* end = ptr + length;

    lock();
    while (ptr != end) {
        if (_putc(*ptr++) == EOF) {
            break;
        }
    }
    unlock();

    return ptr - (const char*)buffer;
}

ssize_t mStream::read(void* buffer, size_t length) {
    char* ptr = (char*)buffer;
    char* end = ptr + length;

    lock();
    while (ptr != end) {
        int c = _getc();
        if (c==EOF) break;
        *ptr++ = c;
    }
    unlock();

    return ptr - (const char*)buffer;
}

off_t mStream::seek(off_t offset, int whence) {
    return 0;
}

off_t mStream::tell() {
    return 0;
}

void mStream::rewind() {
}

int mStream::isatty() {
    return 0;
}

int mStream::sync() {
    return 0;
}

size_t mStream::size() {
    return 0;
}

int mStream::printf(const char* format, ...) {
    lock();
    std::va_list arg;
    va_start(arg, format);
    fflush(_file);
    int r = vfprintf(_file, format, arg);
    va_end(arg);
    unlock();
    return r;
}

int mStream::scanf(const char* format, ...) {
    lock();
    std::va_list arg;
    va_start(arg, format);
    fflush(_file);
    int r = vfscanf(_file, format, arg);
    va_end(arg);
    unlock();
    return r;
}

int mStream::vprintf(const char* format, std::va_list args) {
    lock();
    fflush(_file);
    int r = vfprintf(_file, format, args);
    unlock();
    return r;
}

int mStream::vscanf(const char* format, std::va_list args) {
    lock();
    fflush(_file);
    int r = vfscanf(_file, format, args);
    unlock();
    return r;
}

} // namespace mbed
