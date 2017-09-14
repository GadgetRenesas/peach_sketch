/* mbed Microcontroller Library
 * Copyright (c) 2015 ARM Limited
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

#include "File.h"
#include "mbed.h"
#include <errno.h>


mFile::mFile()
    : _fs(0), _file(0)
{
}

mFile::mFile(FileSystem *fs, const char *path, int flags)
    : FileLike(path), _fs(0), _file(0)
{
    open(fs, path, flags);
}

mFile::~mFile()
{
    if (_fs) {
        close();
    }
}

int mFile::open(FileSystem *fs, const char *path, int flags)
{
    if (_fs) {
        return -EINVAL;
    }

    int err = fs->file_open(&_file, path, flags);
    if (!err) {
        _fs = fs;
    }

    return err;
}

int mFile::close()
{
    if (!_fs) {
        return -EINVAL;
    }

    int err = _fs->file_close(_file);
    _fs = 0;
    return err;
}

ssize_t mFile::read(void *buffer, size_t len)
{
    MBED_ASSERT(_fs);
    return _fs->file_read(_file, buffer, len);
}

ssize_t mFile::write(const void *buffer, size_t len)
{
    MBED_ASSERT(_fs);
    return _fs->file_write(_file, buffer, len);
}

int mFile::sync()
{
    MBED_ASSERT(_fs);
    return _fs->file_sync(_file);
}

int mFile::isatty()
{
    MBED_ASSERT(_fs);
    return _fs->file_isatty(_file);
}

off_t mFile::seek(off_t offset, int whence)
{
    MBED_ASSERT(_fs);
    return _fs->file_seek(_file, offset, whence);
}

off_t mFile::tell()
{
    MBED_ASSERT(_fs);
    return _fs->file_tell(_file);
}

void mFile::rewind()
{
    MBED_ASSERT(_fs);
    return _fs->file_rewind(_file);
}

size_t mFile::size()
{
    MBED_ASSERT(_fs);
    return _fs->file_size(_file);
}

