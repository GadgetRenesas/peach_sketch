#include "LcdCfg_lychee.h"

const DisplayBase::lcd_config_t LcdCfgTbl_lychee = {
      DisplayBase::LCD_TYPE_PARALLEL_RGB                                            /* lcd_type             */
    , LCD_INPUT_CLOCK                                                               /* intputClock          */
    , LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_RISING                                                      /* lcd_edge             */
    , (LCD_PIXEL_WIDTH  + LCD_H_FRONT_PORCH + LCD_H_BACK_PORCH)                     /* h_toatal_period      */
    , (LCD_PIXEL_HEIGHT + LCD_V_FRONT_PORCH + LCD_V_BACK_PORCH)                     /* v_toatal_period      */
    , LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , LCD_H_BACK_PORCH                                                              /* h_back_porch         */
    , LCD_V_BACK_PORCH                                                              /* v_back_porch         */
    , DisplayBase::LCD_TCON_PIN_NON                                                 /* h_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* h_sync_port_polarity */
    , 0                                                                             /* h_sync_width         */
    , DisplayBase::LCD_TCON_PIN_NON                                                 /* v_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* v_sync_port_polarity */
    , 0                                                                             /* v_sync_width         */
    , DisplayBase::LCD_TCON_PIN_2                                                   /* de_port              */
    , DisplayBase::SIG_POL_INVERTED                                                 /* de_port_polarity     */
};

