////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2011 Kentaro Sekimoto  All rights reserved.
////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include "ST7735.h"
#include "font6x8.h"

void SPISW_Initialize(void);
void SPISW_Reset(UINT32 v);
void SPISW_LCD_cmd(UINT8 dat);
void SPISW_LCD_dat(UINT8 dat);

#define COLOR_MODE	16

#define ST7735_OFSX     2
#define ST7735_OFSY     1
#define ST7735_PWX      (128 + 4)
#define ST7735_PWY      160
#define ST7735_SX       2
#define ST7735_SY       2
#define ST7735_WX       128
#define ST7735_WY       160
#define ST7735_BITSPERPIXEL 16
#define ST7735_FCOL     0xFFFFFF
#define ST7735_BCOL     0x000000

#define FONT_WX         6
#define FONT_WY         8
#define TEXT_SX         ST7735_SX
#define TEXT_SY         ST7735_SY
#define LINE_WX         (ST7735_WX/FONT_WX)
#define LINE_WY         (ST7735_WY/FONT_WY)
// SCROLL AREA 4 lines unit
#define SCROLL_SY       1       // screen top
#define SCROLL_EY       31      // screen bottom
#define SCROLL_WY       29      // screen width
#define SCROLL_FLAG     0       // bottom scroll

static int cx = 0;     // character x position
static int cy = 0;     // character y position

static void delay_ms(volatile UINT32 n)
{
    HAL_Time_Sleep_MicroSeconds_InterruptEnabled(n * 1000);
}

void ST7735B_Initialize()
{
    SPISW_Initialize();
    delay_ms(100);
    SPISW_Reset(0);
    delay_ms(100);
    SPISW_Reset(1);
    delay_ms(100);

    SPISW_LCD_cmd(0x01);    // SWRESET
    delay_ms(50);
    SPISW_LCD_cmd(0x11);    // SLPOUT
    delay_ms(500);
    SPISW_LCD_cmd(0x3a);    // COLMOD
    SPISW_LCD_dat(0x05);    // 16 bit color
    SPISW_LCD_cmd(0xB1);    // FRMCTR1
    SPISW_LCD_dat(0x00);    // fastest refresh
    SPISW_LCD_dat(0x06);    // 6 lines front porch
    SPISW_LCD_dat(0x03);    // 3 lines back porch
    delay_ms(10);
    SPISW_LCD_cmd(0x36);    // memory access control
    SPISW_LCD_dat(0x08);
    SPISW_LCD_cmd(0xB6);    // DISSET5
    SPISW_LCD_dat(0x15);    // 1 clock cycle nonoverlap, 2 cycle gate rise, 3 cycle oscil. equalize
    SPISW_LCD_dat(0x02);    // fix on VTL
    SPISW_LCD_cmd(0xB4);    // display inversion control
    SPISW_LCD_dat(0x00);
    SPISW_LCD_cmd(0xc0);    // power control 1
    SPISW_LCD_dat(0x02);
    SPISW_LCD_dat(0x70);
    delay_ms(10);
    SPISW_LCD_cmd(0xc1);    // power control 2
    SPISW_LCD_dat(0x05);
    SPISW_LCD_dat(0xc2);    // power control 3
    SPISW_LCD_dat(0x01);
    SPISW_LCD_dat(0x02);
    SPISW_LCD_cmd(0xc5);    // voltage control
    SPISW_LCD_dat(0x3c);    // VCOMH = 4V
    SPISW_LCD_dat(0x38);    // VCOML = -1.1.V

    SPISW_LCD_cmd(0xe0);    // GMCTRP1
    SPISW_LCD_dat(0x0f);
    SPISW_LCD_dat(0x1a);
    SPISW_LCD_dat(0x0f);
    SPISW_LCD_dat(0x18);
    SPISW_LCD_dat(0x2f);
    SPISW_LCD_dat(0x28);
    SPISW_LCD_dat(0x20);
    SPISW_LCD_dat(0x22);
    SPISW_LCD_dat(0x1f);
    SPISW_LCD_dat(0x1b);
    SPISW_LCD_dat(0x23);
    SPISW_LCD_dat(0x37);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x07);
    SPISW_LCD_dat(0x02);
    SPISW_LCD_dat(0x10);

    SPISW_LCD_cmd(0xe1);    // GMCTRN1
    SPISW_LCD_dat(0x0f);
    SPISW_LCD_dat(0x1b);
    SPISW_LCD_dat(0x0f);
    SPISW_LCD_dat(0x17);
    SPISW_LCD_dat(0x33);
    SPISW_LCD_dat(0x2c);
    SPISW_LCD_dat(0x29);
    SPISW_LCD_dat(0x2e);
    SPISW_LCD_dat(0x30);
    SPISW_LCD_dat(0x30);
    SPISW_LCD_dat(0x39);
    SPISW_LCD_dat(0x3f);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x07);
    SPISW_LCD_dat(0x03);
    SPISW_LCD_dat(0x10);

    SPISW_LCD_cmd(0x2a);    // CASET
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x02);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x81);

    SPISW_LCD_cmd(0x2B);    // RASET
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x02);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0xa0);

    SPISW_LCD_cmd(0x13);   // NORON
    delay_ms(10);
    SPISW_LCD_cmd(0x2c);   // RAMWR
    delay_ms(500);
    SPISW_LCD_cmd(0x29);   //Display on
    delay_ms(500);
}

void ST7735R_Initialize()
{
    SPISW_Initialize();
    delay_ms(100);
    SPISW_Reset(0);
    delay_ms(100);
    SPISW_Reset(1);
    delay_ms(100);

    SPISW_LCD_cmd(0x01);    // SWRESET
    delay_ms(150);
    SPISW_LCD_cmd(0x11);    // SLPOUT
    delay_ms(500);
    //SPISW_LCD_cmd(0x3a);    // COLMOD
    //SPISW_LCD_dat(0x05);    // 16 bit color
    SPISW_LCD_cmd(0xB1);    // FRMCTR1
    SPISW_LCD_dat(0x01);    //
    SPISW_LCD_dat(0x2c);    //
    SPISW_LCD_dat(0x2d);    //
    SPISW_LCD_cmd(0xB2);    // FRMCTR2
    SPISW_LCD_dat(0x01);    //
    SPISW_LCD_dat(0x2c);    //
    SPISW_LCD_dat(0x2d);    //
    SPISW_LCD_cmd(0xB2);    // FRMCTR3
    SPISW_LCD_dat(0x01);    //
    SPISW_LCD_dat(0x2c);    //
    SPISW_LCD_dat(0x2d);    //
    SPISW_LCD_dat(0x01);    //
    SPISW_LCD_dat(0x2c);    //
    SPISW_LCD_dat(0x2d);    //
    //delay_ms(10);
    //SPISW_LCD_cmd(0x36);    // memory access control
    //SPISW_LCD_dat(0x08);
    //SPISW_LCD_cmd(0xB6);    // DISSET5
    //SPISW_LCD_dat(0x15);    // 1 clock cycle nonoverlap, 2 cycle gate rise, 3 cycle oscil. equalize
    //SPISW_LCD_dat(0x02);    // fix on VTL
    SPISW_LCD_cmd(0xB4);    // display inversion control
    SPISW_LCD_dat(0x07);
    SPISW_LCD_cmd(0xc0);    // power control 1
    SPISW_LCD_dat(0xa2);
    SPISW_LCD_dat(0x02);
    SPISW_LCD_dat(0x84);
    SPISW_LCD_cmd(0xc1);    // power control 2
    SPISW_LCD_dat(0xc5);
    SPISW_LCD_dat(0xc2);    // power control 3
    SPISW_LCD_dat(0x0a);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0xc3);    // power control 4
    SPISW_LCD_dat(0x8a);
    SPISW_LCD_dat(0x2a);
    SPISW_LCD_dat(0xc4);    // power control 5
    SPISW_LCD_dat(0x8a);
    SPISW_LCD_dat(0xee);
    SPISW_LCD_cmd(0xc5);    // voltage control
    SPISW_LCD_dat(0x0e);    //
    SPISW_LCD_cmd(0x20);    // INVOFF
    SPISW_LCD_cmd(0x36);    // memory access control
    SPISW_LCD_dat(0xc8);
    SPISW_LCD_cmd(0x3a);    // COLMOD
    SPISW_LCD_dat(0x05);    // 16 bit color

    SPISW_LCD_cmd(0x2a);    // CASET
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x7f);

    SPISW_LCD_cmd(0x00);    // RASET
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x9f);


    SPISW_LCD_cmd(0xe0);    // GMCTRP1
    SPISW_LCD_dat(0x0f);
    SPISW_LCD_dat(0x1a);
    SPISW_LCD_dat(0x0f);
    SPISW_LCD_dat(0x18);
    SPISW_LCD_dat(0x2f);
    SPISW_LCD_dat(0x28);
    SPISW_LCD_dat(0x20);
    SPISW_LCD_dat(0x22);
    SPISW_LCD_dat(0x1f);
    SPISW_LCD_dat(0x1b);
    SPISW_LCD_dat(0x23);
    SPISW_LCD_dat(0x37);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x07);
    SPISW_LCD_dat(0x02);
    SPISW_LCD_dat(0x10);

    SPISW_LCD_cmd(0xe1);    // GMCTRN1
    SPISW_LCD_dat(0x0f);
    SPISW_LCD_dat(0x1b);
    SPISW_LCD_dat(0x0f);
    SPISW_LCD_dat(0x17);
    SPISW_LCD_dat(0x33);
    SPISW_LCD_dat(0x2c);
    SPISW_LCD_dat(0x29);
    SPISW_LCD_dat(0x2e);
    SPISW_LCD_dat(0x30);
    SPISW_LCD_dat(0x30);
    SPISW_LCD_dat(0x39);
    SPISW_LCD_dat(0x3f);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(0x07);
    SPISW_LCD_dat(0x03);
    SPISW_LCD_dat(0x10);

    SPISW_LCD_cmd(0x29);   //Display on
    delay_ms(100);
    SPISW_LCD_cmd(0x13);   // NORON
    delay_ms(10);
    //SPISW_LCD_cmd(0x2c);   // RAMWR
    //delay_ms(500);
    //SPISW_LCD_cmd(0x29);   //Display on
    //delay_ms(500);
}

void ST7735_Initialize()
{
    ST7735R_Initialize();
}

// color = 12-bit color value rrrrggggbbbb
void ST7735_Clear()
{
    UINT32 x;
    SPISW_LCD_cmd(CASET);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(ST7735_OFSX);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(ST7735_WX + ST7735_OFSX);
    SPISW_LCD_cmd(PASET);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(ST7735_OFSY);
    SPISW_LCD_dat(0x00);
    SPISW_LCD_dat(ST7735_WY + ST7735_OFSY);
    SPISW_LCD_cmd(RAMWR);
#if (COLOR_MODE == 12)
    for (x = 0; x < ((ST7735_PWX * ST7735_PWY)/2); x++){
    	SPISW_LCD_dat(0);
        SPISW_LCD_dat(0);
        SPISW_LCD_dat(0);
    }
#elif (COLOR_MODE == 16)
    for (x = 0; x < (ST7735_PWX * ST7735_PWY); x++){
    	SPISW_LCD_dat(0);
    	SPISW_LCD_dat(0);
    }
#else
    for (x = 0; x < (ST7735_PWX * ST7735_PWY); x++){
    	SPISW_LCD_dat(0);
        SPISW_LCD_dat(0);
        SPISW_LCD_dat(0);
    }
#endif
}

// RGB 565 format x2 => RG BR GB 44 44 44 format
// v1: rrrrrggg gggbbbbb
// v2: rrrrrggg gggbbbbb
#define R4G4(v1)        ((UINT8)(((v1 & 0xf000) >> 8) | ((v1 & 0x07e0) >> 7)))
#define B4R4(v1, v2)    ((UINT8)(((v1 & 0x1f) << 3) | (v2 >> 12)))
#define G4B4(v2)        ((UINT8)(((v2 & 0x07e0) >> 3) | ((v2 & 0x1f) >> 1)))

void ST7735_BitBltEx565(int x, int y, int width, int height, UINT32 data[])
{
    int i, j;
    UINT16 v1, v2;
    UINT16 *pdata = (UINT16 *)data;
    //SPISW_LCD_cmd(DATCTL);  // The DATCTL command selects the display mode (8-bit or 12-bit).
    for (j = 0; j < height; j ++) {
        SPISW_LCD_cmd(PASET);
        SPISW_LCD_dat(0x00);
        SPISW_LCD_dat(y + j + ST7735_OFSY);
        SPISW_LCD_dat(0x00);
        SPISW_LCD_dat(y + j + 1 + ST7735_OFSY);
#if (COLOR_MODE == 12)
        for (i = 0; i < width; i += 2) {
            SPISW_LCD_cmd(CASET);
            SPISW_LCD_dat(0x00);
            SPISW_LCD_dat(x + i);
            SPISW_LCD_dat(0x00);
            SPISW_LCD_dat(x + i + 1);
            v1 = *pdata++;
            v2 = *pdata++;
            SPISW_LCD_cmd(RAMWR);
            SPISW_LCD_dat(R4G4(v1));
            SPISW_LCD_dat(B4R4(v1, v2));
            SPISW_LCD_dat(G4B4(v2));
        }
#elif (COLOR_MODE == 16)
        SPISW_LCD_cmd(CASET);
        SPISW_LCD_dat(0x00);
        SPISW_LCD_dat(x + ST7735_OFSX);
        SPISW_LCD_dat(0x00);
        SPISW_LCD_dat(x + width + ST7735_OFSX);
        SPISW_LCD_cmd(RAMWR);
        for (i = 0; i < width; i += 1) {
            v1 = *(pdata + i);
            SPISW_LCD_dat((UINT8)(v1 >> 8));
            SPISW_LCD_dat((UINT8)(v1));
        }
        pdata += ST7735_PWX;
#else
#endif
    }
}

void ST7735_BitBltEx(int x, int y, int width, int height, UINT32 data[])
{
    UINT16 *pdata = (UINT16 *)data;
    pdata += (y * ST7735_PWX + x);
    ST7735_BitBltEx565(x, y, width, height, (UINT32 *)pdata);
}

void ST7735_WriteChar_Color(unsigned char c, UINT8 cx, UINT8 cy, UINT16 fgcol, UINT16 bgcol)
{
    UINT8 x, y;
    UINT16    col0, col1;

    char *font;
    if (c >= 0x80)
        c = 0;
    font = &font6x8[(c & 0x00ff) << 3];
    for (y = 0; y < FONT_WY; y++) {
        SPISW_LCD_cmd(PASET);    //y set
        SPISW_LCD_dat(0x00);
        SPISW_LCD_dat(cy * FONT_WY + y + TEXT_SY);
        SPISW_LCD_dat(0x00);
        SPISW_LCD_dat(ST7735_WY - 1);
        SPISW_LCD_cmd(CASET);
        SPISW_LCD_dat(0x00);
        SPISW_LCD_dat(cx * FONT_WX + TEXT_SX);
        SPISW_LCD_dat(0x00);
        SPISW_LCD_dat(ST7735_WX - 1);
        SPISW_LCD_cmd(RAMWR);
#if (COLOR_MODE == 12)
        for (x = 0; x < (FONT_WX / 2); x++) {
            if (font[y] & (0x80 >> (x * 2))) {
                col0 = fgcol;
            } else {
                col0 = bgcol;
            }
            if (font[y] & (0x40 >> (x * 2))) {
                col1 = fgcol;
            } else {
                col1 = bgcol;
            }
            SPISW_LCD_dat((0xff & (UINT8)(col0 >> 4)));
            SPISW_LCD_dat((0xf0 & (UINT8)(col0 << 4)) | (0x0f & ((UINT8)(col1 >> 8))));
            SPISW_LCD_dat((UINT8)(0xff & col1));
        }
#elif (COLOR_MODE == 16)
        for (x = 0; x < FONT_WX; x++) {
            if (font[y] & (0x80 >> x)) {
                col0 = fgcol;
            } else {
                col0 = bgcol;
            }
            SPISW_LCD_dat((UINT8)(col0 >> 8));
            SPISW_LCD_dat((UINT8)(col0));
        }
#else
#endif
    }
}

void ST7735_WriteChar(unsigned char c, int cx, int cy)
{
    ST7735_WriteChar_Color(c, (UINT8)cx, (UINT8)cy, ST7735_FCOL, ST7735_BCOL);
}

void ST7735_WriteFormattedChar(unsigned char ch)
{
    int x;
    if (ch == 0xc) {
        ST7735_Clear();
        cx = 0;
        cy = 0;
    } else if (ch == '\n') {
        cy++;
        if (cy == ST7735_WY/FONT_WY) {
            cy = 0;
        }
    } else if (ch == '\r') {
        cx = 0;
    } else {
        if (cx == 0)
            for (x = 0; x < ST7735_WX/FONT_WX; x++)
                ST7735_WriteChar(0x20, x, cy);
        ST7735_WriteChar(ch, cx, cy);
        cx ++;
        if (cx == ST7735_WX/FONT_WX) {
            cx = 0;
            cy ++;
            if (cy == ST7735_WY/FONT_WY) {
                cy = 0;
            }
        }
    }
}

BOOL LCD_Initialize()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    ST7735_Initialize();
    LCD_Clear();
    return true;
}

BOOL LCD_Uninitialize()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return true;
}

void LCD_Clear()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    cx = 0;
    cy = 0;
    ST7735_Clear();
}

void LCD_BitBltEx(int x, int y, int width, int height, UINT32 data[])
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    ST7735_BitBltEx(x, y, width, height, data);
}

void LCD_BitBltEx565(int x, int y, int width, int height, UINT32 data[])
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    ST7735_BitBltEx565(x, y, width, height, data);
}

void LCD_BitBlt(int width, int height, int widthInWords, UINT32 data[], BOOL fUseDelta)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    ST7735_BitBltEx(0, 0, width, height, data);
}

void LCD_WriteChar(unsigned char c, int row, int col)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    ST7735_WriteChar(c, row, col);
}

void LCD_WriteFormattedChar(unsigned char c)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    ST7735_WriteFormattedChar(c);
}

void LCD_WriteFormattedChar16(unsigned short s)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    ST7735_WriteFormattedChar((char)s);
}

INT32 LCD_GetWidth()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return ST7735_WX;
}

INT32 LCD_GetHeight()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return ST7735_WY;
}

INT32 LCD_GetBitsPerPixel()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return ST7735_BITSPERPIXEL;
}

UINT32 LCD_GetPixelClockDivider()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return 0;
}
INT32 LCD_GetOrientation()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return 0;
}

void LCD_PowerSave(BOOL On)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
}

UINT32 *LCD_GetFrameBuffer()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return NULL;
}

UINT32 LCD_ConvertColor(UINT32 color)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return color;
}

//--//
