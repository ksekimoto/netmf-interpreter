////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _DRIVERS_DISPLAY_SED15E0_FONT12X12_CPP_
#define _DRIVERS_DISPLAY_SED15E0_FONT12X12_CPP_ 1


#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rodata = "font_12x12"
#endif

#include "font12x12.h"

#define FONT_BITS   12  // 12x12
#define FONT_BYTES  (((FONT_BITS + 7) / 8) * FONT_BITS)

#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rodata
#endif

/*
 *  Object:
 *      ffont               EXPORTED function
 *
 *      Returns the glyph for a character
 *
 * The font file is slightly compressed, this function knows where
 * the holes are. See comment above.
 */
const unsigned char* Font_GetGlyph(unsigned char c)
{
    c &= 0xFF;

//    if     (c > 0x1F && c < 0x7F) c = c - ' ';
//    else if(c > 0x9F            ) c = c - ' ' - (0xA0 - 0x7F);
//    else                          c = 0; /* non-printable --> space */

    return &font_6x12[c*12];
}

const unsigned char* Font_GetGlyph16(unsigned short u)
{
    unsigned short i;
    unsigned short fidx;
    unsigned char *p;
    unsigned short tblH = u / CUNIFONT_TBL_SIZE;
    unsigned short tblL = u % CUNIFONT_TBL_SIZE;
    unsigned char mask = (unsigned char)(1 << (u&7));

    if (CUniFontMap[tblH][tblL/8] & mask)  {
        fidx = CUniFontIdx[tblH];
        for (i = 0; i < tblL; i++) {
            mask = (1 << (i & 7));
            if (CUniFontMap[tblH][i/8] & mask) {
                fidx ++;
            }
        }
        p = (unsigned char*)&font_12x12[0] + (fidx * FONT_BYTES);
    }
    return p;
}

int Font_Width()
{
    return 6;
}

int Font_Height()
{
    return 12;
}

int Font_TabWidth()
{
    return 4;
}

#endif
