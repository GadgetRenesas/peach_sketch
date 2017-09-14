/*
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
 
#ifndef RGA_FUNC_H
#define RGA_FUNC_H

#include "RGA.h"

typedef struct {
    char * style;
    int x;
    int y;
    int w;
    int h;
} draw_rectangle_pos_t;

#define IMG_DRAW_WIDTH                      (144)
#define IMG_DRAW_HEIGHT                     (108)
#define IMAGE_WIDTH_SCROLL_FUNC             (211)      // Image width of Scroll function
#define IMAGE_HEIGHT_SCROLL_FUNC            (139)      // Image height of Scroll function
#define IMAGE_WIDTH_ZOOM_FUNC               (211)      // Image width of Zoom function
#define IMAGE_HEIGHT_ZOOM_FUNC              (139)      // Image height of Zoom function

#define DISSOLVE_MAX_NUM                    (256)
#define SCROLL_MAX_NUM                      (IMAGE_WIDTH_SCROLL_FUNC - IMG_DRAW_WIDTH)
#define ZOOM_MAX_NUM                        (IMAGE_HEIGHT_ZOOM_FUNC / 2)
#define ROTATION_MAX_NUM                    (360)
#define ACCELERATE_MAX_NUM                  (256)

#define ANIMATION_TIMING_EASE               (0)
#define ANIMATION_TIMING_LINEAR             (1)
#define ANIMATION_TIMING_EASE_IN            (2)
#define ANIMATION_TIMING_EASE_OUT           (3)
#define ANIMATION_TIMING_EASE_IN_OUT        (4)

extern void Set_RGAObject(frame_buffer_t* frmbuf_info);
extern void RGA_Func_DrawTopScreen(frame_buffer_t* frmbuf_info);
extern void RGA_Func_DrawRectangle(frame_buffer_t* frmbuf_info, draw_rectangle_pos_t * pos, int pos_num);
extern void RGA_Func_DrawImage(frame_buffer_t* frmbuf_info, int x, int y);
extern void RGA_Func_Dissolve(frame_buffer_t* frmbuf_info, float32_t global_alpha);
extern void RGA_Func_Scroll(frame_buffer_t* frmbuf_info, int src_width_pos);
extern void RGA_Func_Zoom(frame_buffer_t* frmbuf_info, int src_height_pos);
extern void RGA_Func_Rotation(frame_buffer_t* frmbuf_info, graphics_matrix_float_t image_angle);
extern void RGA_Func_Accelerate(frame_buffer_t* frmbuf_info, int animation_timing, float32_t relative_pos);

#endif
