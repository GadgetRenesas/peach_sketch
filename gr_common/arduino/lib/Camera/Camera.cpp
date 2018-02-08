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

#include "Arduino.h"
#include "Camera.h"
#include "JPEG_Converter.h"
#include "dcache-control.h"
#include "stdlib.h"
#if defined(TARGET_RZ_A1H) || defined(TARGET_GR_LYCHEE)
#include "OV7725_config.h"
#endif
#if defined(TARGET_GR_LYCHEE)
#include "LCD_config_lychee.h"
#elif defined(TARGET_RZ_A1H)
#include "LCD_shield_config_4_3inch.h"
#endif

#define USE_NEWWORKBUF
#define USE_MALLOCMEM

#ifdef USE_MALLOCMEM
static uint8_t* FrameBuffer_Video_pre;
static uint8_t* FrameBuffer_Video; // specified aligned 32bytes address after begin function
#else
static uint8_t FrameBuffer_Video[FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(32)));
#endif
static uint8_t JpegBuffer[1024 * 64]__attribute((aligned(32)));
static uint32_t captured = 0;

/* jpeg convert */
static JPEG_Converter Jcu;
static DisplayBase Display;
DisplayBase::video_ext_in_config_t Camera::ext_in_config;

static size_t encode_jpeg(uint8_t* buf, int len, int width, int height, uint8_t* inbuf, uint8_t format) {
    size_t encode_size;
    uint8_t* work_buf; // for the case of RGB888 & GRAY format

    JPEG_Converter::bitmap_buff_info_t bitmap_buff_info;
    JPEG_Converter::encode_options_t encode_options;
    bitmap_buff_info.width = width;
    bitmap_buff_info.height = height;
    bitmap_buff_info.format = JPEG_Converter::WR_RD_YCbCr422;
    encode_options.encode_buff_size = len;
    encode_options.p_EncodeCallBackFunc = NULL;
    encode_options.input_swapsetting = JPEG_Converter::WR_RD_WRSWA_32_16_8BIT;

    if(format == Camera::FORMAT_RGB888){
#ifdef USE_NEWWORKBUF
    	work_buf = (uint8_t*)malloc(BYTE_PER_PIXEL_YUV * width * height);
#else
    	work_buf = FrameBuffer_Video;
#endif
    	if(work_buf == NULL){ // unable to get working buffer
    		return 0;
    	}
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j+=2) { // processing 2 pixels
                uint8_t r0,g0,b0,r1,g1,b1,y0,y1,u,v;
                r1 = inbuf[(i * width * BYTE_PER_PIXEL_RGB)
                        + BYTE_PER_PIXEL_RGB * j + 5];
                g1 = inbuf[(i * width * BYTE_PER_PIXEL_RGB)
                        + BYTE_PER_PIXEL_RGB * j + 4];
                b1 = inbuf[(i * width * BYTE_PER_PIXEL_RGB)
                        + BYTE_PER_PIXEL_RGB * j + 3];
                r0 = inbuf[(i * width * BYTE_PER_PIXEL_RGB)
                        + BYTE_PER_PIXEL_RGB * j + 2];
                g0 = inbuf[(i * width * BYTE_PER_PIXEL_RGB)
                        + BYTE_PER_PIXEL_RGB * j + 1];
                b0 = inbuf[(i * width * BYTE_PER_PIXEL_RGB)
                        + BYTE_PER_PIXEL_RGB * j + 0];

                y0 = round( 0.256788 * r0 + 0.504129 * g0 + 0.097906 * b0) +  16;
                y1 = round( 0.256788 * r1 + 0.504129 * g1 + 0.097906 * b1) +  16;
                if(r0!=g0){ // it's not gray that means color line
                    u = round(-0.148223 * r0 - 0.290993 * g0 + 0.439216 * b0) + 128;
                    v = round( 0.439216 * r0 - 0.367788 * g0 - 0.071427 * b0) + 128;
                } else if (r1!=g1){ // it's not gray that means color line
                    u = round(-0.148223 * r1 - 0.290993 * g1 + 0.439216 * b1) + 128;
                    v = round( 0.439216 * r1 - 0.367788 * g1 - 0.071427 * b1) + 128;
                } else { // it's gray.
                    u = round(-0.148223 * r0 - 0.290993 * g0 + 0.439216 * b0) + 128;
                    v = round( 0.439216 * r0 - 0.367788 * g0 - 0.071427 * b0) + 128;
                }

                work_buf[(i * width * BYTE_PER_PIXEL_YUV)
                        + BYTE_PER_PIXEL_YUV * j + 0] = y0;
                work_buf[(i * width * BYTE_PER_PIXEL_YUV)
                        + BYTE_PER_PIXEL_YUV * j + 1] = u;
                work_buf[(i * width * BYTE_PER_PIXEL_YUV)
                        + BYTE_PER_PIXEL_YUV * j + 2] = y1;
                work_buf[(i * width * BYTE_PER_PIXEL_YUV)
                        + BYTE_PER_PIXEL_YUV * j + 3] = v;
            }
        }
        bitmap_buff_info.buffer_address = (void *) work_buf;
    } else if (format == Camera::FORMAT_GRAY){
#ifdef USE_NEWWORKBUF
    	work_buf = (uint8_t*)malloc(BYTE_PER_PIXEL_YUV * width * height);
#else
    	work_buf = FrameBuffer_Video;
#endif
    	if(work_buf == NULL){ // unable to get working buffer
    		return 0;
    	}
        for (int i = 0; i < (int)(BYTE_PER_PIXEL_YUV * width * height); i += 2) {
        	work_buf[i + 0] = 0x10;
        	work_buf[i + 1] = 0x80;
        }
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
            	work_buf[(i * width * BYTE_PER_PIXEL_YUV)
                        + BYTE_PER_PIXEL_YUV * j] = inbuf[i * width + j];
            }
        }
        bitmap_buff_info.buffer_address = (void *) work_buf;
    } else {
        bitmap_buff_info.buffer_address = (void *) inbuf;
    }

#ifdef USE_NEWWORKBUF
    dcache_clean(bitmap_buff_info.buffer_address, BYTE_PER_PIXEL_YUV * width * height);
#else
#endif

    encode_size = 0;
    dcache_invalid(buf, len);
    if (Jcu.encode(&bitmap_buff_info, buf, &encode_size, &encode_options)
            != JPEG_Converter::JPEG_CONV_OK) {
        encode_size = 0;
    }

#ifdef USE_NEWWORKBUF
    if(format == Camera::FORMAT_RGB888 || format == Camera::FORMAT_GRAY){
    	free(work_buf);
    }
#endif

    return encode_size;
}

static void IntCallbackFunc_Vfield(DisplayBase::int_type_t int_type) {
	captured++;
}

Camera::Camera(uint16_t width, uint16_t height, uint8_t mode){
	_width = width;
	_height = height;
	_size = _width*_height*BYTE_PER_PIXEL_YUV;
	_mode = mode;
}

Camera::~Camera(){

}

/* Starts the camera */
void Camera::begin(void)
{
#ifdef USE_MALLOCMEM
	// specified aligned 32bytes address.
	FrameBuffer_Video_pre = (uint8_t*)malloc(_width * DATA_SIZE_PER_PIC * _height + 31u);
	uint32_t temp = (uint32_t)FrameBuffer_Video_pre & ~31u;
	FrameBuffer_Video = (uint8_t*)temp;
#endif
    // Initialize the background to black
    for (int i = 0; i < (int)sizeof(FrameBuffer_Video); i += 2) {
        FrameBuffer_Video[i + 0] = 0x10;
        FrameBuffer_Video[i + 1] = 0x80;
    }

    Display.Graphics_init(&LcdCfgTbl_LCD_shield);

    // Camera
    if(_mode == CAMERA_TYPE_CVBS){
#if defined(TARGET_RZ_A1H)
    	Display.Graphics_Video_init(DisplayBase::INPUT_SEL_VDEC, NULL);
#endif
    } else if(_mode == CAMERA_TYPE_WCSHIELD){
#if defined(TARGET_RZ_A1H)
        PinName cmos_camera_pin[11] = {
            /* data pin */
            P2_7, P2_6, P2_5, P2_4, P2_3, P2_2, P2_1, P2_0,
            /* control pin */
            P10_0,      /* DV0_CLK   */
            P1_0,       /* DV0_Vsync */
            P1_1        /* DV0_Hsync */
        };
        DigitalOut pwdn(P3_15);
        DigitalOut rstb(P3_14);

        pwdn = 0;
        rstb = 0;
        Thread::wait(10 + 1);
        rstb = 1;
        Thread::wait(1 + 1);

        /* OV7725 camera input config */
        OV7725_config::Initialise();
        ext_in_config.inp_format     = DisplayBase::VIDEO_EXTIN_FORMAT_BT601; /* BT601 8bit YCbCr format */
        ext_in_config.inp_pxd_edge   = DisplayBase::EDGE_RISING;              /* Clock edge select for capturing data          */
        ext_in_config.inp_vs_edge    = DisplayBase::EDGE_RISING;              /* Clock edge select for capturing Vsync signals */
        ext_in_config.inp_hs_edge    = DisplayBase::EDGE_RISING;              /* Clock edge select for capturing Hsync signals */
        ext_in_config.inp_endian_on  = DisplayBase::OFF;                      /* External input bit endian change on/off       */
        ext_in_config.inp_swap_on    = DisplayBase::OFF;                      /* External input B/R signal swap on/off         */
        ext_in_config.inp_vs_inv     = DisplayBase::SIG_POL_NOT_INVERTED;     /* External input DV_VSYNC inversion control     */
        ext_in_config.inp_hs_inv     = DisplayBase::SIG_POL_NOT_INVERTED;     /* External input DV_HSYNC inversion control     */
        ext_in_config.inp_f525_625   = DisplayBase::EXTIN_LINE_525;           /* Number of lines for BT.656 external input */
        ext_in_config.inp_h_pos      = DisplayBase::EXTIN_H_POS_YCBYCR;       /* Y/Cb/Y/Cr data string start timing to Hsync reference */
        ext_in_config.cap_vs_pos     = 4+21;                                  /* Capture start position from Vsync */
        ext_in_config.cap_hs_pos     = 68;                                    /* Capture start position form Hsync */
        ext_in_config.cap_width  = 640;                                       /* Capture width */
        ext_in_config.cap_height = (unsigned short)(640*_height/_width);    /* Capture heigh */
        Display.Graphics_Video_init( DisplayBase::INPUT_SEL_EXT, &ext_in_config);
        /* camera input port setting */
        Display.Graphics_Dvinput_Port_Init(cmos_camera_pin, 11);
#endif // TARGET_RZ_A1H
    } else {
#if defined(TARGET_RZ_A1H)
        PinName cmos_camera_pin[11] = {
            /* data pin */
            P2_7, P2_6, P2_5, P2_4, P2_3, P2_2, P2_1, P2_0,
            /* control pin */
            P10_0,      /* DV0_CLK   */
            P1_0,       /* DV0_Vsync */
            P1_1        /* DV0_Hsync */
        };
        DigitalOut pwdn(P3_12);
        pwdn = 0;
        Thread::wait(1 + 1);
        /* MT9V111 camera input config */
        ext_in_config.inp_format     = DisplayBase::VIDEO_EXTIN_FORMAT_BT601; /* BT601 8bit YCbCr format */
        ext_in_config.inp_pxd_edge   = DisplayBase::EDGE_RISING;              /* Clock edge select for capturing data          */
        ext_in_config.inp_vs_edge    = DisplayBase::EDGE_RISING;              /* Clock edge select for capturing Vsync signals */
        ext_in_config.inp_hs_edge    = DisplayBase::EDGE_RISING;              /* Clock edge select for capturing Hsync signals */
        ext_in_config.inp_endian_on  = DisplayBase::OFF;                      /* External input bit endian change on/off       */
        ext_in_config.inp_swap_on    = DisplayBase::OFF;                      /* External input B/R signal swap on/off         */
        ext_in_config.inp_vs_inv     = DisplayBase::SIG_POL_NOT_INVERTED;     /* External input DV_VSYNC inversion control     */
        ext_in_config.inp_hs_inv     = DisplayBase::SIG_POL_INVERTED;         /* External input DV_HSYNC inversion control     */
        ext_in_config.inp_f525_625   = DisplayBase::EXTIN_LINE_525;           /* Number of lines for BT.656 external input */
        ext_in_config.inp_h_pos      = DisplayBase::EXTIN_H_POS_CRYCBY;       /* Y/Cb/Y/Cr data string start timing to Hsync reference */
        ext_in_config.cap_vs_pos     = 6;                                     /* Capture start position from Vsync */
        ext_in_config.cap_hs_pos     = 150;                                   /* Capture start position form Hsync */
        ext_in_config.cap_width  = 640;                                       /* Capture width */
        ext_in_config.cap_height = (unsigned short)(640*_height/_width);    /* Capture heigh */
#elif defined(TARGET_GR_LYCHEE)
        PinName cmos_camera_pin[11] = {
            /* data pin */
            P1_0, P1_1, P1_2, P1_3, P1_8, P1_9, P1_10, P1_11,
            /* control pin */
            P7_8,       /* DV0_CLK   */
            P7_9,       /* DV0_Vsync */
            P7_10       /* DV0_Hsync */
        };
        DigitalOut pwdn(P7_11);
        DigitalOut rstb(P2_3);

        pwdn = 0;
        rstb = 0;
        Thread::wait(10 + 1);
        rstb = 1;
        Thread::wait(1 + 1);
        /* OV7725 camera input config */
        OV7725_config::Initialise();

        ext_in_config.inp_format     = DisplayBase::VIDEO_EXTIN_FORMAT_BT601; /* BT601 8bit YCbCr format */
        ext_in_config.inp_pxd_edge   = DisplayBase::EDGE_RISING;              /* Clock edge select for capturing data          */
        ext_in_config.inp_vs_edge    = DisplayBase::EDGE_RISING;              /* Clock edge select for capturing Vsync signals */
        ext_in_config.inp_hs_edge    = DisplayBase::EDGE_RISING;              /* Clock edge select for capturing Hsync signals */
        ext_in_config.inp_endian_on  = DisplayBase::OFF;                      /* External input bit endian change on/off       */
        ext_in_config.inp_swap_on    = DisplayBase::OFF;                      /* External input B/R signal swap on/off         */
        ext_in_config.inp_vs_inv     = DisplayBase::SIG_POL_NOT_INVERTED;     /* External input DV_VSYNC inversion control     */
        ext_in_config.inp_hs_inv     = DisplayBase::SIG_POL_NOT_INVERTED;     /* External input DV_HSYNC inversion control     */
        ext_in_config.inp_f525_625   = DisplayBase::EXTIN_LINE_525;           /* Number of lines for BT.656 external input */
        ext_in_config.inp_h_pos      = DisplayBase::EXTIN_H_POS_YCBYCR;       /* Y/Cb/Y/Cr data string start timing to Hsync reference */
        ext_in_config.cap_vs_pos     = 4+21;                                  /* Capture start position from Vsync */
        ext_in_config.cap_hs_pos     = 68;                                    /* Capture start position form Hsync */
        ext_in_config.cap_width  = 640;                                       /* Capture width */
        ext_in_config.cap_height = (unsigned short)(640*_height/_width);    /* Capture heigh */

#endif
        Display.Graphics_Video_init( DisplayBase::INPUT_SEL_EXT, &ext_in_config);
        /* camera input port setting */
        Display.Graphics_Dvinput_Port_Init(cmos_camera_pin, 11);
    }

    Display.Graphics_Irq_Handler_Set(DisplayBase::INT_TYPE_S0_VFIELD, 0, IntCallbackFunc_Vfield);

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
    /* Video write process start */
    Display.Video_Start(DisplayBase::VIDEO_INPUT_CHANNEL_0);

    /* Video write process stop */
    Display.Video_Stop(DisplayBase::VIDEO_INPUT_CHANNEL_0);

    /* Video write process start */
    Display.Video_Start(DisplayBase::VIDEO_INPUT_CHANNEL_0);
    captured = 0;
}

void Camera::stop(){
    Display.Video_Stop(DisplayBase::VIDEO_INPUT_CHANNEL_0);
    captured = 0;
#if 0
    //todo
    refresh();
#endif
}

void Camera::restart(){
    Display.Video_Start(DisplayBase::VIDEO_INPUT_CHANNEL_0);
}

void Camera::refresh(){
	dcache_invalid(FrameBuffer_Video, _size);
}

size_t Camera::createJpeg(){
	size_t size = encode_jpeg(JpegBuffer, sizeof(JpegBuffer), _width, _height, FrameBuffer_Video, FORMAT_YUV422);
    return size;
}

size_t Camera::createJpeg(uint16_t width, uint16_t height, uint8_t* buf, uint8_t format){
    return encode_jpeg(JpegBuffer, sizeof(JpegBuffer), width, height, buf, format);
}

uint8_t* Camera::getJpegAdr(){
    return JpegBuffer;
}

uint8_t* Camera::getImageAdr(){
	return (uint8_t*)FrameBuffer_Video;
}

uint16_t Camera::getWidth(){
	return _width;
}
uint16_t Camera::getHeight(){
	return _height;
}

bool Camera::isCaptured(){
	if(captured > 1){
		return true;
	}
	return false;
}
