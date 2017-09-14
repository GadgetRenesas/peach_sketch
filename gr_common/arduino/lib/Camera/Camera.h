/*
 Camera.h - Camera library for GR-LYCHEE
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

#ifndef CAMERA_IF_H_
#define CAMERA_IF_H_

#include "mbed.h"
#include "DisplayBace.h"

/* Video input and LCD layer 0 output */
#define VIDEO_FORMAT           (DisplayBase::VIDEO_FORMAT_YCBCR422)
#define GRAPHICS_FORMAT        (DisplayBase::GRAPHICS_FORMAT_YCBCR422)
#define WR_RD_WRSWA            (DisplayBase::WR_RD_WRSWA_32_16BIT)
#define DATA_SIZE_PER_PIC      (2u)

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
 in accordance with the frame buffer burst transfer mode. */
#define VIDEO_PIXEL_HW         (640u)  /* VGA */
#define VIDEO_PIXEL_VW         (480u)  /* VGA */

#define FRAME_BUFFER_STRIDE    (((VIDEO_PIXEL_HW * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (VIDEO_PIXEL_VW)

class Camera {
public:
	typedef enum {
		VIDEO_CVBS = 0,
		VIDEO_CMOS_CAMERA
	};
	Camera(uint16_t width = VIDEO_PIXEL_HW, uint16_t height = VIDEO_PIXEL_VW, uint8_t mode = VIDEO_CMOS_CAMERA);
	~Camera();
	void begin();
	size_t createJpeg();
	uint8_t* getJpegAdr();
	uint8_t* getImageAdr();
	uint16_t getWidth();
	uint16_t getHeight();

private:
	uint16_t _width;
	uint16_t _height;
	uint8_t _mode;

};

#endif /* Camera_h */
