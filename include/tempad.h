#ifndef TEMPAD_H
#define TEMPAD_H

#include <3ds.h>
#include <citro2d.h>

// ---------------------------------------------------------
// Screen Dimensions (Nintendo 3DS)
// ---------------------------------------------------------
#define TOP_WIDTH        400
#define TOP_HEIGHT       240
#define BOTTOM_WIDTH     320
#define BOTTOM_HEIGHT    240

// ---------------------------------------------------------
// Canonical TVA Color Palette (Amber CRT & Sacred Timeline)
// ---------------------------------------------------------
// Backgrounds & Panels
#define CLR_BG_DARK             C2D_Color32(12, 8, 4, 255)
#define CLR_PANEL_BG            C2D_Color32(22, 15, 9, 255)
#define CLR_PANEL_INSET         C2D_Color32(14, 9, 5, 255)
#define CLR_BORDER_DIM          C2D_Color32(65, 40, 16, 255)
#define CLR_BORDER_MID          C2D_Color32(110, 68, 24, 255)
#define CLR_GRID                C2D_Color32(36, 22, 8, 140)

// Sacred Timeline (Canonical Bright White / Light Gray)
#define CLR_TIMELINE_MAIN       C2D_Color32(235, 235, 240, 255)
#define CLR_TIMELINE_CORE       C2D_Color32(255, 255, 255, 255)
#define CLR_TIMELINE_GLOW       C2D_Color32(180, 185, 205, 110)

// TVA Amber / Orange (UI & Variant Branches)
#define CLR_AMBER_BRIGHT        C2D_Color32(255, 195, 45, 255)
#define CLR_AMBER_CORE          C2D_Color32(255, 245, 190, 255)
#define CLR_AMBER_GLOW          C2D_Color32(255, 125, 0, 110)
#define CLR_AMBER_TEXT          C2D_Color32(255, 175, 35, 255)
#define CLR_AMBER_DARK          C2D_Color32(120, 65, 15, 255)
#define CLR_TEXT_INVERTED       C2D_Color32(18, 10, 4, 255)

// Critical Nexus Alert (Redline)
#define CLR_CRITICAL_RED        C2D_Color32(245, 45, 25, 255)
#define CLR_RED_GLOW            C2D_Color32(220, 35, 15, 130)

// Buttons & Controls
#define CLR_BTN_NORMAL          C2D_Color32(46, 28, 15, 255)
#define CLR_BTN_NORMAL_BORDER   C2D_Color32(140, 85, 25, 255)
#define CLR_BTN_PRESSED         C2D_Color32(255, 150, 10, 255)
#define CLR_BTN_PRESSED_BORDER  C2D_Color32(255, 225, 120, 255)

#define CLR_PURGE_BTN_NORMAL    C2D_Color32(65, 20, 14, 255)
#define CLR_PURGE_BTN_BORDER    C2D_Color32(180, 45, 20, 255)
#define CLR_PURGE_BTN_PRESSED   C2D_Color32(245, 45, 25, 255)

// ---------------------------------------------------------
// UI State Machine
// ---------------------------------------------------------
typedef enum {
    STATE_DASHBOARD,
    STATE_TIMEDOOR_SELECT,
    STATE_TIMEDOOR_ACTIVE
} AppState;

// ---------------------------------------------------------
// Shared Drawing Primitive Helper
// ---------------------------------------------------------
static inline void drawBoxOutline(float x, float y, float w, float h, float thickness, u32 col) {
    C2D_DrawLine(x, y, col, x + w, y, col, thickness, 0.0f);             // Top
    C2D_DrawLine(x, y + h, col, x + w, y + h, col, thickness, 0.0f);     // Bottom
    C2D_DrawLine(x, y, col, x, y + h, col, thickness, 0.0f);             // Left
    C2D_DrawLine(x + w, y, col, x + w, y + h, col, thickness, 0.0f);     // Right
}

#endif // TEMPAD_H

