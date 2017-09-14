/* mbed EasyDec_Wav Library
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

/**************************************************************************//**
* @file          EasyDec_Wav.h
* @brief         wav
******************************************************************************/
#ifndef __EASY_DECODER_WAV_H__
#define __EASY_DECODER_WAV_H__

#include "EasyDecoder.h"

/** A class to communicate a EasyDec_Wav
 *
 */
class EasyDec_Wav : public EasyDecoder {
public:

    static inline EasyDecoder* inst() { return new EasyDec_Wav; }

    /** analyze header
     *
     * @param p_title title tag buffer
     * @param p_artist artist tag buffer
     * @param p_album album tag buffer
     * @param tag_size tag buffer size
     * @param fp file pointer
     * @return true = success, false = failure
     */
    virtual bool AnalyzeHeder(char* p_title, char* p_artist, char* p_album, uint16_t tag_size, FILE* fp) {
        bool result = false;
        size_t read_size;
        uint8_t wk_read_buff[36];
        char *data;
        uint32_t chunk_size;
        uint32_t sub_chunk_size;
        uint32_t list_index_max;
        bool list_ok = false;
        uint32_t read_index = 0;
        uint32_t data_index = 0;
        uint16_t wk_len;

        if (fp == NULL) {
            return false;
        }
        music_data_size  = 0;
        music_data_index = 0;
        wav_fp = fp;
        if (p_title != NULL) {
            p_title[0] = '\0';
        }
        if (p_artist != NULL) {
            p_artist[0] = '\0';
        }
        if (p_album != NULL) {
            p_album[0] = '\0';
        }

        read_size = fread(&wk_read_buff[0], sizeof(char), 36, wav_fp);
        if (read_size < 36) {
            // do nothing
        } else if (memcmp(&wk_read_buff[0], "RIFF", 4) != 0) {
            // do nothing
        } else if (memcmp(&wk_read_buff[8], "WAVE", 4) != 0) {
            // do nothing
        } else if (memcmp(&wk_read_buff[12], "fmt ", 4) != 0) {
            // do nothing
        } else {
            read_index += 36;
            channel = ((uint32_t)wk_read_buff[22] << 0) + ((uint32_t)wk_read_buff[23] << 8);
            sampling_rate = ((uint32_t)wk_read_buff[24] << 0)
                          + ((uint32_t)wk_read_buff[25] << 8)
                          + ((uint32_t)wk_read_buff[26] << 16)
                          + ((uint32_t)wk_read_buff[27] << 24);
            block_size = ((uint32_t)wk_read_buff[34] << 0) + ((uint32_t)wk_read_buff[35] << 8);
            while (1) {
                read_size = fread(&wk_read_buff[0], sizeof(char), 8, wav_fp);
                read_index += 8;
                if (read_size < 8) {
                    break;
                } else {
                    chunk_size = ((uint32_t)wk_read_buff[4] << 0)
                               + ((uint32_t)wk_read_buff[5] << 8)
                               + ((uint32_t)wk_read_buff[6] << 16)
                               + ((uint32_t)wk_read_buff[7] << 24);
                    if (memcmp(&wk_read_buff[0], "data", 4) == 0) {
                        result = true;
                        music_data_size = chunk_size;
                        if (list_ok == true) {
                            break;
                        } else {
                            data_index = read_index;
                            fseek(wav_fp, chunk_size, SEEK_CUR);
                            read_index += chunk_size;
                        }
                    } else if (memcmp(&wk_read_buff[0], "LIST", 4) == 0) {
                        list_ok = true;
                        list_index_max = read_index + chunk_size;
                        read_size = fread(&wk_read_buff[0], sizeof(char), 4, wav_fp);
                        read_index += 4;
                        while (read_index < list_index_max) {
                            read_size = fread(&wk_read_buff[0], sizeof(char), 8, wav_fp);
                            read_index += 8;
                            if (read_size < 8) {
                                break;
                            } else if (memcmp(&wk_read_buff[0], "INAM", 4) == 0) {
                                data = p_title;
                            } else if (memcmp(&wk_read_buff[0], "IART", 4) == 0) {
                                data = p_artist;
                            } else if (memcmp(&wk_read_buff[0], "IPRD", 4) == 0) {
                                data = p_album;
                            } else {
                                data = NULL;
                            }
                            sub_chunk_size = ((uint32_t)wk_read_buff[4] << 0)
                                           + ((uint32_t)wk_read_buff[5] << 8)
                                           + ((uint32_t)wk_read_buff[6] << 16)
                                           + ((uint32_t)wk_read_buff[7] << 24);
                            if ((data != NULL) && (tag_size != 0)) {
                                if (sub_chunk_size > (uint32_t)(tag_size - 1)) {
                                    wk_len = (tag_size - 1);
                                } else {
                                    wk_len = sub_chunk_size;
                                }
                                read_size = fread(data, sizeof(char), wk_len, wav_fp);
                                data[wk_len] = '\0';
                            }
                            if ((sub_chunk_size & 0x00000001) != 0) {
                                sub_chunk_size += 1;
                            }
                            read_index += sub_chunk_size;
                            fseek(wav_fp, read_index, SEEK_SET);
                        }
                        if (data_index != 0) {
                            break;
                        } else {
                            fseek(wav_fp, list_index_max, SEEK_SET);
                        }
                    } else {
                        fseek(wav_fp, chunk_size, SEEK_CUR);
                        read_index += chunk_size;
                    }
                }
            }

            if (data_index != 0) {
                fseek(wav_fp, data_index, SEEK_SET);
            }
        }

        return result;
    };

    /** get next data
     *
     * @param buf data buffer address
     * @param len data buffer length
     * @return get data size
     */
    virtual size_t GetNextData(void *buf, size_t len) {
        if (block_size == 24) {
            // Add padding
            uint32_t write_index = 0;
            uint32_t wavfile_index;
            uint32_t read_len;
            uint32_t pading_index = 0;
            uint8_t * p_buf = (uint8_t *)buf;
            size_t ret;

            if ((music_data_index + len) > music_data_size) {
                len = music_data_size - music_data_index;
            }
            while (write_index < (uint32_t)len) {
                read_len = (len - write_index) * 3 / 4;
                if (read_len > sizeof(wk_wavfile_buff)) {
                    read_len = sizeof(wk_wavfile_buff);
                }
                music_data_index += read_len;
                ret = fread(wk_wavfile_buff, sizeof(char), read_len, wav_fp);
                if (ret < read_len) {
                    break;
                }
                wavfile_index = 0;
                while ((write_index < len) && (wavfile_index < read_len)) {
                    if (pading_index == 0) {
                        p_buf[write_index] = 0;
                    } else {
                        p_buf[write_index] = wk_wavfile_buff[wavfile_index];
                        wavfile_index++;
                    }
                    if (pading_index < 3) {
                        pading_index++;
                    } else {
                        pading_index = 0;
                    }
                    write_index++;
                }
            }

            return write_index;
        } else {
            if ((music_data_index + len) > music_data_size) {
                len = music_data_size - music_data_index;
            }
            music_data_index += len;

            return fread(buf, sizeof(char), len, wav_fp);
        }
    };

    /** get channel
     *
     * @return channel
     */
    virtual uint16_t GetChannel() {
        return channel;
    };

    /** get block size
     *
     * @return block size
     */
    virtual uint16_t GetBlockSize() {
        return block_size;
    };

    /** get sampling rate
     *
     * @return sampling rate
     */
    virtual uint32_t GetSamplingRate() {
        return sampling_rate;
    };

private:
    FILE * wav_fp;
    uint32_t music_data_size;
    uint32_t music_data_index;
    uint16_t channel;
    uint16_t block_size;
    uint32_t sampling_rate;
    uint8_t wk_wavfile_buff[3072];
};

#endif
