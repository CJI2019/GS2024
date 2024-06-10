#pragma once

#include "stdafx.h"
#include "resource.h"

// Color constants.
const COLORREF rgbRed = 0x000000FF;
const COLORREF rgbGreen = 0x0000FF00;
const COLORREF rgbBlue = 0x00FF0000;
const COLORREF rgbBlack = 0x00000000;
const COLORREF rgbWhite = 0x00FFFFFF;


HFONT GetFont() {
    // 폰트 생성
    static HFONT hFont = CreateFont(
        42,                        // Height
        0,                         // Width
        0,                         // Escapement
        0,                         // Orientation
        FW_NORMAL,                 // Weight
        FALSE,                     // Italic
        FALSE,                     // Underline
        FALSE,                     // StrikeOut
        DEFAULT_CHARSET,           // CharSet
        OUT_DEFAULT_PRECIS,        // OutPrecision
        CLIP_DEFAULT_PRECIS,       // ClipPrecision
        DEFAULT_QUALITY,           // Quality
        DEFAULT_PITCH | FF_SWISS,  // PitchAndFamily
        _T("Arial"));              // FaceName

    return hFont;
}