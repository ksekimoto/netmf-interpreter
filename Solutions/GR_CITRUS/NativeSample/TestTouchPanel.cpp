////////////////////////////////////////////////////////////////////////////
// Portions Copyright (c) 2010-2011 Kentaro Sekimoto  All rights reserved.
////////////////////////////////////////////////////////////////////////////
/*
 * TestTouchPanel.c
 *
 *  Created on: 2011/12/17
 *      Author: Kentaro Sekimoto
 */
#ifndef __GNUC__
#define __GNUC__
#endif
#include <tinyhal.h>

#ifdef TEST_TOUCHPANEL
volatile int count;

void TouchGetPoint(void)
{
    TOUCH_PANEL_SAMPLE_FLAGS flags = 100;
    int src = 0;
    int px = 0;
    int py = 0;
    HAL_TOUCH_PANEL_GetPoint(&flags, &src, &px, &py);
    lcd_printf("x=%4d  y=%4d\r\n", px, py);
    count --;
}

void TestTouchPanel_old(void)
{
    count = 5;
    Buttons_Initialize();
    HAL_TOUCH_PANEL_Enable((GPIO_INTERRUPT_SERVICE_ROUTINE)TouchGetPoint);
    while (1) {
#if 1
        TOUCH_PANEL_SAMPLE_FLAGS flags = 100;
        int src = 0;
        int px = 0;
        int py = 0;
        HAL_TOUCH_PANEL_GetPoint(&flags, &src, &px, &py);
        lcd_printf("x=%4d  y=%4d\r\n", px, py);
        count --;
#endif
        if (count <= 0)
            break;
    }
    HAL_TOUCH_PANEL_Disable();
    Buttons_Uninitialize();
}

int touch_count;

void PostManagedEvent(UINT8 category, UINT8 subCategory, UINT16 data1, UINT32 data2)
{
//    UINT32 flags;
//    UINT16 source;
    UINT16 x;
    UINT16 y;
//    INT64 time;
//    HRESULT hr;
    UINT32 location = (*(TouchPoint*)data2).location;
    x = location & 0x3FFF;
    y = (location >> 14) & 0x3FFF;
    if ((category == EVENT_TOUCH) && (subCategory == TouchPanelStylusDown)) {
//        hr = TOUCH_PANEL_GetTouchPoint(&flags, &source, &x, &y, &time);
//        lcd_printf("TOUCH_PANEL_GetTouchPoint() hr=%d x=%d, y=%d\r\n", hr, x, y);
        lcd_printf("x=%04d, y=%04d\r\n", x, y);
    }
    if ((category == EVENT_TOUCH) && (subCategory == TouchPanelStylusUp)) {
        touch_count ++;
    }
}

void TestTouchPanel(void)
{
    HRESULT hr;
    touch_count = 0;
    Buttons_Initialize();
    hr = TOUCH_PANEL_Initialize();
    lcd_printf("Initialize() hr=%d\r\n", hr);
//    hr = TOUCH_PANEL_ResetCalibration();
//    lcd_printf("ResetCalibration() hr=%d\r\n", hr);
    while (1) {
        UINT32 Events = Events_WaitForEvents(SYSTEM_EVENT_FLAG_BUTTON, 10);
        if (Events & SYSTEM_EVENT_FLAG_BUTTON) {
            UINT32 ButtonsPressed;
            UINT32 ButtonsReleased;
            Events_Clear(SYSTEM_EVENT_FLAG_BUTTON);
            while (Buttons_GetNextStateChange(ButtonsPressed, ButtonsReleased))
                ;
            if (ButtonsPressed)
                lcd_printf("ButtonPressed = 0x%04X\r\n", ButtonsPressed);
            if (ButtonsReleased)
                lcd_printf("ButtonReleased = 0x%04X\r\n", ButtonsReleased);
            if (ButtonsReleased & BUTTON_B0) {
                lcd_printf("Exiting\r\n");
                break;
            }
        }
        if (touch_count > 4)
            break;
    }
    hr = TOUCH_PANEL_Uninitialize();
    lcd_printf("Uninitialize() hr=%d\r\n", hr);
    Buttons_Uninitialize();
}
#endif
