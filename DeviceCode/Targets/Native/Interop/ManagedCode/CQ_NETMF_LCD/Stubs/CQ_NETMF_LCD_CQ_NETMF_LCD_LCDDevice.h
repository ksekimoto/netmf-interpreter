//-----------------------------------------------------------------------------
//
//                   ** WARNING! ** 
//    This file was generated automatically by a tool.
//    Re-running the tool will overwrite this file.
//    You should copy this file to a custom location
//    before adding any customization in the copy to
//    prevent loss of your changes when the tool is
//    re-run.
//
//-----------------------------------------------------------------------------


#ifndef _CQ_NETMF_LCD_CQ_NETMF_LCD_LCDDEVICE_H_
#define _CQ_NETMF_LCD_CQ_NETMF_LCD_LCDDEVICE_H_

namespace CQ
{
    namespace NETMF
    {
        namespace LCD
        {
            struct LCDDevice
            {
                // Helper Functions to access fields of managed object
                // Declaration of stubs. These functions are implemented by Interop code developers
                static void LCDDevice_LCD_Initialize( HRESULT &hr );
                static void LCDDevice_LCD_Clear( HRESULT &hr );
                static void LCDDevice_LCD_BitBltEx( UINT32 param0, UINT32 param1, UINT32 param2, UINT32 param3, CLR_RT_TypedArray_UINT8 param4, HRESULT &hr );
                static void LCDDevice_LCD_WriteFormattedChar( UINT16 param0, HRESULT &hr );
            };
        }
    }
}
#endif  //_CQ_NETMF_LCD_CQ_NETMF_LCD_LCDDEVICE_H_
