#ifndef TEMPAD_H
#define TEMPAD_H

#include <3ds.h>
#include <citro2d.h>
#include "tva_style.h"

// ---------------------------------------------------------
// Aliases to Centralized Style Palette (include/tva_style.h)
// ---------------------------------------------------------
#define CLR_BG_DARK             COLOR_BG_DARK
#define CLR_PANEL_BG            COLOR_PANEL_BG
#define CLR_PANEL_INSET         COLOR_PANEL_INSET
#define CLR_BORDER_DIM          COLOR_BORDER_DIM
#define CLR_BORDER_MID          COLOR_BORDER_MID
#define CLR_GRID                COLOR_GRID_LINE

#define CLR_TIMELINE_MAIN       COLOR_SACRED_MAIN
#define CLR_TIMELINE_CORE       COLOR_SACRED_CORE
#define CLR_TIMELINE_GLOW       COLOR_SACRED_GLOW

#define CLR_AMBER_BRIGHT        COLOR_BRANCH_AMBER
#define CLR_AMBER_CORE          COLOR_BRANCH_SELECTED
#define CLR_AMBER_GLOW          COLOR_BRANCH_AMBER_GLOW
#define CLR_AMBER_TEXT          COLOR_AMBER_TEXT
#define CLR_AMBER_DARK          COLOR_AMBER_DIM
#define CLR_TEXT_INVERTED       COLOR_TEXT_INVERTED

#define CLR_CRITICAL_RED        COLOR_BRANCH_RED
#define CLR_RED_GLOW            COLOR_BRANCH_RED_GLOW

#define CLR_BTN_NORMAL          COLOR_BTN_NORMAL
#define CLR_BTN_NORMAL_BORDER   COLOR_BTN_NORMAL_BORDER
#define CLR_BTN_PRESSED         COLOR_BTN_PRESSED
#define CLR_BTN_PRESSED_BORDER  COLOR_BTN_PRESSED_BORDER

#define CLR_PURGE_BTN_NORMAL    COLOR_PURGE_BTN_NORMAL
#define CLR_PURGE_BTN_BORDER    COLOR_PURGE_BTN_BORDER
#define CLR_PURGE_BTN_PRESSED   COLOR_PURGE_BTN_PRESSED

// ---------------------------------------------------------
// UI State Machine
// ---------------------------------------------------------
typedef enum {
    STATE_DASHBOARD,
    STATE_TIMEDOOR_SELECT,
    STATE_TIMEDOOR_ACTIVE
} AppState;

// ---------------------------------------------------------
// Shared TVA Custom Font & Drawing Primitives
// ---------------------------------------------------------
extern C2D_Font g_tvaFont;

static inline void tempadParseText(C2D_Text* text, C2D_TextBuf buf, const char* str) {
    if (g_tvaFont != NULL) {
        C2D_TextFontParse(text, g_tvaFont, buf, str);
    } else {
        C2D_TextParse(text, buf, str);
    }
    C2D_TextOptimize(text);
}

static inline void drawBoxOutline(float x, float y, float w, float h, float thickness, u32 col) {
    C2D_DrawLine(x, y, col, x + w, y, col, thickness, 0.0f);             // Top
    C2D_DrawLine(x, y + h, col, x + w, y + h, col, thickness, 0.0f);     // Bottom
    C2D_DrawLine(x, y, col, x, y + h, col, thickness, 0.0f);             // Left
    C2D_DrawLine(x + w, y, col, x + w, y + h, col, thickness, 0.0f);     // Right
}

#endif // TEMPAD_H
