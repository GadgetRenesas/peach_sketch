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

#include "mbed.h"
#include "rga_func.h"
#include "BinaryImage_RZ_A1H.h"

#define ZOOM_SRC_CENTER_X                   (IMAGE_WIDTH_ZOOM_FUNC / 2)
#define ZOOM_SRC_CENTER_Y                   (IMAGE_HEIGHT_ZOOM_FUNC / 2)

#define CRYSTAL_DIAMETER                    (55)

Canvas2D_ContextClass canvas2d;

static animation_timing_function_t* accelerator;

void Set_RGAObject(frame_buffer_t* frmbuf_info) {
    errnum_t err;
    Canvas2D_ContextConfigClass config;

    config.frame_buffer = frmbuf_info;
    canvas2d = R_RGA_New_Canvas2D_ContextClass(config);
    err = R_OSPL_GetErrNum();
    if (err != 0) {
        printf("Line %d, error %d\n", __LINE__, err);
        while (1);
    }
}

void RGA_Func_DrawTopScreen(frame_buffer_t* frmbuf_info) {
    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    /* Draw a image */
    canvas2d.drawImage(TopScrn_jpg_File, 0, 0);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_DrawRectangle(frame_buffer_t* frmbuf_info, draw_rectangle_pos_t * pos, int pos_num) {
    int cnt;

    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    /* Draw description */
    canvas2d.drawImage(decs_swipe_jpg_File, 138, 240);

    /* Draw rectangle */
    for (cnt = 0; cnt < pos_num; cnt++) {
        canvas2d.fillStyle = pos->style;
        canvas2d.fillRect(pos->x, pos->y, pos->w, pos->h);
        pos++;
    }

    /* Draw return button */
    canvas2d.drawImage(RetBtn_jpg_File, 384, 6);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_DrawImage(frame_buffer_t* frmbuf_info, int x, int y) {
    int draw_pos_x = x - (CRYSTAL_DIAMETER / 2);
    int draw_pos_y = y - (CRYSTAL_DIAMETER / 2);

    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    /* Draw description */
    canvas2d.drawImage(decs_swipe_jpg_File, 138, 240);

    /* Draw the first image */
    canvas2d.drawImage(Flower_jpg_File, 60, 65);

    /* Draw the second image */
    if ((frmbuf_info->pixel_format == PIXEL_FORMAT_ARGB8888) || (frmbuf_info->pixel_format == PIXEL_FORMAT_XRGB8888)) {
        canvas2d.drawImage(Crystal_png_ARGB8888, draw_pos_x, draw_pos_y);
    } else if (frmbuf_info->pixel_format != PIXEL_FORMAT_YUV422) {
        canvas2d.drawImage(Crystal_png_ARGB4444, draw_pos_x, draw_pos_y);
    } else {
        canvas2d.drawImage(Crystal_png_File, draw_pos_x, draw_pos_y);
    }

    /* Draw return button */
    canvas2d.drawImage(RetBtn_jpg_File, 384, 6);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_Dissolve(frame_buffer_t* frmbuf_info, float32_t global_alpha) {
    /* Dissolve = Fade Out + Fade In */
    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    /* Draw description */
    canvas2d.drawImage(decs_swipe_side_jpg_File, 84, 240);

    /* fade Out */
    canvas2d.globalAlpha = 1.0f - global_alpha;
    canvas2d.drawImage(Flower_jpg_File, 60, 65);

    /* fade In */
    canvas2d.globalAlpha = global_alpha;
    canvas2d.drawImage(Landscape_jpg_File, 60, 65);

    canvas2d.globalAlpha = 1.0f;

    /* Draw return button */
    canvas2d.drawImage(RetBtn_jpg_File, 384, 6);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_Scroll(frame_buffer_t* frmbuf_info, int src_width_pos) {
    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    /* Draw description */
    canvas2d.drawImage(decs_swipe_side_jpg_File, 84, 240);

    /* Scroll from left to right */
    canvas2d.drawImage(Landscape_wide_jpg_File,
                        /* src */       src_width_pos, 15, IMG_DRAW_WIDTH, IMG_DRAW_HEIGHT,
                        /* dst */       60, 65, IMG_DRAW_WIDTH, IMG_DRAW_HEIGHT);

    /* Draw return button */
    canvas2d.drawImage(RetBtn_jpg_File, 384, 6);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_Zoom(frame_buffer_t* frmbuf_info, int src_height_pos) {
    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    /* Draw description */
    canvas2d.drawImage(decs_swipe_side_jpg_File, 84, 240);

    /* Zoom out */
    canvas2d.drawImage(Landscape_wide_jpg_File,
                        /* src X */     ZOOM_SRC_CENTER_X - (src_height_pos * IMG_DRAW_WIDTH / IMG_DRAW_HEIGHT),
                        /* src Y */     ZOOM_SRC_CENTER_Y - src_height_pos,
                        /* src W */     src_height_pos * 2 * IMG_DRAW_WIDTH / IMG_DRAW_HEIGHT,
                        /* src H */     src_height_pos * 2,
                        /* dst   */     60, 65, IMG_DRAW_WIDTH, IMG_DRAW_HEIGHT);

    /* Draw return button */
    canvas2d.drawImage(RetBtn_jpg_File, 384, 6);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_Rotation(frame_buffer_t* frmbuf_info, graphics_matrix_float_t image_angle) {
    /* Rotate the image to the right */
    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    /* Draw description */
    canvas2d.drawImage(decs_swipe_side_jpg_File, 84, 240);

    /* Move to drawing position */
    canvas2d.translate((60 + (IMG_DRAW_WIDTH / 2)), (65 + (IMG_DRAW_HEIGHT / 2)));

    /* Rotate */
    canvas2d.rotate(image_angle * (3.14159 / 180));

    /* Move to center to rotate */
    canvas2d.translate((-IMG_DRAW_WIDTH / 2), (-IMG_DRAW_HEIGHT / 2));
    canvas2d.drawImage(Flower_jpg_File, 0, 0);
    canvas2d.setTransform(1, 0, 0, 1, 0, 0);

    /* Draw return button */
    canvas2d.drawImage(RetBtn_jpg_File, 384, 6);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_Accelerate(frame_buffer_t* frmbuf_info, int animation_timing, float32_t relative_pos) {
    /* Do accelerated motion from right to left */
    int position;

    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    if (relative_pos == 0.0f) {
        switch (animation_timing) {
            case ANIMATION_TIMING_EASE:
                R_Get_AnimationTimingFunction("ease", &accelerator);
                break;
            case ANIMATION_TIMING_LINEAR:
                R_Get_AnimationTimingFunction("linear", &accelerator);
                break;
            case ANIMATION_TIMING_EASE_IN:
                R_Get_AnimationTimingFunction("ease-in", &accelerator);
                break;
            case ANIMATION_TIMING_EASE_OUT:
                R_Get_AnimationTimingFunction("ease-out", &accelerator);
                break;
            case ANIMATION_TIMING_EASE_IN_OUT:
            default:
                R_Get_AnimationTimingFunction("ease-in-out", &accelerator);
                break;
        }
    }

    /* Draw rectangle */
    canvas2d.fillStyle = "#0000FF";  /* blue */
    switch (animation_timing) {
        case ANIMATION_TIMING_EASE:
            canvas2d.fillRect(10, 235, 81, 6);
            break;
        case ANIMATION_TIMING_LINEAR:
            canvas2d.fillRect(103, 235, 81, 6);
            break;
        case ANIMATION_TIMING_EASE_IN:
            canvas2d.fillRect(198, 235, 81, 6);
            break;
        case ANIMATION_TIMING_EASE_OUT:
            canvas2d.fillRect(292, 235, 81, 6);
            break;
        case ANIMATION_TIMING_EASE_IN_OUT:
        default:
            canvas2d.fillRect(386, 235, 81, 6);
            break;
    }

    /* Draw button */
    canvas2d.drawImage(animetion_timing_jpg_File, 0, 205);

    position = R_ANIMATION_TIMING_FUNCTION_GetValue(accelerator, relative_pos, 330.0f, 6.0f);
    canvas2d.drawImage(Flower_jpg_File, position, 65);

    /* Draw return button */
    canvas2d.drawImage(RetBtn_jpg_File, 384, 6);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}
