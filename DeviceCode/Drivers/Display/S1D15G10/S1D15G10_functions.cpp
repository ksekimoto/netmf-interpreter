////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2011 Kentaro Sekimoto  All rights reserved.
////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include "S1D15G10.h"
#include "font6x8.h"

void SPISW_Initialize(void);
void SPISW_LCD_cmd(UINT8 dat);
void SPISW_LCD_dat(UINT8 dat);

#define S1D15G10_PWX    132
#define S1D15G10_PWY    132
#define S1D15G10_SX     2
#define S1D15G10_SY     4
#define S1D15G10_WX     128
#define S1D15G10_WY     128
#define S1D15G10_BITSPERPIXEL   12
#define S1D15G10_FCOL   0xFFFFFF
#define S1D15G10_BCOL   0x000000

#define FONT_WX         6
#define FONT_WY         8
#define TEXT_SX         S1D15G10_SX
#define TEXT_SY         S1D15G10_SY
#define LINE_WX         (S1D15G10_WX/FONT_WX)
#define LINE_WY         (S1D15G10_WY/FONT_WY)
// SCROLL AREA 4 lines unit
#define SCROLL_SY       1       // screen top
#define SCROLL_EY       31      // screen bottom
#define SCROLL_WY       29      // screen width
#define SCROLL_FLAG     0       // bottom scroll

int cx = 0;     // character x position
int cy = 0;     // character y position

static void delay_ms(volatile UINT32 n)
{
    HAL_Time_Sleep_MicroSeconds_InterruptEnabled(n * 1000);
}

void S1D15G10_Initialize()
{
    SPISW_Initialize();
    delay_ms(200);
    SPISW_LCD_cmd(DISCTL);  // Display Control
    SPISW_LCD_dat(0x0d);    // 0x00 = 2 divisions, switching period=8 (default)
    SPISW_LCD_dat(0x20);    // 0x20 = nlines/4 - 1 = 132/4 - 1 = 32)
    SPISW_LCD_dat(0x00);    // 0x00 = no inversely highlighted lines
    SPISW_LCD_cmd(COMSCN);  // common output scan direction
    SPISW_LCD_dat(0x01);    // 0x01 = Scan 1->80, 160<-81
    SPISW_LCD_cmd(OSCON);   // oscillators on and get out of sleep mode.
    SPISW_LCD_cmd(SLPOUT);  // sleep out
    SPISW_LCD_cmd(PWRCTR);  // reference voltage regulator on, circuit voltage follower on, BOOST ON
    SPISW_LCD_dat(0xf);     // everything on, no external reference resistors
    SPISW_LCD_cmd(DISNOR);  // display mode
    SPISW_LCD_cmd(DISINV);  // display mode
    SPISW_LCD_cmd(PTLOUT);  // Partial area off
    SPISW_LCD_cmd(DATCTL);  // The DATCTL command selects the display mode (8-bit or 12-bit).
    SPISW_LCD_dat(0x00);    // 0x01 = page address inverted, col address normal, address scan in col direction
    SPISW_LCD_dat(0x00);    // 0x00 = RGB sequence (default value)
    SPISW_LCD_dat(0x02);    // 0x02 = Grayscale -> 16 (selects 12-bit color, type A)
    SPISW_LCD_cmd(VOLCTR);  // The contrast is set by the Electronic Volume Command (VOLCTR).
    SPISW_LCD_dat(28);      // 32 volume value (adjust this setting for your display 0 .. 63)
    SPISW_LCD_dat(0);       // 3 resistance ratio (determined by experiment)
    SPISW_LCD_cmd(TMPGRD);  // Temprature gradient
    SPISW_LCD_dat(0);       // default value
    delay_ms(100);
    SPISW_LCD_dat(0);
    SPISW_LCD_cmd(DISON);   // display on
}

// color = 12-bit color value rrrrggggbbbb
void S1D15G10_Clear()
{
    UINT32 x;
    SPISW_LCD_cmd(PASET);
    SPISW_LCD_dat(0);
    SPISW_LCD_dat(S1D15G10_PWX-1);
    SPISW_LCD_cmd(CASET);
    SPISW_LCD_dat(0);
    SPISW_LCD_dat(S1D15G10_PWY-1);
    SPISW_LCD_cmd(RAMWR);
    for (x = 0; x < ((S1D15G10_PWX * S1D15G10_PWY)/2); x++){
        SPISW_LCD_dat(0);
        SPISW_LCD_dat(0);
        SPISW_LCD_dat(0);
    }
    SPISW_LCD_cmd(SCSTART);
    SPISW_LCD_dat(0);
}

// RGB 565 format x2 => RG BR GB 44 44 44 format
// v1: rrrrrggg gggbbbbb
// v2: rrrrrggg gggbbbbb
#define R4G4(v1)        ((UINT8)(((v1 & 0xf000) >> 8) | ((v1 & 0x07e0) >> 7)))
#define B4R4(v1, v2)    ((UINT8)(((v1 & 0x1f) << 3) | (v2 >> 12)))
#define G4B4(v2)        ((UINT8)(((v2 & 0x07e0) >> 3) | ((v2 & 0x1f) >> 1)))

void S1D15G10_BitBltEx565(int x, int y, int width, int height, UINT32 data[])
{
    int i, j;
    UINT16 v1, v2;
    UINT16 *pdata = (UINT16 *)data;
    SPISW_LCD_cmd(DATCTL);  // The DATCTL command selects the display mode (8-bit or 12-bit).
    for (j = 0; j < height; j ++) {
        SPISW_LCD_cmd(PASET);
        SPISW_LCD_dat(y + j);
        SPISW_LCD_dat(y + j + 1);
        for (i = 0; i < width; i += 2) {
            SPISW_LCD_cmd(CASET);
            SPISW_LCD_dat(x + i);
            SPISW_LCD_dat(x + i + 1);
            v1 = *pdata++;
            v2 = *pdata++;
            SPISW_LCD_cmd(RAMWR);
            SPISW_LCD_dat(R4G4(v1));
            SPISW_LCD_dat(B4R4(v1, v2));
            SPISW_LCD_dat(G4B4(v2));
        }
    }
}

void S1D15G10_BitBltEx(int x, int y, int width, int height, UINT32 data[])
{
    int i, j;
    UINT16 v1, v2;
    UINT16 *pdata = (UINT16 *)data;
    pdata += (y * S1D15G10_PWX + x);
    S1D15G10_BitBltEx565(x, y, width, height, (UINT32 *)pdata);
}

void S1D15G10_WriteChar_Color(unsigned char c, UINT16 cx, UINT16 cy, UINT16 fgcol, UINT16 bgcol)
{
    UINT16 x, y;
    UINT16    col0, col1;

    char *font;
    if (c >= 0x80)
        c = 0;
    font = &font6x8[(c & 0x00ff) << 3];
    for (y = 0; y < FONT_WY; y++) {
        SPISW_LCD_cmd(PASET);    //y set
        SPISW_LCD_dat(cy * FONT_WY + y + TEXT_SY);
        SPISW_LCD_dat(S1D15G10_WY - 1);
        SPISW_LCD_cmd(CASET);
        SPISW_LCD_dat(cx * FONT_WX + TEXT_SX);
        SPISW_LCD_dat(S1D15G10_WX - 1);
        SPISW_LCD_cmd(RAMWR);
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
    }
}

void S1D15G10_WriteChar(unsigned char c, int cx, int cy)
{
    S1D15G10_WriteChar_Color(c, cx, cy, S1D15G10_FCOL, S1D15G10_BCOL);
}

void S1D15G10_WriteFormattedChar(unsigned char ch)
{
    if (ch == 0xc) {
        S1D15G10_Clear();
        cx = 0;
        cy = 0;
    } else if (ch == '\n') {
        cy++;
        if (cy == S1D15G10_WY/FONT_WY) {
            cy = 0;
        }
    } else if (ch == '\r') {
        cx = 0;
    } else {
        S1D15G10_WriteChar(ch, cx, cy);
        cx ++;
        if (cx == S1D15G10_WX/FONT_WX) {
            cx = 0;
            cy ++;
            if (cy == S1D15G10_WY/FONT_WY) {
                cy = 0;
            }
        }
    }
}

BOOL LCD_Initialize()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    S1D15G10_Initialize();
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
    S1D15G10_Clear();
}

void LCD_BitBltEx(int x, int y, int width, int height, UINT32 data[])
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    S1D15G10_BitBltEx(x, y, width, height, data);
}

void LCD_BitBltEx565(int x, int y, int width, int height, UINT32 data[])
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    S1D15G10_BitBltEx565(x, y, width, height, data);
}

void LCD_BitBlt(int width, int height, int widthInWords, UINT32 data[], BOOL fUseDelta)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    S1D15G10_BitBltEx(0, 0, width, height, data);
}

void LCD_WriteChar(unsigned char c, int row, int col)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    S1D15G10_WriteChar(c, row, col);
}

void LCD_WriteFormattedChar(unsigned char c)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    S1D15G10_WriteFormattedChar(c);
}

void LCD_WriteFormattedChar16(unsigned short s)
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    S1D15G10_WriteFormattedChar((char)s);
}

INT32 LCD_GetWidth()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return S1D15G10_WX;
}

INT32 LCD_GetHeight()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return S1D15G10_WY;
}

INT32 LCD_GetBitsPerPixel()
{
    NATIVE_PROFILE_HAL_DRIVERS_DISPLAY();
    return S1D15G10_BITSPERPIXEL;
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
