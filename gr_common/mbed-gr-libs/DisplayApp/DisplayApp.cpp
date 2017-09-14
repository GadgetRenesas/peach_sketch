
#include "mbed.h"
#include "DisplayApp.h"

void DisplayApp::display_app_process() {
    int data;
    int pos_x_wk;
    int pos_wk;

    pPcApp = new USBSerial;

    displayThread.set_priority(change_pri);
    while (1) {
        if (pPcApp->readable()) {
            data = pPcApp->getc();
            if (data == '{') {
                pos_seq = POS_SEQ_START;
            } else if (data == 'X') {
                if (pos_seq == POS_SEQ_START) {
                    pos_seq = POS_SEQ_X;
                } else {
                    pos_seq = POS_SEQ_INIT;
                }
            } else if (data == 'Y') {
                if (pos_seq == POS_SEQ_C) {
                    pos_seq = POS_SEQ_Y;
                } else {
                    pos_seq = POS_SEQ_INIT;
                }
            } else if (data == '=') {
                if (pos_seq == POS_SEQ_X) {
                    pos_seq = POS_SEQ_X_POS;
                    pos_wk = 0;
                } else if (pos_seq == POS_SEQ_Y) {
                    pos_seq = POS_SEQ_Y_POS;
                    pos_wk = 0;
                } else {
                    pos_seq = POS_SEQ_INIT;
                }
            } else if (data == '-') {
                if (pos_seq == POS_SEQ_X_POS) {
                    pos_seq = POS_SEQ_X_M;
                } else if (pos_seq == POS_SEQ_Y_POS) {
                    pos_seq = POS_SEQ_Y_M;
                } else {
                    pos_seq = POS_SEQ_INIT;
                }
            } else if ((data >= '0') && (data <= '9')) {
                if ((pos_seq == POS_SEQ_X_POS) || (pos_seq == POS_SEQ_Y_POS)) {
                    pos_wk = (pos_wk * 10) + (data - '0');
                } else if ((pos_seq == POS_SEQ_X_M) && (data == '1')) {
                    pos_wk = -1;
                } else if ((pos_seq == POS_SEQ_Y_M) && (data == '1')) {
                    pos_wk = -1;
                } else {
                    pos_seq = POS_SEQ_INIT;
                }
            } else if (data == ',') {
                if ((pos_seq == POS_SEQ_X_POS) || (pos_seq == POS_SEQ_X_M)) {
                    pos_x_wk = pos_wk;
                    pos_seq = POS_SEQ_C;
                } else {
                    pos_seq = POS_SEQ_INIT;
                }
            } else if (data == '}') {
                if ((pos_seq == POS_SEQ_Y_POS) || (pos_seq == POS_SEQ_Y_M)) {
                    pos_seq = POS_SEQ_END;
                    if ((pos_x != pos_x_wk) || (pos_y != pos_wk)) {
                        pos_x = pos_x_wk;
                        pos_y = pos_wk;
                        event.call();
                    }
                } else {
                    pos_seq = POS_SEQ_INIT;
                }
            } else {
                pos_seq = POS_SEQ_INIT;
            }
        } else {
            Thread::wait(10);
        }
    }
}

/* static */void DisplayApp::display_app_process_static(void const * arg) {
    ((DisplayApp *)arg)->display_app_process();
}

DisplayApp::DisplayApp(osPriority tsk_pri, osPriority init_pri, uint32_t stack_size) : change_pri(tsk_pri), displayThread(DisplayApp::display_app_process_static, (void *)this, init_pri, stack_size) {
}

void DisplayApp::SendHeader(uint32_t size) {
    uint8_t headder_data[12] = {0xFF,0xFF,0xAA,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    headder_data[8]  = (uint8_t)((uint32_t)size >> 0);
    headder_data[9]  = (uint8_t)((uint32_t)size >> 8);
    headder_data[10] = (uint8_t)((uint32_t)size >> 16);
    headder_data[11] = (uint8_t)((uint32_t)size >> 24);
    pPcApp->writeBlock((uint8_t *)headder_data, sizeof(headder_data));
}

void DisplayApp::SendData(uint8_t * buf, uint32_t size) {
    int send_size;
    int send_index = 0;

    while (size > 0) {
        if (size > MAX_PACKET_SIZE_EPBULK) {
            send_size = MAX_PACKET_SIZE_EPBULK;
        } else {
            send_size = size;
        }
        pPcApp->writeBlock(&buf[send_index], send_size);
        send_index += send_size;
        size -= send_size;
    }
}

int DisplayApp::SendRgb888(uint8_t * buf, uint32_t pic_width, uint32_t pic_height) {
    uint32_t offset_size = 54;
    uint32_t buf_stride = (((pic_width * 4u) + 31u) & ~31u);
    uint32_t pic_size = buf_stride * pic_height;
    uint32_t total_size = pic_size + offset_size;
    uint8_t wk_bitmap_buf[54];
    int wk_idx = 0;

    if (pPcApp == NULL) {
        return 0;
    }
    if (pPcApp->_putc(0) == 0) {  // terminal connect check
        return 0;
    }
    SendHeader(total_size);

    /* BITMAPFILEHEADER */
    wk_bitmap_buf[wk_idx++] = 'B';
    wk_bitmap_buf[wk_idx++] = 'M';
    wk_bitmap_buf[wk_idx++] = (uint8_t)(total_size >> 0);   /* bfSize */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(total_size >> 8);   /* bfSize */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(total_size >> 16);  /* bfSize */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(total_size >> 24);  /* bfSize */
    wk_bitmap_buf[wk_idx++] = 0;  /* bfReserved1 */
    wk_bitmap_buf[wk_idx++] = 0;  /* bfReserved1 */
    wk_bitmap_buf[wk_idx++] = 0;  /* bfReserved2 */
    wk_bitmap_buf[wk_idx++] = 0;  /* bfReserved2 */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(offset_size >> 0);   /* bfOffBits */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(offset_size >> 8);   /* bfOffBits */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(offset_size >> 16);  /* bfOffBits */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(offset_size >> 24);  /* bfOffBits */

    /* BITMAPINFOHEADER */
    wk_bitmap_buf[wk_idx++] = 40; /* biSize */
    wk_bitmap_buf[wk_idx++] = 0;  /* biSize */
    wk_bitmap_buf[wk_idx++] = 0;  /* biSize */
    wk_bitmap_buf[wk_idx++] = 0;  /* biSize */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(pic_width >> 0);    /* biWidth */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(pic_width >> 8);    /* biWidth */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(pic_width >> 16);   /* biWidth */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(pic_width >> 24);   /* biWidth */
    wk_bitmap_buf[wk_idx++] = (uint8_t)((-(long)pic_height) >> 0);   /* biHeight */
    wk_bitmap_buf[wk_idx++] = (uint8_t)((-(long)pic_height) >> 8);   /* biHeight */
    wk_bitmap_buf[wk_idx++] = (uint8_t)((-(long)pic_height) >> 16);  /* biHeight */
    wk_bitmap_buf[wk_idx++] = (uint8_t)((-(long)pic_height) >> 24);  /* biHeight */
    wk_bitmap_buf[wk_idx++] = 1;  /* biPlanes */
    wk_bitmap_buf[wk_idx++] = 0;  /* biPlanes */
    wk_bitmap_buf[wk_idx++] = 32; /* biBitCount */
    wk_bitmap_buf[wk_idx++] = 0;  /* biBitCount */

    wk_bitmap_buf[wk_idx++] = 0;  /* biCopmression */
    wk_bitmap_buf[wk_idx++] = 0;  /* biCopmression */
    wk_bitmap_buf[wk_idx++] = 0;  /* biCopmression */
    wk_bitmap_buf[wk_idx++] = 0;  /* biCopmression */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(pic_size >> 0);   /* biSizeImage */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(pic_size >> 8);   /* biSizeImage */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(pic_size >> 16);  /* biSizeImage */
    wk_bitmap_buf[wk_idx++] = (uint8_t)(pic_size >> 24);  /* biSizeImage */
    wk_bitmap_buf[wk_idx++] = 0;  /* biXPixPerMeter */
    wk_bitmap_buf[wk_idx++] = 0;  /* biXPixPerMeter */
    wk_bitmap_buf[wk_idx++] = 0;  /* biXPixPerMeter */
    wk_bitmap_buf[wk_idx++] = 0;  /* biXPixPerMeter */
    wk_bitmap_buf[wk_idx++] = 0;  /* biYPixPerMeter */
    wk_bitmap_buf[wk_idx++] = 0;  /* biYPixPerMeter */
    wk_bitmap_buf[wk_idx++] = 0;  /* biYPixPerMeter */
    wk_bitmap_buf[wk_idx++] = 0;  /* biYPixPerMeter */

    wk_bitmap_buf[wk_idx++] = 0;  /* biClrUsed */
    wk_bitmap_buf[wk_idx++] = 0;  /* biClrUsed */
    wk_bitmap_buf[wk_idx++] = 0;  /* biClrUsed */
    wk_bitmap_buf[wk_idx++] = 0;  /* biClrUsed */
    wk_bitmap_buf[wk_idx++] = 0;  /* biCirImportant */
    wk_bitmap_buf[wk_idx++] = 0;  /* biCirImportant */
    wk_bitmap_buf[wk_idx++] = 0;  /* biCirImportant */
    wk_bitmap_buf[wk_idx++] = 0;  /* biCirImportant */
    pPcApp->writeBlock(wk_bitmap_buf, wk_idx);

    SendData(buf, pic_size);
    wk_idx += pic_size;

    return wk_idx;
};

int DisplayApp::SendJpeg(uint8_t * buf, uint32_t size) {
    if (pPcApp == NULL) {
        return 0;
    }
    if (pPcApp->_putc(0) == 0) {  // terminal connect check
        return 0;
    }
    SendHeader(size);
    SendData(buf, size);

    return size;
}

int DisplayApp::GetMaxTouchNum(void) {
    return 1;
}

int DisplayApp::GetCoordinates(int touch_buff_num, touch_pos_t * p_touch) {
    touch_pos_t * wk_touch;
    int count = 0;
    int x = pos_x;
    int y = pos_y;

    if (touch_buff_num > 0) {
        count = 0;
        wk_touch        = &p_touch[0];
        wk_touch->valid = false;
        wk_touch->x     = 0;
        wk_touch->y     = 0;
        if (x >= 0) {
            count = 1;
            wk_touch->valid = true;
            wk_touch->x = (uint32_t)x;
        }
        if (y >= 0) {
            count = 1;
            wk_touch->valid = true;
            wk_touch->y = (uint32_t)y;
        }
    }

    return count;
}
