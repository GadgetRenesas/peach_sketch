/* mbed EasyPlayback Library
 * Copyright (C) 2017 dkato
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

#include "mbed.h"
#include "dcache-control.h"
#include "EasyPlayback.h"

EasyPlayback::EasyPlayback() : audio(0x80, (AUDIO_WRITE_BUFF_NUM - 1), 0), _buff_index(0),
    _skip(false), _pause(false), _init_end(false)
{
    _heap_buf = new uint8_t[AUDIO_WRITE_BUFF_SIZE * AUDIO_WRITE_BUFF_NUM + 31];
    _audio_buf = (uint8_t (*)[AUDIO_WRITE_BUFF_SIZE])(((uint32_t)_heap_buf + 31ul) & ~31ul);
}

EasyPlayback::~EasyPlayback()
{
    delete [] _heap_buf;
}

bool EasyPlayback::get_tag(const char* filename, char* p_title, char* p_artist, char* p_album, uint16_t tag_size)
{
    FILE * fp;
    EasyDecoder * decoder;
    bool ret = false;

    decoder = create_decoer_class(filename);
    if (decoder == NULL) {
        return false;
    }

    fp = fopen(filename, "r");
    if (decoder->AnalyzeHeder(p_title, p_artist, p_album, tag_size, fp) != false) {
        ret = true;
    }
    delete decoder;
    fclose(fp);

    return ret;
}

bool EasyPlayback::play(const char* filename)
{
    const rbsp_data_conf_t audio_write_async_ctl = {NULL, NULL};
    size_t audio_data_size = AUDIO_WRITE_BUFF_SIZE;
    FILE * fp;
    uint8_t * p_buf;
    EasyDecoder * decoder;
    bool ret = false;

    decoder = create_decoer_class(filename);
    if (decoder == NULL) {
        return false;
    }

    if (!_init_end) {
        audio.power();
        audio.outputVolume(1.0, 1.0);
        _init_end = true;
    }

     _skip = false;
    fp = fopen(filename, "r");
    if (decoder->AnalyzeHeder(NULL, NULL, NULL, 0, fp) == false) {
        // do nothing
    } else if  ((decoder->GetChannel() != 2)
            || (audio.format(decoder->GetBlockSize()) == false)
            || (audio.frequency(decoder->GetSamplingRate()) == false)) {
        // do nothing
    } else {
        while (audio_data_size == AUDIO_WRITE_BUFF_SIZE) {
            while ((_pause) && (!_skip)) {
                Thread::wait(100);
            }
            if (_skip) {
                break;
            }
            p_buf = _audio_buf[_buff_index];
            audio_data_size = decoder->GetNextData(p_buf, AUDIO_WRITE_BUFF_SIZE);
            if (audio_data_size > 0) {
                dcache_clean(p_buf, audio_data_size);
                audio.write(p_buf, audio_data_size, &audio_write_async_ctl);
                _buff_index = (_buff_index + 1) & AUDIO_MSK_RING_BUFF;
            }
        }
        Thread::wait(500);
        ret = true;
    }
    delete decoder;
    fclose(fp);

    return ret;
}

bool EasyPlayback::is_paused(void)
{
    return _pause;
}

void EasyPlayback::pause()
{
    _pause = !_pause;
}

void EasyPlayback::pause(bool type)
{
    _pause = type;
}

void EasyPlayback::skip(void)
{
    _skip = true;
}

bool EasyPlayback::outputVolume(float VolumeOut)
{
    if (!_init_end) {
        audio.power();
        _init_end = true;
    }
    return audio.outputVolume(VolumeOut, VolumeOut);
}

EasyDecoder * EasyPlayback::create_decoer_class(const char* filename)
{
    std::map<std::string, EasyDecoder*(*)()>::iterator itr;
    char *extension = strstr(filename, ".");

    if (extension == NULL) {
        return NULL;
    }

    itr = m_lpDecoders.find(extension);
    if (itr == m_lpDecoders.end()) {
        return NULL;
    }

    return (*itr).second();
}

