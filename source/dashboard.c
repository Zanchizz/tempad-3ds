#include "dashboard.h"
#include "timeline.h"
#include <string.h>

// ---------------------------------------------------------
// Internal Dashboard State
// ---------------------------------------------------------
static DashboardTileDef s_dashTiles[DASH_APP_COUNT];
static DashboardActionDef s_dashActions[DASH_ACT_COUNT];
static int s_selectedDashApp = DASH_APP_TIMEDOOR;
static int s_pressedDashTile = -1;
static int s_pressedDashAction = -1;

static int s_lastTappedApp = -1;
static int s_tapTimer = 99;
static char s_statusMessage[64];

void dashboardInit(void) {
    s_selectedDashApp = DASH_APP_TIMEDOOR;
    s_pressedDashTile = -1;
    s_pressedDashAction = -1;
    s_lastTappedApp = -1;
    s_tapTimer = 99;

    // 2x2 Matrix in Left Area (x: 10 to 206)
    const float tileW = 94.0f;
    const float tileH = 76.0f;
    const float col0X = 10.0f;
    const float col1X = 110.0f;
    const float row0Y = 44.0f;
    const float row1Y = 126.0f;

    s_dashTiles[DASH_APP_TIMEDOOR]     = (DashboardTileDef){ col0X, row0Y, tileW, tileH, "TIMEDOOR",     "PRG // 001" };
    s_dashTiles[DASH_APP_MISS_MINUTES] = (DashboardTileDef){ col1X, row0Y, tileW, tileH, "MISS MINUTES", "AI // SYS"  };
    s_dashTiles[DASH_APP_SETTINGS]     = (DashboardTileDef){ col0X, row1Y, tileW, tileH, "SETTINGS",     "CFG // 042" };
    s_dashTiles[DASH_APP_DIRECTORY]    = (DashboardTileDef){ col1X, row1Y, tileW, tileH, "DIRECTORY",    "LOG // TVA" };

    // Action List in Right Area (x: 212 to 310)
    const float actX = 212.0f;
    const float actW = 98.0f;
    const float actH = 38.0f;

    s_dashActions[DASH_ACT_RUN]    = (DashboardActionDef){ actX, 66.0f,  actW, actH, "RUN PROGRAM"    };
    s_dashActions[DASH_ACT_STATUS] = (DashboardActionDef){ actX, 110.0f, actW, actH, "CHECK STATUS"   };
    s_dashActions[DASH_ACT_PURGE]  = (DashboardActionDef){ actX, 154.0f, actW, actH, "PRUNE TIMELINE" };

    dashboardSetStatus("SYSTEM NOMINAL // SACRED TIMELINE");
}

void dashboardUpdate(void) {
    if (s_tapTimer < 100) {
        s_tapTimer++;
    }
}

void dashboardSetStatus(const char* msg) {
    if (msg) {
        strncpy(s_statusMessage, msg, sizeof(s_statusMessage) - 1);
        s_statusMessage[sizeof(s_statusMessage) - 1] = '\0';
    }
}

void dashboardHandleInput(u32 kDown, u32 kHeld, touchPosition touch, AppState* currentState) {
    // 1. Physical 'A' Button
    if (kDown & KEY_A) {
        if (s_selectedDashApp == DASH_APP_TIMEDOOR) {
            *currentState = STATE_TIMEDOOR_SELECT;
            dashboardSetStatus("SUBMENU: SAVED TIMES ACTIVATED");
            return;
        } else if (s_selectedDashApp == DASH_APP_MISS_MINUTES) {
            dashboardSetStatus("MISS MINUTES: 'WELCOME TO THE TEMPAD!'");
        } else if (s_selectedDashApp == DASH_APP_SETTINGS) {
            dashboardSetStatus("SETTINGS: TEMPAD OS v0.2 NOMINAL");
        } else if (s_selectedDashApp == DASH_APP_DIRECTORY) {
            dashboardSetStatus("DIRECTORY: CLASSIFIED TVA FILES");
        }
    }

    // 2. D-Pad Navigation
    if (kDown & KEY_UP) {
        if (s_selectedDashApp >= 2) s_selectedDashApp -= 2;
    }
    if (kDown & KEY_DOWN) {
        if (s_selectedDashApp < 2) s_selectedDashApp += 2;
    }
    if (kDown & KEY_LEFT) {
        if (s_selectedDashApp % 2 == 1) s_selectedDashApp--;
    }
    if (kDown & KEY_RIGHT) {
        if (s_selectedDashApp % 2 == 0) s_selectedDashApp++;
    }

    // 3. Continuous Touch Feedback (kHeld)
    s_pressedDashTile = -1;
    s_pressedDashAction = -1;

    if (kHeld & KEY_TOUCH) {
        for (int i = 0; i < DASH_APP_COUNT; i++) {
            const DashboardTileDef* t = &s_dashTiles[i];
            if (touch.px >= t->x && touch.px <= (t->x + t->w) &&
                touch.py >= t->y && touch.py <= (t->y + t->h)) {
                s_pressedDashTile = i;
                break;
            }
        }
        for (int i = 0; i < DASH_ACT_COUNT; i++) {
            const DashboardActionDef* a = &s_dashActions[i];
            if (touch.px >= a->x && touch.px <= (a->x + a->w) &&
                touch.py >= a->y && touch.py <= (a->y + a->h)) {
                s_pressedDashAction = i;
                break;
            }
        }
    }

    // 4. Touch Trigger Actions (kDown)
    if (kDown & KEY_TOUCH) {
        // Touch on 2x2 Matrix
        for (int i = 0; i < DASH_APP_COUNT; i++) {
            const DashboardTileDef* t = &s_dashTiles[i];
            if (touch.px >= t->x && touch.px <= (t->x + t->w) &&
                touch.py >= t->y && touch.py <= (t->y + t->h)) {

                if (i == DASH_APP_TIMEDOOR && s_lastTappedApp == DASH_APP_TIMEDOOR && s_tapTimer < 25) {
                    *currentState = STATE_TIMEDOOR_SELECT;
                    dashboardSetStatus("DOUBLE TAP: SELECT TIME OPENED");
                    s_lastTappedApp = -1;
                    return;
                }

                s_selectedDashApp = i;
                s_lastTappedApp = i;
                s_tapTimer = 0;

                if (i == DASH_APP_TIMEDOOR) {
                    dashboardSetStatus("TIMEDOOR READY // TAP 'RUN PROGRAM'");
                } else if (i == DASH_APP_MISS_MINUTES) {
                    dashboardSetStatus("MISS MINUTES: 'WELCOME TO THE TEMPAD!'");
                } else if (i == DASH_APP_SETTINGS) {
                    dashboardSetStatus("SETTINGS: INTERFACE CONFIGURATION");
                } else if (i == DASH_APP_DIRECTORY) {
                    dashboardSetStatus("DIRECTORY: TVA VARIANT INDEX");
                }
                break;
            }
        }

        // Touch on Action List
        for (int i = 0; i < DASH_ACT_COUNT; i++) {
            const DashboardActionDef* a = &s_dashActions[i];
            if (touch.px >= a->x && touch.px <= (a->x + a->w) &&
                touch.py >= a->y && touch.py <= (a->y + a->h)) {

                if (i == DASH_ACT_RUN) {
                    if (s_selectedDashApp == DASH_APP_TIMEDOOR) {
                        *currentState = STATE_TIMEDOOR_SELECT;
                        dashboardSetStatus("SUBMENU: SAVED TIMES ACTIVATED");
                    } else if (s_selectedDashApp == DASH_APP_MISS_MINUTES) {
                        dashboardSetStatus("MISS MINUTES: 'WELCOME TO THE TEMPAD!'");
                    } else {
                        dashboardSetStatus("PROGRAM EXECUTED SUCCESSFULLY");
                    }
                } else if (i == DASH_ACT_STATUS) {
                    timelineTriggerBranch();
                    dashboardSetStatus("STATUS: UPDATING TIMELINE...");
                } else if (i == DASH_ACT_PURGE) {
                    timelinePruneAll();
                    dashboardSetStatus("PURGE: RESET CHARGE APPLIED");
                }
                break;
            }
        }
    }
}

void dashboardRender(C2D_TextBuf textBufDynamic) {
    // 1. Top Title Bar
    C2D_Text txtTitle;
    C2D_TextParse(&txtTitle, textBufDynamic, "TEMPAD // TVA MAIN DASHBOARD");
    C2D_TextOptimize(&txtTitle);
    C2D_DrawText(&txtTitle, C2D_WithColor, 12.0f, 10.0f, 0.5f, 0.44f, 0.44f, CLR_AMBER_BRIGHT);

    // Quick Status Telemetry
    C2D_Text txtQuick;
    C2D_TextParse(&txtQuick, textBufDynamic, s_statusMessage);
    C2D_TextOptimize(&txtQuick);
    C2D_DrawText(&txtQuick, C2D_WithColor, 12.0f, 26.0f, 0.5f, 0.35f, 0.35f, CLR_AMBER_DARK);

    // 2. 2x2 Matrix (TIMEDOOR, MISS MINUTES, SETTINGS, DIRECTORY)
    for (int i = 0; i < DASH_APP_COUNT; i++) {
        const DashboardTileDef* t = &s_dashTiles[i];
        bool isSelected = (s_selectedDashApp == i);
        bool isPressed  = (s_pressedDashTile == i);

        float drawX = t->x;
        float drawY = isPressed ? (t->y + 2.0f) : t->y;

        u32 bgCol     = isSelected ? (isPressed ? CLR_AMBER_CORE : CLR_AMBER_BRIGHT) : CLR_BTN_NORMAL;
        u32 borderCol = isSelected ? CLR_AMBER_CORE : (isPressed ? CLR_AMBER_BRIGHT : CLR_BORDER_MID);
        u32 textCol   = isSelected ? CLR_TEXT_INVERTED : CLR_AMBER_TEXT;
        u32 subCol    = isSelected ? C2D_Color32(40, 20, 5, 255) : CLR_AMBER_DARK;

        C2D_DrawRectSolid(drawX, drawY, 0.0f, t->w, t->h, bgCol);
        drawBoxOutline(drawX, drawY, t->w, t->h, 1.5f, borderCol);

        // Stylized Retro Icon / Glyph
        float iconBoxX = drawX + 8.0f;
        float iconBoxY = drawY + 8.0f;
        u32 iconCol = isSelected ? CLR_TEXT_INVERTED : CLR_AMBER_BRIGHT;

        if (i == DASH_APP_TIMEDOOR) {
            drawBoxOutline(iconBoxX, iconBoxY, 18.0f, 22.0f, 1.5f, iconCol);
            C2D_DrawLine(iconBoxX + 9.0f, iconBoxY, iconCol, iconBoxX + 9.0f, iconBoxY + 22.0f, iconCol, 1.0f, 0.0f);
        } else if (i == DASH_APP_MISS_MINUTES) {
            drawBoxOutline(iconBoxX, iconBoxY + 2.0f, 18.0f, 18.0f, 1.5f, iconCol);
            C2D_DrawLine(iconBoxX + 9.0f, iconBoxY + 5.0f, iconCol, iconBoxX + 9.0f, iconBoxY + 11.0f, iconCol, 1.5f, 0.0f);
            C2D_DrawLine(iconBoxX + 9.0f, iconBoxY + 11.0f, iconCol, iconBoxX + 14.0f, iconBoxY + 11.0f, iconCol, 1.5f, 0.0f);
        } else if (i == DASH_APP_SETTINGS) {
            drawBoxOutline(iconBoxX, iconBoxY + 2.0f, 18.0f, 18.0f, 1.0f, iconCol);
            C2D_DrawLine(iconBoxX + 4.0f, iconBoxY + 7.0f, iconCol, iconBoxX + 14.0f, iconBoxY + 7.0f, iconCol, 1.5f, 0.0f);
            C2D_DrawLine(iconBoxX + 4.0f, iconBoxY + 13.0f, iconCol, iconBoxX + 14.0f, iconBoxY + 13.0f, iconCol, 1.5f, 0.0f);
        } else {
            drawBoxOutline(iconBoxX, iconBoxY + 2.0f, 18.0f, 18.0f, 1.5f, iconCol);
            C2D_DrawLine(iconBoxX + 3.0f, iconBoxY + 6.0f, iconCol, iconBoxX + 15.0f, iconBoxY + 6.0f, iconCol, 1.0f, 0.0f);
            C2D_DrawLine(iconBoxX + 3.0f, iconBoxY + 10.0f, iconCol, iconBoxX + 15.0f, iconBoxY + 10.0f, iconCol, 1.0f, 0.0f);
            C2D_DrawLine(iconBoxX + 3.0f, iconBoxY + 14.0f, iconCol, iconBoxX + 10.0f, iconBoxY + 14.0f, iconCol, 1.0f, 0.0f);
        }

        C2D_Text txtTile;
        C2D_TextParse(&txtTile, textBufDynamic, t->title);
        C2D_TextOptimize(&txtTile);
        C2D_DrawText(&txtTile, C2D_WithColor, drawX + 8.0f, drawY + 36.0f, 0.5f, 0.40f, 0.40f, textCol);

        C2D_Text txtSub;
        C2D_TextParse(&txtSub, textBufDynamic, t->code);
        C2D_TextOptimize(&txtSub);
        C2D_DrawText(&txtSub, C2D_WithColor, drawX + 8.0f, drawY + 54.0f, 0.5f, 0.32f, 0.32f, subCol);
    }

    // 3. Action List (Right Column)
    C2D_Text txtActHeader;
    C2D_TextParse(&txtActHeader, textBufDynamic, "ACTIONS //////");
    C2D_TextOptimize(&txtActHeader);
    C2D_DrawText(&txtActHeader, C2D_WithColor, 214.0f, 46.0f, 0.5f, 0.38f, 0.38f, CLR_AMBER_BRIGHT);

    for (int i = 0; i < DASH_ACT_COUNT; i++) {
        const DashboardActionDef* a = &s_dashActions[i];
        bool isPressed = (s_pressedDashAction == i);

        float drawX = a->x;
        float drawY = isPressed ? (a->y + 2.0f) : a->y;

        u32 btnBg     = isPressed ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
        u32 btnBorder = isPressed ? CLR_AMBER_CORE   : CLR_BORDER_MID;
        u32 btnText   = isPressed ? CLR_TEXT_INVERTED: CLR_AMBER_TEXT;

        if (i == DASH_ACT_PURGE) {
            btnBg     = isPressed ? CLR_PURGE_BTN_PRESSED : CLR_PURGE_BTN_NORMAL;
            btnBorder = isPressed ? CLR_AMBER_CORE        : CLR_PURGE_BTN_BORDER;
            btnText   = isPressed ? CLR_TEXT_INVERTED     : C2D_Color32(255, 120, 90, 255);
        }

        C2D_DrawRectSolid(drawX, drawY, 0.0f, a->w, a->h, btnBg);
        drawBoxOutline(drawX, drawY, a->w, a->h, 1.5f, btnBorder);

        C2D_Text txtLabel;
        C2D_TextParse(&txtLabel, textBufDynamic, a->label);
        C2D_TextOptimize(&txtLabel);
        C2D_DrawText(&txtLabel, C2D_WithColor | C2D_AlignCenter,
                     drawX + (a->w / 2.0f), drawY + 11.0f, 0.5f, 0.38f, 0.38f, btnText);
    }

    // 4. Bottom Footer Instructions
    C2D_Text txtFooter;
    C2D_TextParse(&txtFooter, textBufDynamic, "SELECT MODULE OR 'RUN PROGRAM' // [START] EXIT");
    C2D_TextOptimize(&txtFooter);
    C2D_DrawText(&txtFooter, C2D_WithColor | C2D_AlignCenter, 160.0f, 216.0f, 0.5f, 0.35f, 0.35f, CLR_AMBER_DARK);
}

