#include "timedoor.h"
#include "dashboard.h"
#include <math.h>

// ---------------------------------------------------------
// Internal Timedoor State (Destinations in include/tva_catalog.h)
// ---------------------------------------------------------
#define LOCATION_COUNT TIMEDOOR_LOCATION_COUNT
#define s_locations    g_timedoorLocations
static int s_selectedDestination = 1; // Default: 2012, EARTH
static int s_pressedDestCell = -1;
static int s_pressedScrollBtn = 0;    // 1: Up, 2: Down, 0: None
static bool s_pressedBackButton = false;

static int s_lastTappedDest = -1;
static int s_tapTimer = 99;

void timedoorInit(void) {
    s_selectedDestination = 1;
    s_pressedDestCell = -1;
    s_pressedScrollBtn = 0;
    s_pressedBackButton = false;
    s_lastTappedDest = -1;
    s_tapTimer = 99;
}

void timedoorUpdate(void) {
    if (s_tapTimer < 100) {
        s_tapTimer++;
    }
}

const Location* timedoorGetSelectedLocation(void) {
    return &s_locations[s_selectedDestination];
}

void timedoorHandleInput(u32 kDown, u32 kHeld, touchPosition touch, AppState* currentState) {
    // 1. Physical 'B' Button: Return to previous state
    if (kDown & KEY_B) {
        if (*currentState == STATE_TIMEDOOR_ACTIVE) {
            *currentState = STATE_TIMEDOOR_SELECT;
            dashboardSetStatus("TIMEDOOR CLOSED // SELECT TIME");
            return;
        } else if (*currentState == STATE_TIMEDOOR_SELECT) {
            *currentState = STATE_DASHBOARD;
            dashboardSetStatus("DASHBOARD // MAIN MENU");
            return;
        }
    }

    // 2. Physical 'A' Button: Confirm / Toggle Timedoor
    if (kDown & KEY_A) {
        if (*currentState == STATE_TIMEDOOR_SELECT) {
            *currentState = STATE_TIMEDOOR_ACTIVE;
            dashboardSetStatus("TIMEDOOR DEPLOYED // PORTAL ACTIVE");
            return;
        } else if (*currentState == STATE_TIMEDOOR_ACTIVE) {
            *currentState = STATE_DASHBOARD;
            dashboardSetStatus("TIMEDOOR CLOSED // RETURN TO BASE");
            return;
        }
    }

    // 3. D-Pad Navigation in Saved Times
    if (*currentState == STATE_TIMEDOOR_SELECT) {
        if (kDown & KEY_UP) {
            if (s_selectedDestination > 0) s_selectedDestination--;
        }
        if (kDown & KEY_DOWN) {
            if (s_selectedDestination < LOCATION_COUNT - 1) s_selectedDestination++;
        }
    }

    // 4. Continuous Touch Feedback (kHeld)
    s_pressedDestCell = -1;
    s_pressedScrollBtn = 0;
    s_pressedBackButton = false;

    if (kHeld & KEY_TOUCH) {
        if (*currentState == STATE_TIMEDOOR_SELECT) {
            // '< BACK' Button
            if (touch.px >= 8 && touch.px <= 75 && touch.py >= 6 && touch.py <= 26) {
                s_pressedBackButton = true;
            }

            // Destination Cells (Right Column)
            for (int i = 0; i < LOCATION_COUNT; i++) {
                float cellY = 50.0f + i * 40.0f;
                if (touch.px >= 152 && touch.px <= 290 && touch.py >= cellY && touch.py <= (cellY + 36.0f)) {
                    s_pressedDestCell = i;
                    break;
                }
            }

            // Scrollbar Arrows
            if (touch.px >= 294 && touch.px <= 310) {
                if (touch.py >= 50 && touch.py <= 66) s_pressedScrollBtn = 1;       // Up
                else if (touch.py >= 190 && touch.py <= 206) s_pressedScrollBtn = 2; // Down
            }
        } else if (*currentState == STATE_TIMEDOOR_ACTIVE) {
            if (touch.px >= 8 && touch.px <= 140 && touch.py >= 6 && touch.py <= 28) {
                s_pressedBackButton = true;
            }
        }
    }

    // 5. Touch Trigger Actions (kDown)
    if (kDown & KEY_TOUCH) {
        if (*currentState == STATE_TIMEDOOR_SELECT) {
            // '< BACK' Button
            if (touch.px >= 8 && touch.px <= 75 && touch.py >= 6 && touch.py <= 26) {
                *currentState = STATE_DASHBOARD;
                dashboardSetStatus("DASHBOARD // MAIN MENU");
                return;
            }

            // Destination Cells Selection
            for (int i = 0; i < LOCATION_COUNT; i++) {
                float cellY = 50.0f + i * 40.0f;
                if (touch.px >= 152 && touch.px <= 290 && touch.py >= cellY && touch.py <= (cellY + 36.0f)) {
                    // Double tap on active destination deploys Timedoor
                    if (s_selectedDestination == i && s_lastTappedDest == i && s_tapTimer < 25) {
                        *currentState = STATE_TIMEDOOR_ACTIVE;
                        dashboardSetStatus("TIMEDOOR DEPLOYED // PORTAL ACTIVE");
                        s_lastTappedDest = -1;
                        return;
                    }

                    s_selectedDestination = i;
                    s_lastTappedDest = i;
                    s_tapTimer = 0;
                    break;
                }
            }

            // Scrollbar Arrows
            if (touch.px >= 294 && touch.px <= 310) {
                if (touch.py >= 50 && touch.py <= 66) {
                    if (s_selectedDestination > 0) s_selectedDestination--;
                } else if (touch.py >= 190 && touch.py <= 206) {
                    if (s_selectedDestination < LOCATION_COUNT - 1) s_selectedDestination++;
                }
            }

            // '[ OPEN TIMEDOOR ]' Button (Left Column Bottom)
            if (touch.px >= 10 && touch.px <= 140 && touch.py >= 190 && touch.py <= 226) {
                *currentState = STATE_TIMEDOOR_ACTIVE;
                dashboardSetStatus("TIMEDOOR DEPLOYED // PORTAL ACTIVE");
                return;
            }
        } else if (*currentState == STATE_TIMEDOOR_ACTIVE) {
            if (touch.px >= 8 && touch.px <= 140 && touch.py >= 6 && touch.py <= 28) {
                *currentState = STATE_DASHBOARD;
                dashboardSetStatus("DASHBOARD // MAIN MENU");
                return;
            }
        }
    }
}

void timedoorRenderSelect(C2D_TextBuf textBufDynamic, float animPhase) {
    // 1. Top Bar with Back Button
    float backW = 68.0f, backH = 20.0f;
    u32 backBg = s_pressedBackButton ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
    u32 backTxt = s_pressedBackButton ? CLR_TEXT_INVERTED : CLR_AMBER_TEXT;

    C2D_DrawRectSolid(8.0f, 6.0f, 0.0f, backW, backH, backBg);
    drawBoxOutline(8.0f, 6.0f, backW, backH, 1.0f, CLR_BORDER_MID);

    C2D_Text txtBack;
    tempadParseText(&txtBack, textBufDynamic, "< BACK");
    C2D_DrawText(&txtBack, C2D_WithColor | C2D_AlignCenter, 8.0f + (backW / 2.0f), 8.0f, 0.5f, 0.41f, 0.43f, backTxt);

    C2D_Text txtSubHeader;
    tempadParseText(&txtSubHeader, textBufDynamic, "TVA // TEMPORAL COORDINATES");
    C2D_DrawText(&txtSubHeader, C2D_WithColor | C2D_AlignRight, 312.0f, 8.0f, 0.5f, 0.39f, 0.42f, CLR_AMBER_DARK);

    // -----------------------------------------------------
    // LEFT COLUMN (Destination Preview)
    // -----------------------------------------------------
    C2D_Text txtLeftTitle;
    tempadParseText(&txtLeftTitle, textBufDynamic, "SELECT TIME");
    C2D_DrawText(&txtLeftTitle, C2D_WithColor, 10.0f, 30.0f, 0.5f, 0.46f, 0.48f, CLR_AMBER_BRIGHT);

    // Camera Viewfinder / Thumbnail Placeholder (130x65 px)
    const float viewX = 10.0f, viewY = 48.0f, viewW = 130.0f, viewH = 65.0f;
    C2D_DrawRectSolid(viewX, viewY, 0.0f, viewW, viewH, C2D_Color32(28, 18, 8, 180));
    drawBoxOutline(viewX, viewY, viewW, viewH, 1.5f, CLR_AMBER_BRIGHT);

    // Reticle & CRT Details inside Viewfinder
    float vMidX = viewX + viewW / 2.0f;
    float vMidY = viewY + viewH / 2.0f;
    C2D_DrawLine(vMidX - 12.0f, vMidY, CLR_AMBER_DARK, vMidX - 3.0f, vMidY, CLR_AMBER_DARK, 1.0f, 0.0f);
    C2D_DrawLine(vMidX + 3.0f, vMidY, CLR_AMBER_DARK, vMidX + 12.0f, vMidY, CLR_AMBER_DARK, 1.0f, 0.0f);
    C2D_DrawLine(vMidX, vMidY - 10.0f, CLR_AMBER_DARK, vMidX, vMidY - 3.0f, CLR_AMBER_DARK, 1.0f, 0.0f);
    C2D_DrawLine(vMidX, vMidY + 3.0f, CLR_AMBER_DARK, vMidX, vMidY + 10.0f, CLR_AMBER_DARK, 1.0f, 0.0f);

    // Animated Viewfinder Scanline
    float scanlineY = viewY + fmodf(animPhase * 25.0f, viewH);
    C2D_DrawLine(viewX + 2.0f, scanlineY, C2D_Color32(255, 180, 40, 70), viewX + viewW - 2.0f, scanlineY, C2D_Color32(255, 180, 40, 70), 1.0f, 0.0f);

    C2D_Text txtCamFeed;
    tempadParseText(&txtCamFeed, textBufDynamic, "[ ARCHIVE FEED ]");
    C2D_DrawText(&txtCamFeed, C2D_WithColor, viewX + 6.0f, viewY + 4.0f, 0.5f, 0.34f, 0.36f, CLR_AMBER_TEXT);

    // Selected Location Name & Details
    const Location* curLoc = &s_locations[s_selectedDestination];

    C2D_Text txtDestName;
    tempadParseText(&txtDestName, textBufDynamic, curLoc->name);
    C2D_DrawText(&txtDestName, C2D_WithColor, 10.0f, 118.0f, 0.5f, 0.48f, 0.50f, CLR_AMBER_BRIGHT);

    C2D_Text txtCoordsLabel;
    tempadParseText(&txtCoordsLabel, textBufDynamic, "COORDINATES:");
    C2D_DrawText(&txtCoordsLabel, C2D_WithColor, 10.0f, 140.0f, 0.5f, 0.36f, 0.38f, CLR_AMBER_DARK);

    C2D_Text txtCoords;
    tempadParseText(&txtCoords, textBufDynamic, curLoc->coords);
    C2D_DrawText(&txtCoords, C2D_WithColor, 10.0f, 154.0f, 0.5f, 0.40f, 0.42f, CLR_AMBER_TEXT);

    C2D_Text txtEra;
    tempadParseText(&txtEra, textBufDynamic, curLoc->era);
    C2D_DrawText(&txtEra, C2D_WithColor, 10.0f, 171.0f, 0.5f, 0.31f, 0.35f, CLR_AMBER_DARK);

    // Deploy Timedoor Button
    C2D_DrawRectSolid(10.0f, 192.0f, 0.0f, 130.0f, 36.0f, CLR_BTN_NORMAL);
    drawBoxOutline(10.0f, 192.0f, 130.0f, 36.0f, 1.5f, CLR_AMBER_BRIGHT);

    C2D_Text txtDeploy;
    tempadParseText(&txtDeploy, textBufDynamic, "[ OPEN TIMEDOOR ]");
    C2D_DrawText(&txtDeploy, C2D_WithColor | C2D_AlignCenter, 75.0f, 200.0f, 0.5f, 0.41f, 0.44f, CLR_AMBER_CORE);

    // -----------------------------------------------------
    // RIGHT COLUMN ("SAVED TIMES" Destination List)
    // -----------------------------------------------------
    C2D_Text txtListHeader;
    tempadParseText(&txtListHeader, textBufDynamic, "SAVED TIMES //////");
    C2D_DrawText(&txtListHeader, C2D_WithColor, 152.0f, 30.0f, 0.5f, 0.44f, 0.46f, CLR_AMBER_BRIGHT);

    const float listX = 152.0f;
    const float cellW = 138.0f;
    const float cellH = 36.0f;

    for (int i = 0; i < LOCATION_COUNT; i++) {
        float cellY = 50.0f + i * 40.0f;
        bool isSelected = (s_selectedDestination == i);
        bool isPressed  = (s_pressedDestCell == i);

        u32 cellBg    = isSelected ? (isPressed ? CLR_AMBER_CORE : CLR_AMBER_BRIGHT) : CLR_BTN_NORMAL;
        u32 borderCol = isSelected ? CLR_AMBER_CORE : CLR_BORDER_MID;
        u32 textCol   = isSelected ? CLR_TEXT_INVERTED : CLR_AMBER_TEXT;

        C2D_DrawRectSolid(listX, cellY, 0.0f, cellW, cellH, cellBg);
        drawBoxOutline(listX, cellY, cellW, cellH, 1.5f, borderCol);

        if (isSelected) {
            C2D_Text txtMarker;
            tempadParseText(&txtMarker, textBufDynamic, ">");
            C2D_DrawText(&txtMarker, C2D_WithColor, listX + 5.0f, cellY + 7.0f, 0.5f, 0.44f, 0.46f, textCol);
        }

        C2D_Text txtItem;
        tempadParseText(&txtItem, textBufDynamic, s_locations[i].name);
        C2D_DrawText(&txtItem, C2D_WithColor, listX + (isSelected ? 17.0f : 9.0f), cellY + 7.0f, 0.5f, 0.42f, 0.45f, textCol);
    }

    // Right Scrollbar Simulation
    const float sbX = 296.0f, sbY = 50.0f, sbW = 16.0f, sbH = 156.0f;
    C2D_DrawRectSolid(sbX, sbY, 0.0f, sbW, sbH, CLR_PANEL_INSET);
    drawBoxOutline(sbX, sbY, sbW, sbH, 1.0f, CLR_BORDER_MID);

    // Up Arrow
    u32 upBg = (s_pressedScrollBtn == 1) ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
    C2D_DrawRectSolid(sbX, sbY, 0.0f, sbW, 16.0f, upBg);
    drawBoxOutline(sbX, sbY, sbW, 16.0f, 1.0f, CLR_BORDER_MID);
    C2D_Text txtUp;
    tempadParseText(&txtUp, textBufDynamic, "^");
    C2D_DrawText(&txtUp, C2D_WithColor | C2D_AlignCenter, sbX + 8.0f, sbY, 0.5f, 0.42f, 0.42f, CLR_AMBER_TEXT);

    // Down Arrow
    u32 downBg = (s_pressedScrollBtn == 2) ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
    C2D_DrawRectSolid(sbX, sbY + sbH - 16.0f, 0.0f, sbW, 16.0f, downBg);
    drawBoxOutline(sbX, sbY + sbH - 16.0f, sbW, 16.0f, 1.0f, CLR_BORDER_MID);
    C2D_Text txtDown;
    tempadParseText(&txtDown, textBufDynamic, "v");
    C2D_DrawText(&txtDown, C2D_WithColor | C2D_AlignCenter, sbX + 8.0f, sbY + sbH - 16.0f, 0.5f, 0.42f, 0.42f, CLR_AMBER_TEXT);

    // Slider Thumb
    float thumbY = sbY + 18.0f + (float)s_selectedDestination * ((sbH - 60.0f) / (LOCATION_COUNT - 1));
    C2D_DrawRectSolid(sbX + 2.0f, thumbY, 0.0f, sbW - 4.0f, 24.0f, CLR_AMBER_BRIGHT);
}

void timedoorRenderActive(C2D_TextBuf textBufDynamic, float animPhase) {
    // 1. Quick Close / Return Button
    float backW = 142.0f, backH = 22.0f;
    u32 backBg = s_pressedBackButton ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
    u32 backTxt = s_pressedBackButton ? CLR_TEXT_INVERTED : CLR_AMBER_TEXT;

    C2D_DrawRectSolid(8.0f, 6.0f, 0.0f, backW, backH, backBg);
    drawBoxOutline(8.0f, 6.0f, backW, backH, 1.0f, CLR_BORDER_MID);

    C2D_Text txtBack;
    tempadParseText(&txtBack, textBufDynamic, "< [B] CLOSE TIMEDOOR");
    C2D_DrawText(&txtBack, C2D_WithColor | C2D_AlignCenter, 8.0f + (backW / 2.0f), 8.0f, 0.5f, 0.39f, 0.42f, backTxt);

    // 2. Retro Open Timedoor Portal Graphic
    float portalX = 160.0f, portalY = 115.0f;
    float pW = 110.0f, pH = 130.0f;

    float pulse = 3.0f * sinf(animPhase * 5.0f);
    C2D_DrawRectSolid(portalX - (pW / 2.0f) - 4.0f, portalY - (pH / 2.0f) - 4.0f, 0.0f, pW + 8.0f, pH + 8.0f, CLR_AMBER_GLOW);

    C2D_DrawRectSolid(portalX - (pW / 2.0f), portalY - (pH / 2.0f), 0.0f, pW, pH, C2D_Color32(240, 150, 20, 255));
    drawBoxOutline(portalX - (pW / 2.0f), portalY - (pH / 2.0f), pW, pH, 2.5f, CLR_AMBER_CORE);

    C2D_DrawRectSolid(portalX - 35.0f - pulse, portalY - 45.0f - pulse, 0.0f, 70.0f + pulse * 2.0f, 90.0f + pulse * 2.0f, CLR_AMBER_CORE);

    // 3. Safe Passage Telemetry
    const Location* curLoc = &s_locations[s_selectedDestination];

    C2D_Text txtStatus;
    tempadParseText(&txtStatus, textBufDynamic, "TIMEDOOR OPEN // STABLE LINK");
    C2D_DrawText(&txtStatus, C2D_WithColor | C2D_AlignCenter, 160.0f, 34.0f, 0.5f, 0.45f, 0.48f, CLR_AMBER_BRIGHT);

    C2D_Text txtDest;
    tempadParseText(&txtDest, textBufDynamic, curLoc->name);
    C2D_DrawText(&txtDest, C2D_WithColor | C2D_AlignCenter, 160.0f, 190.0f, 0.5f, 0.54f, 0.56f, CLR_AMBER_CORE);

    C2D_Text txtCoord;
    tempadParseText(&txtCoord, textBufDynamic, curLoc->coords);
    C2D_DrawText(&txtCoord, C2D_WithColor | C2D_AlignCenter, 160.0f, 213.0f, 0.5f, 0.41f, 0.43f, CLR_AMBER_DARK);
}

