/*
 Camera.cpp - Camera library for GR-LYCHEE.
 Copyright (c) 2017 Renesas Electronics.  All right reserved.

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
 */

#include "Camera.h"
#include "JPEG_Converter.h"
#include "EasyAttach_CameraAndLCD.h"
#include "dcache-control.h"

static uint8_t FrameBuffer_Video[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));
static uint8_t JpegBuffer[1024 * 63]__attribute((aligned(32)));

/* jpeg convert */
static JPEG_Converter Jcu;
static DisplayBase Display;

static size_t encode_jpeg(uint8_t* buf, int len, int width, int height, uint8_t* inbuf) {
    size_t encode_size;
    JPEG_Converter::bitmap_buff_info_t bitmap_buff_info;
    JPEG_Converter::encode_options_t encode_options;
    bitmap_buff_info.width = width;
    bitmap_buff_info.height = height;
    bitmap_buff_info.format = JPEG_Converter::WR_RD_YCbCr422;
    bitmap_buff_info.buffer_address = (void *) inbuf;
    encode_options.encode_buff_size = len;
    encode_options.p_EncodeCallBackFunc = NULL;
    encode_options.input_swapsetting = JPEG_Converter::WR_RD_WRSWA_32_16_8BIT;

    encode_size = 0;
    dcache_invalid(buf, len);
    if (Jcu.encode(&bitmap_buff_info, buf, &encode_size, &encode_options)
            != JPEG_Converter::JPEG_CONV_OK) {
        encode_size = 0;
    }

    return encode_size;
}

Camera::Camera(uint16_t width, uint16_t height, uint8_t mode){
	_width = width;
	_height = height;
	_mode = mode;
}

Camera::~Camera(){

}

/* Starts the camera */
void Camera::begin(void)
{
    // Initialize the background to black
    for (int i = 0; i < sizeof(FrameBuffer_Video); i += 2) {
        FrameBuffer_Video[i + 0] = 0x10;
        FrameBuffer_Video[i + 1] = 0x80;
    }

    // Camera
    if(_mode == VIDEO_CVBS){
        DisplayBase::graphics_error_t error;

        EasyAttach_Init(Display);

        error = Display.Graphics_Video_init(DisplayBase::INPUT_SEL_VDEC, NULL);
        if( error != DisplayBase::GRAPHICS_OK ) {
            printf("Line %d, error %d\n", __LINE__, error);
            return;
        }
    } else {
        EasyAttach_Init(Display);
    }

    // Video capture setting (progressive form fixed)
    Display.Video_Write_Setting(
        DisplayBase::VIDEO_INPUT_CHANNEL_0,
        DisplayBase::COL_SYS_NTSC_358,
        (void *)FrameBuffer_Video,
		(((_width * DATA_SIZE_PER_PIC) + 31u) & ~31u),
        VIDEO_FORMAT,
        WR_RD_WRSWA,
        _height,
        _width
    );
    EasyAttach_CameraStart(Display, DisplayBase::VIDEO_INPUT_CHANNEL_0);
}

size_t Camera::createJpeg(){
    return encode_jpeg(JpegBuffer, sizeof(JpegBuffer), _width, _height, FrameBuffer_Video);
}

size_t Camera::createJpeg(uint16_t width, uint16_t height, uint8_t* buf){
    return encode_jpeg(JpegBuffer, sizeof(JpegBuffer), width, height, buf);
}

uint8_t* Camera::getJpegAdr(){
    return JpegBuffer;
}

uint8_t* Camera::getImageAdr(){
	return FrameBuffer_Video;
}

uint16_t Camera::getWidth(){
	return _width;
}
uint16_t Camera::getHeight(){
	return _height;
}


