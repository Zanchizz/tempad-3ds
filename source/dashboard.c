#include "dashboard.h"
#include "assets.h"
#include "timeline.h"
#include <stdio.h>
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
static int s_actionFlashIdx = -1;
static int s_actionFlashTimer = 0;
static char s_statusMessage[64];

// ---------------------------------------------------------
// Helper de Texto con Glow CRT (4 pasadas de halo + nucleo nitido)
// ---------------------------------------------------------
static void drawGlowText(C2D_Text* txt, u32 flags, float x, float y, float z,
                         float scaleX, float scaleY, u32 haloCol, u32 coreCol) {
    u32 drawFlags = flags | C2D_WithColor;
    C2D_DrawText(txt, drawFlags, x - 1.0f, y,        z, scaleX, scaleY, haloCol);
    C2D_DrawText(txt, drawFlags, x + 1.0f, y,        z, scaleX, scaleY, haloCol);
    C2D_DrawText(txt, drawFlags, x,        y - 1.0f, z, scaleX, scaleY, haloCol);
    C2D_DrawText(txt, drawFlags, x,        y + 1.0f, z, scaleX, scaleY, haloCol);
    C2D_DrawText(txt, drawFlags, x,        y,        z, scaleX, scaleY, coreCol);
}

void dashboardInit(void) {
    s_selectedDashApp = DASH_APP_TIMEDOOR;
    s_pressedDashTile = -1;
    s_pressedDashAction = -1;
    s_lastTappedApp = -1;
    s_tapTimer = 99;
    s_actionFlashIdx = -1;
    s_actionFlashTimer = 0;

    // 2x2 Matrix Invisible Hitboxes in Left Area (x: 8 to 192)
    const float tileW = 90.0f;
    const float tileH = 76.0f;
    const float col0X = 8.0f;
    const float col1X = 102.0f;
    const float row0Y = 44.0f;
    const float row1Y = 126.0f;

    s_dashTiles[DASH_APP_TIMEDOOR]     = (DashboardTileDef){ col0X, row0Y, tileW, tileH, "TIMEDOOR",     "" };
    s_dashTiles[DASH_APP_MISS_MINUTES] = (DashboardTileDef){ col1X, row0Y, tileW, tileH, "MISS MINUTES", "" };
    s_dashTiles[DASH_APP_SETTINGS]     = (DashboardTileDef){ col0X, row1Y, tileW, tileH, "SETTINGS",     "" };
    s_dashTiles[DASH_APP_DIRECTORY]    = (DashboardTileDef){ col1X, row1Y, tileW, tileH, "DIRECTORY",    "" };

    // Action List Invisible Hitboxes in Right Area (x: 198 to 314)
    const float actX = 198.0f;
    const float actW = 116.0f;
    const float actH = 34.0f;

    s_dashActions[DASH_ACT_RUN]    = (DashboardActionDef){ actX, 72.0f,  actW, actH, "1. RUN PROGRAM"    };
    s_dashActions[DASH_ACT_STATUS] = (DashboardActionDef){ actX, 112.0f, actW, actH, "2. CHECK STATUS"   };
    s_dashActions[DASH_ACT_PURGE]  = (DashboardActionDef){ actX, 152.0f, actW, actH, "3. PRUNE TIMELINE" };

    dashboardSetStatus("SYSTEM NOMINAL // SACRED TIMELINE");
}

void dashboardUpdate(void) {
    if (s_tapTimer < 100) {
        s_tapTimer++;
    }
    if (s_actionFlashTimer > 0) {
        s_actionFlashTimer--;
        if (s_actionFlashTimer == 0) {
            s_actionFlashIdx = -1;
        }
    }
}

void dashboardSetStatus(const char* msg) {
    if (msg) {
        strncpy(s_statusMessage, msg, sizeof(s_statusMessage) - 1);
        s_statusMessage[sizeof(s_statusMessage) - 1] = '\0';
    }
}

void dashboardHandleInput(u32 kDown, u32 kHeld, touchPosition touch, AppState* currentState) {
    // 1. Branch Selection (D-Pad Left/Right or L/R Shoulder Buttons)
    if (kDown & (KEY_LEFT | KEY_L)) {
        timelineSelectPrevBranch();
        const VariantProfile* var = timelineGetSelectedVariant();
        if (var) {
            char msg[64];
            snprintf(msg, sizeof(msg), "TARGET: VARIANT# %s (%d)", var->variant_id, var->year);
            dashboardSetStatus(msg);
        }
    }
    if (kDown & (KEY_RIGHT | KEY_R)) {
        timelineSelectNextBranch();
        const VariantProfile* var = timelineGetSelectedVariant();
        if (var) {
            char msg[64];
            snprintf(msg, sizeof(msg), "TARGET: VARIANT# %s (%d)", var->variant_id, var->year);
            dashboardSetStatus(msg);
        }
    }

    // 2. Selective Branch Pruning (Button X, or Button A when branches are active)
    if (kDown & KEY_X) {
        const VariantProfile* var = timelineGetSelectedVariant();
        if (var && timelinePruneSelected()) {
            char msg[64];
            snprintf(msg, sizeof(msg), "PRUNING VARIANT# %s...", var->variant_id);
            dashboardSetStatus(msg);
        }
    }

    // 3. Physical 'A' Button (Prunes selected branch if active, else runs module)
    if (kDown & KEY_A) {
        const VariantProfile* var = timelineGetSelectedVariant();
        if (var && timelinePruneSelected()) {
            char msg[64];
            snprintf(msg, sizeof(msg), "PRUNING VARIANT# %s...", var->variant_id);
            dashboardSetStatus(msg);
        } else if (s_selectedDashApp == DASH_APP_TIMEDOOR) {
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

    // 4. D-Pad Up/Down Navigation for Dashboard Modules
    if (kDown & KEY_UP) {
        if (s_selectedDashApp >= 2) s_selectedDashApp -= 2;
    }
    if (kDown & KEY_DOWN) {
        if (s_selectedDashApp < 2) s_selectedDashApp += 2;
    }

    // 5. Continuous Touch Feedback (kHeld)
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

    // 6. Touch Trigger Actions (kDown)
    if (kDown & KEY_TOUCH) {
        // Touch on 2x2 Matrix (invisible hitboxes)
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

        // Touch on Action List (invisible hitboxes + brief glow feedback flash)
        for (int i = 0; i < DASH_ACT_COUNT; i++) {
            const DashboardActionDef* a = &s_dashActions[i];
            if (touch.px >= a->x && touch.px <= (a->x + a->w) &&
                touch.py >= a->y && touch.py <= (a->y + a->h)) {

                s_actionFlashIdx = i;
                s_actionFlashTimer = 18;

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
    // 1. Top Title Bar & Status Telemetry
    C2D_Text txtTitle;
    tempadParseText(&txtTitle, textBufDynamic, "TEMPAD // TVA MAIN DASHBOARD");
    drawGlowText(&txtTitle, 0, 12.0f, 5.0f, 0.5f, 0.65f, 0.68f, COLOR_DASH_GLOW_HALO, COLOR_TVA_ORANGE);

    C2D_Text txtQuick;
    tempadParseText(&txtQuick, textBufDynamic, s_statusMessage);
    C2D_DrawText(&txtQuick, C2D_WithColor, 12.0f, 23.0f, 0.5f, 0.54f, 0.60f, COLOR_DASH_DIM);

    // 2. 2x2 Floating Modules (NO boxes or borders — icons & labels float directly over grid)
    for (int i = 0; i < DASH_APP_COUNT; i++) {
        const DashboardTileDef* t = &s_dashTiles[i];
        bool isSelected = (s_selectedDashApp == i);
        bool isPressed  = (s_pressedDashTile == i);
        bool isGlowing  = (isSelected || isPressed);

        float drawX = t->x;
        float drawY = t->y;

        // Sprite centrado usando el centro real de cada sprite (subtex->width/2, subtex->height/2)
        bool drewIconSprite = false;
        if (assetsHasUiSheet()) {
            C2D_Image iconImg = assetsGetDashboardIcon(i, isGlowing);
            if (iconImg.tex != NULL) {
                float iconCenterX = drawX + (t->w * 0.5f);
                float iconCenterY = drawY + 27.0f;
                float iconX = iconCenterX - ((float)iconImg.subtex->width * 0.5f);
                float iconY = iconCenterY - ((float)iconImg.subtex->height * 0.5f);
                C2D_DrawImageAt(iconImg, iconX, iconY, 0.0f, NULL, 1.0f, 1.0f);
                drewIconSprite = true;
            }
        }

        if (!drewIconSprite) {
            float iconX = drawX + (t->w - 32.0f) * 0.5f;
            float iconY = drawY + 10.0f;
            drawBoxOutline(iconX, iconY, 32.0f, 32.0f, 1.5f, isGlowing ? COLOR_DASH_GLOW_CORE : COLOR_DASH_DIM);
        }

        // Nombre del modulo centrado debajo del icono (brillo CRT al seleccionar / apagado inactivo)
        C2D_Text txtTile;
        tempadParseText(&txtTile, textBufDynamic, t->title);
        float labelCenterX = drawX + (t->w * 0.5f);
        float labelY = drawY + 55.0f;

        if (isGlowing) {
            drawGlowText(&txtTile, C2D_AlignCenter, labelCenterX, labelY, 0.5f,
                         0.53f, 0.58f, COLOR_DASH_GLOW_HALO, COLOR_DASH_GLOW_CORE);
        } else {
            C2D_DrawText(&txtTile, C2D_WithColor | C2D_AlignCenter, labelCenterX, labelY, 0.5f,
                         0.53f, 0.58f, COLOR_DASH_DIM);
        }
    }

    // 3. Floating Right Panel: "ACTION LIST //////" + Numbered List (NO boxes or borders)
    C2D_Text txtActHeader;
    tempadParseText(&txtActHeader, textBufDynamic, "ACTION LIST //////");
    drawGlowText(&txtActHeader, 0, 198.0f, 44.0f, 0.5f,
                 0.49f, 0.56f, COLOR_DASH_GLOW_HALO, COLOR_DASH_GLOW_CORE);

    for (int i = 0; i < DASH_ACT_COUNT; i++) {
        const DashboardActionDef* a = &s_dashActions[i];
        bool isActive = (s_pressedDashAction == i) ||
                        (s_actionFlashIdx == i && s_actionFlashTimer > 0);

        C2D_Text txtLabel;
        tempadParseText(&txtLabel, textBufDynamic, a->label);
        float textX = a->x;
        float textY = a->y + 8.0f;

        if (isActive) {
            u32 haloCol = (i == DASH_ACT_PURGE) ? COLOR_DASH_RED_HALO : COLOR_DASH_GLOW_HALO;
            u32 coreCol = (i == DASH_ACT_PURGE) ? COLOR_DASH_RED_CORE : COLOR_DASH_GLOW_CORE;
            drawGlowText(&txtLabel, 0, textX, textY, 0.5f, 0.51f, 0.58f, haloCol, coreCol);
        } else {
            u32 idleCol = (i == DASH_ACT_PURGE) ? COLOR_DASH_RED_DIM : COLOR_DASH_DIM;
            C2D_DrawText(&txtLabel, C2D_WithColor, textX, textY, 0.5f, 0.51f, 0.58f, idleCol);
        }
    }

    // 4. Bottom Footer Instructions (Clean, no TVA logo on bottom screen)
    C2D_Text txtFooter;
    tempadParseText(&txtFooter, textBufDynamic, "SELECT MODULE OR 'RUN PROGRAM' // [START] EXIT");
    C2D_DrawText(&txtFooter, C2D_WithColor | C2D_AlignCenter, 160.0f, 212.0f, 0.5f, 0.51f, 0.58f, COLOR_AMBER_DIM);
}
