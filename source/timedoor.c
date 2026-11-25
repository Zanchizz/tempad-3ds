#include "timedoor.h"
#include "dashboard.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------
// Two-Step Timedoor Submenu State
// ---------------------------------------------------------
typedef enum {
    TIMEDOOR_STEP_LOCATION = 0,
    TIMEDOOR_STEP_DATE
} TimedoorStep;

#define LOCATION_COUNT      TIMEDOOR_LOCATION_COUNT
#define s_locations         g_timedoorLocations

// Layout constants for Step 1 (5 destinations in right column)
#define LIST_X              152.0f
#define LIST_START_Y        48.0f
#define LIST_CELL_W         138.0f
#define LIST_CELL_H         32.0f
#define LIST_CELL_PITCH     36.0f

// Layout constants for Step 2 (DAY / MONTH / YEAR date picker)
#define DATE_DAY_X          152.0f
#define DATE_MONTH_X        204.0f
#define DATE_YEAR_X         256.0f
#define DATE_COL_W          48.0f
#define DATE_YEAR_W         56.0f

#define DATE_UP_Y           74.0f
#define DATE_VAL_Y          114.0f
#define DATE_DOWN_Y         164.0f
#define DATE_BTN_H          34.0f
#define DATE_VAL_H          44.0f

static TimedoorStep s_step = TIMEDOOR_STEP_LOCATION;
static int s_selectedDestination = 0; // Default: 2026, EARTH
static int s_day = 0;
static int s_month = 0;
static int s_dateFocus = 0;           // 0 = DAY, 1 = MONTH

static int s_pressedDestCell = -1;
static int s_pressedScrollBtn = 0;    // 1: Up, 2: Down, 0: None
static int s_pressedDateBtn = 0;      // 1: Day+, 2: Day-, 3: Mon+, 4: Mon-
static bool s_pressedBackButton = false;
static bool s_pressedOpenButton = false;

static bool s_portalOpen = false;

// ---------------------------------------------------------
// Date Helpers (supports 00/00 default state)
// ---------------------------------------------------------
static int getDaysInMonth(int month, int year) {
    static const int kDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month <= 0 || month > 12) return 31;
    if (month == 2) {
        bool isLeap = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
        return isLeap ? 29 : 28;
    }
    return kDays[month - 1];
}

static void loadDestinationDefaults(int destIdx) {
    if (destIdx < 0) destIdx = 0;
    if (destIdx >= LOCATION_COUNT) destIdx = LOCATION_COUNT - 1;

    s_selectedDestination = destIdx;
    const Location* loc = &s_locations[s_selectedDestination];

    s_month = loc->default_month;
    if (s_month < 0) s_month = 0;
    if (s_month > 12) s_month = 12;

    int maxDay = getDaysInMonth(s_month, loc->year);
    s_day = loc->default_day;
    if (s_day < 0) s_day = 0;
    if (s_day > maxDay) s_day = maxDay;
}

static void adjustDay(int delta) {
    const Location* loc = &s_locations[s_selectedDestination];
    int maxDay = getDaysInMonth(s_month, loc->year);

    if (s_day <= 0) {
        s_day = (delta > 0) ? 1 : maxDay;
        return;
    }

    s_day += delta;
    if (s_day > maxDay) s_day = 1;
    if (s_day < 1) s_day = maxDay;
}

static void adjustMonth(int delta) {
    const Location* loc = &s_locations[s_selectedDestination];

    if (s_month <= 0) {
        s_month = (delta > 0) ? 1 : 12;
    } else {
        s_month += delta;
        if (s_month > 12) s_month = 1;
        if (s_month < 1) s_month = 12;
    }

    if (s_day > 0) {
        int maxDay = getDaysInMonth(s_month, loc->year);
        if (s_day > maxDay) {
            s_day = maxDay;
        }
    }
}

static void openTimedoorPortal(void) {
    const Location* loc = &s_locations[s_selectedDestination];
    s_portalOpen = true;
    char msg[64];
    snprintf(msg, sizeof(msg), "PORTAL LOCKED // %02d.%02d.%04d", s_day, s_month, loc->year);
    dashboardSetStatus(msg);
}

static void drawGlowTextSimple(C2D_Text* txt, u32 flags, float x, float y, float z,
                               float scaleX, float scaleY, u32 haloCol, u32 coreCol) {
    u32 drawFlags = flags | C2D_WithColor;
    C2D_DrawText(txt, drawFlags, x - 1.0f, y,        z, scaleX, scaleY, haloCol);
    C2D_DrawText(txt, drawFlags, x + 1.0f, y,        z, scaleX, scaleY, haloCol);
    C2D_DrawText(txt, drawFlags, x,        y - 1.0f, z, scaleX, scaleY, haloCol);
    C2D_DrawText(txt, drawFlags, x,        y + 1.0f, z, scaleX, scaleY, haloCol);
    C2D_DrawText(txt, drawFlags, x,        y,        z, scaleX, scaleY, coreCol);
}

static void drawArrowButton(float x, float y, float w, float h, bool pointingUp,
                            bool isFocused, bool isPressed) {
    u32 bgCol     = isPressed ? COLOR_TVA_ORANGE : COLOR_BTN_NORMAL;
    u32 borderCol = isPressed ? COLOR_BRANCH_SELECTED : (isFocused ? COLOR_TVA_ORANGE : COLOR_BORDER_MID);
    u32 triCol    = isPressed ? COLOR_TEXT_INVERTED : (isFocused ? COLOR_DASH_GLOW_CORE : COLOR_TVA_ORANGE);

    C2D_DrawRectSolid(x, y, 0.0f, w, h, bgCol);
    drawBoxOutline(x, y, w, h, isFocused ? 1.6f : 1.2f, borderCol);

    float cx = x + (w * 0.5f);
    float cy = y + (h * 0.5f);
    if (pointingUp) {
        C2D_DrawTriangle(cx, cy - 7.0f, triCol,
                         cx - 9.0f, cy + 6.0f, triCol,
                         cx + 9.0f, cy + 6.0f, triCol, 0.0f);
    } else {
        C2D_DrawTriangle(cx - 9.0f, cy - 6.0f, triCol,
                         cx + 9.0f, cy - 6.0f, triCol,
                         cx, cy + 7.0f, triCol, 0.0f);
    }
}

void timedoorInit(void) {
    s_step = TIMEDOOR_STEP_LOCATION;
    s_dateFocus = 0;
    s_pressedDestCell = -1;
    s_pressedScrollBtn = 0;
    s_pressedDateBtn = 0;
    s_pressedBackButton = false;
    s_pressedOpenButton = false;
    s_portalOpen = false;
    loadDestinationDefaults(0); // Default: 2026, EARTH (00/00)
}

void timedoorUpdate(void) {
    // Persistent portal state remains active until user closes it via touch or B
}

const Location* timedoorGetSelectedLocation(void) {
    return &s_locations[s_selectedDestination];
}

bool timedoorIsPortalOpen(void) {
    return s_portalOpen;
}

void timedoorHandleInput(u32 kDown, u32 kHeld, touchPosition touch, AppState* currentState) {
    // 0. While Timedoor Portal is Open: ANY tap on touch screen or 'B' closes it immediately
    //    and returns on this frame so the tap does not activate buttons or arrows underneath.
    if (s_portalOpen) {
        s_pressedDestCell = -1;
        s_pressedScrollBtn = 0;
        s_pressedDateBtn = 0;
        s_pressedBackButton = false;
        s_pressedOpenButton = false;

        if ((kDown & KEY_TOUCH) || (kDown & KEY_B)) {
            s_portalOpen = false;
            dashboardSetStatus("TIMEDOOR CLOSED");
        }
        return;
    }

    // 1. Physical 'B' Button: Step 2 -> Step 1, Step 1 -> Dashboard
    if (kDown & KEY_B) {
        if (*currentState == STATE_TIMEDOOR_ACTIVE) {
            *currentState = STATE_TIMEDOOR_SELECT;
            s_step = TIMEDOOR_STEP_DATE;
            return;
        }
        if (*currentState == STATE_TIMEDOOR_SELECT) {
            if (s_step == TIMEDOOR_STEP_DATE) {
                s_step = TIMEDOOR_STEP_LOCATION;
                return;
            } else {
                *currentState = STATE_DASHBOARD;
                dashboardSetStatus("DASHBOARD // MAIN MENU");
                return;
            }
        }
    }

    // 2. Physical 'A' Button
    if (kDown & KEY_A) {
        if (*currentState == STATE_TIMEDOOR_SELECT) {
            if (s_step == TIMEDOOR_STEP_LOCATION) {
                loadDestinationDefaults(s_selectedDestination);
                s_step = TIMEDOOR_STEP_DATE;
                s_dateFocus = 0;
                return;
            } else {
                openTimedoorPortal();
                return;
            }
        } else if (*currentState == STATE_TIMEDOOR_ACTIVE) {
            *currentState = STATE_DASHBOARD;
            dashboardSetStatus("TIMEDOOR CLOSED // RETURN TO BASE");
            return;
        }
    }

    // 3. D-Pad Navigation by Step
    if (*currentState == STATE_TIMEDOOR_SELECT) {
        if (s_step == TIMEDOOR_STEP_LOCATION) {
            if (kDown & KEY_UP) {
                if (s_selectedDestination > 0) {
                    loadDestinationDefaults(s_selectedDestination - 1);
                }
            }
            if (kDown & KEY_DOWN) {
                if (s_selectedDestination < LOCATION_COUNT - 1) {
                    loadDestinationDefaults(s_selectedDestination + 1);
                }
            }
        } else {
            // Step 2: Left/Right switches focus between DAY (0) and MONTH (1)
            if (kDown & KEY_LEFT)  s_dateFocus = 0;
            if (kDown & KEY_RIGHT) s_dateFocus = 1;

            // Up/Down adjusts focused field with wrap-around
            if (kDown & KEY_UP) {
                if (s_dateFocus == 0) adjustDay(+1);
                else adjustMonth(+1);
            }
            if (kDown & KEY_DOWN) {
                if (s_dateFocus == 0) adjustDay(-1);
                else adjustMonth(-1);
            }
        }
    }

    // 4. Continuous Touch Feedback (kHeld)
    s_pressedDestCell = -1;
    s_pressedScrollBtn = 0;
    s_pressedDateBtn = 0;
    s_pressedBackButton = false;
    s_pressedOpenButton = false;

    if (kHeld & KEY_TOUCH) {
        if (*currentState == STATE_TIMEDOOR_SELECT) {
            // '< BACK' Button
            if (touch.px >= 8 && touch.px <= 76 && touch.py >= 6 && touch.py <= 26) {
                s_pressedBackButton = true;
            }
            // '[ OPEN TIMEDOOR ]' Button
            if (touch.px >= 10 && touch.px <= 140 && touch.py >= 192 && touch.py <= 228) {
                s_pressedOpenButton = true;
            }

            if (s_step == TIMEDOOR_STEP_LOCATION) {
                for (int i = 0; i < LOCATION_COUNT; i++) {
                    float cellY = LIST_START_Y + i * LIST_CELL_PITCH;
                    if (touch.px >= LIST_X && touch.px <= (LIST_X + LIST_CELL_W) &&
                        touch.py >= cellY && touch.py <= (cellY + LIST_CELL_H)) {
                        s_pressedDestCell = i;
                        break;
                    }
                }
                if (touch.px >= 294 && touch.px <= 312) {
                    if (touch.py >= 48 && touch.py <= 64) s_pressedScrollBtn = 1;
                    else if (touch.py >= 208 && touch.py <= 224) s_pressedScrollBtn = 2;
                }
            } else {
                // Step 2: 4 Arrow Buttons (generous 48x34 hitboxes)
                if (touch.px >= DATE_DAY_X && touch.px <= (DATE_DAY_X + DATE_COL_W)) {
                    if (touch.py >= DATE_UP_Y && touch.py <= (DATE_UP_Y + DATE_BTN_H)) s_pressedDateBtn = 1;
                    else if (touch.py >= DATE_DOWN_Y && touch.py <= (DATE_DOWN_Y + DATE_BTN_H)) s_pressedDateBtn = 2;
                } else if (touch.px >= DATE_MONTH_X && touch.px <= (DATE_MONTH_X + DATE_COL_W)) {
                    if (touch.py >= DATE_UP_Y && touch.py <= (DATE_UP_Y + DATE_BTN_H)) s_pressedDateBtn = 3;
                    else if (touch.py >= DATE_DOWN_Y && touch.py <= (DATE_DOWN_Y + DATE_BTN_H)) s_pressedDateBtn = 4;
                }
            }
        } else if (*currentState == STATE_TIMEDOOR_ACTIVE) {
            if (touch.px >= 8 && touch.px <= 156 && touch.py >= 6 && touch.py <= 28) {
                s_pressedBackButton = true;
            }
        }
    }

    // 5. Touch Trigger Actions (kDown)
    if (kDown & KEY_TOUCH) {
        if (*currentState == STATE_TIMEDOOR_SELECT) {
            // '< BACK' Button
            if (touch.px >= 8 && touch.px <= 76 && touch.py >= 6 && touch.py <= 26) {
                if (s_step == TIMEDOOR_STEP_DATE) {
                    s_step = TIMEDOOR_STEP_LOCATION;
                    return;
                } else {
                    *currentState = STATE_DASHBOARD;
                    dashboardSetStatus("DASHBOARD // MAIN MENU");
                    return;
                }
            }

            // '[ OPEN TIMEDOOR ]' Button (works in BOTH Step 1 and Step 2)
            if (touch.px >= 10 && touch.px <= 140 && touch.py >= 192 && touch.py <= 228) {
                openTimedoorPortal();
                return;
            }

            if (s_step == TIMEDOOR_STEP_LOCATION) {
                // Step 1: Tapping a destination selects it, loads defaults, and moves to Step 2
                for (int i = 0; i < LOCATION_COUNT; i++) {
                    float cellY = LIST_START_Y + i * LIST_CELL_PITCH;
                    if (touch.px >= LIST_X && touch.px <= (LIST_X + LIST_CELL_W) &&
                        touch.py >= cellY && touch.py <= (cellY + LIST_CELL_H)) {
                        loadDestinationDefaults(i);
                        s_step = TIMEDOOR_STEP_DATE;
                        s_dateFocus = 0;
                        return;
                    }
                }

                // Scrollbar Arrows
                if (touch.px >= 294 && touch.px <= 312) {
                    if (touch.py >= 48 && touch.py <= 64) {
                        if (s_selectedDestination > 0) loadDestinationDefaults(s_selectedDestination - 1);
                    } else if (touch.py >= 208 && touch.py <= 224) {
                        if (s_selectedDestination < LOCATION_COUNT - 1) loadDestinationDefaults(s_selectedDestination + 1);
                    }
                }
            } else {
                // Step 2: DAY column arrows & value focus
                if (touch.px >= DATE_DAY_X && touch.px <= (DATE_DAY_X + DATE_COL_W)) {
                    if (touch.py >= DATE_UP_Y && touch.py <= (DATE_UP_Y + DATE_BTN_H)) {
                        s_dateFocus = 0;
                        adjustDay(+1);
                    } else if (touch.py >= DATE_DOWN_Y && touch.py <= (DATE_DOWN_Y + DATE_BTN_H)) {
                        s_dateFocus = 0;
                        adjustDay(-1);
                    } else if (touch.py >= DATE_VAL_Y && touch.py <= (DATE_VAL_Y + DATE_VAL_H)) {
                        s_dateFocus = 0;
                    }
                }
                // Step 2: MONTH column arrows & value focus
                else if (touch.px >= DATE_MONTH_X && touch.px <= (DATE_MONTH_X + DATE_COL_W)) {
                    if (touch.py >= DATE_UP_Y && touch.py <= (DATE_UP_Y + DATE_BTN_H)) {
                        s_dateFocus = 1;
                        adjustMonth(+1);
                    } else if (touch.py >= DATE_DOWN_Y && touch.py <= (DATE_DOWN_Y + DATE_BTN_H)) {
                        s_dateFocus = 1;
                        adjustMonth(-1);
                    } else if (touch.py >= DATE_VAL_Y && touch.py <= (DATE_VAL_Y + DATE_VAL_H)) {
                        s_dateFocus = 1;
                    }
                }
            }
        } else if (*currentState == STATE_TIMEDOOR_ACTIVE) {
            if (touch.px >= 8 && touch.px <= 156 && touch.py >= 6 && touch.py <= 28) {
                *currentState = STATE_DASHBOARD;
                dashboardSetStatus("DASHBOARD // MAIN MENU");
                return;
            }
        }
    }
}

void timedoorRenderSelect(C2D_TextBuf textBufDynamic, float animPhase) {
    const Location* curLoc = &s_locations[s_selectedDestination];

    // 1. Top Bar with Back Button
    float backW = 68.0f, backH = 20.0f;
    u32 backBg = s_pressedBackButton ? COLOR_TVA_ORANGE : COLOR_BTN_NORMAL;
    u32 backTxt = s_pressedBackButton ? COLOR_TEXT_INVERTED : COLOR_TVA_ORANGE;

    C2D_DrawRectSolid(8.0f, 6.0f, 0.0f, backW, backH, backBg);
    drawBoxOutline(8.0f, 6.0f, backW, backH, 1.0f, COLOR_BORDER_MID);

    C2D_Text txtBack;
    tempadParseText(&txtBack, textBufDynamic, "< BACK");
    C2D_DrawText(&txtBack, C2D_WithColor | C2D_AlignCenter, 8.0f + (backW / 2.0f), 6.0f, 0.0f, 0.58f, 0.62f, backTxt);

    C2D_Text txtSubHeader;
    tempadParseText(&txtSubHeader, textBufDynamic, "TVA // TEMPORAL COORDINATES");
    C2D_DrawText(&txtSubHeader, C2D_WithColor | C2D_AlignRight, 312.0f, 6.0f, 0.0f, 0.54f, 0.60f, COLOR_DASH_DIM);

    // -----------------------------------------------------
    // LEFT COLUMN (Destination Preview & Live Coordinates)
    // -----------------------------------------------------
    C2D_Text txtLeftTitle;
    tempadParseText(&txtLeftTitle, textBufDynamic, "SELECT TIME");
    C2D_DrawText(&txtLeftTitle, C2D_WithColor, 10.0f, 26.0f, 0.0f, 0.64f, 0.68f, COLOR_TVA_ORANGE);

    // Camera Viewfinder / Thumbnail Placeholder (130x65 px)
    const float viewX = 10.0f, viewY = 48.0f, viewW = 130.0f, viewH = 65.0f;
    C2D_DrawRectSolid(viewX, viewY, 0.0f, viewW, viewH, COLOR_BTN_NORMAL);
    drawBoxOutline(viewX, viewY, viewW, viewH, 1.5f, COLOR_TVA_ORANGE);

    // Reticle & CRT Details inside Viewfinder
    float vMidX = viewX + viewW / 2.0f;
    float vMidY = viewY + viewH / 2.0f;
    C2D_DrawLine(vMidX - 12.0f, vMidY, COLOR_AMBER_DIM, vMidX - 3.0f, vMidY, COLOR_AMBER_DIM, 1.0f, 0.0f);
    C2D_DrawLine(vMidX + 3.0f, vMidY, COLOR_AMBER_DIM, vMidX + 12.0f, vMidY, COLOR_AMBER_DIM, 1.0f, 0.0f);
    C2D_DrawLine(vMidX, vMidY - 10.0f, COLOR_AMBER_DIM, vMidX, vMidY - 3.0f, COLOR_AMBER_DIM, 1.0f, 0.0f);
    C2D_DrawLine(vMidX, vMidY + 3.0f, COLOR_AMBER_DIM, vMidX, vMidY + 10.0f, COLOR_AMBER_DIM, 1.0f, 0.0f);

    // Animated Viewfinder Scanline
    float scanlineY = viewY + fmodf(animPhase * 25.0f, viewH);
    C2D_DrawLine(viewX + 2.0f, scanlineY, COLOR_SCANLINE, viewX + viewW - 2.0f, scanlineY, COLOR_SCANLINE, 1.0f, 0.0f);

    C2D_Text txtCamFeed;
    tempadParseText(&txtCamFeed, textBufDynamic, "[ ARCHIVE FEED ]");
    C2D_DrawText(&txtCamFeed, C2D_WithColor, viewX + 6.0f, viewY + 3.0f, 0.0f, 0.50f, 0.56f, COLOR_TVA_ORANGE);

    // Selected Location Name
    C2D_Text txtDestName;
    tempadParseText(&txtDestName, textBufDynamic, curLoc->name);
    C2D_DrawText(&txtDestName, C2D_WithColor, 10.0f, 115.0f, 0.0f, 0.66f, 0.72f, COLOR_TVA_ORANGE);

    // Live Dynamic Coordinates (reflects DD.MM.YYYY // CODE in real time)
    C2D_Text txtCoordsLabel;
    tempadParseText(&txtCoordsLabel, textBufDynamic, "COORDINATES:");
    C2D_DrawText(&txtCoordsLabel, C2D_WithColor, 10.0f, 136.0f, 0.0f, 0.54f, 0.60f, COLOR_DASH_DIM);

    char liveCoords[32];
    snprintf(liveCoords, sizeof(liveCoords), "%02d.%02d.%04d // %s",
             s_day, s_month, curLoc->year, curLoc->loc_code);

    C2D_Text txtCoords;
    tempadParseText(&txtCoords, textBufDynamic, liveCoords);
    C2D_DrawText(&txtCoords, C2D_WithColor, 10.0f, 151.0f, 0.0f, 0.56f, 0.62f, COLOR_DASH_GLOW_CORE);

    // Sector / Portal Locked confirmation line
    char eraLine[48];
    u32 eraCol = COLOR_DASH_DIM;
    if (s_portalOpen) {
        snprintf(eraLine, sizeof(eraLine), "PORTAL LOCKED // %02d.%02d.%04d", s_day, s_month, curLoc->year);
        eraCol = COLOR_DASH_GLOW_CORE;
    } else {
        snprintf(eraLine, sizeof(eraLine), "%s", curLoc->era);
    }

    size_t eraLen = strlen(eraLine);
    float eraScaleX = (eraLen > 24) ? 0.39f : 0.43f;
    float eraScaleY = eraScaleX / 0.86f;

    C2D_Text txtEra;
    tempadParseText(&txtEra, textBufDynamic, eraLine);
    C2D_DrawText(&txtEra, C2D_WithColor, 10.0f, 169.0f, 0.0f, eraScaleX, eraScaleY, eraCol);

    // Deploy Timedoor Button (stays lit solid orange "TIMEDOOR OPEN" while s_portalOpen == true)
    bool isOpenActive = s_portalOpen || s_pressedOpenButton;
    u32 openBg     = isOpenActive ? COLOR_TVA_ORANGE : COLOR_BTN_NORMAL;
    u32 openBorder = isOpenActive ? COLOR_DASH_GLOW_CORE : COLOR_TVA_ORANGE;
    u32 openTxtCol = isOpenActive ? COLOR_TEXT_INVERTED : COLOR_DASH_GLOW_CORE;
    const char* openLabel = s_portalOpen ? "TIMEDOOR OPEN" : "[ OPEN TIMEDOOR ]";

    C2D_DrawRectSolid(10.0f, 192.0f, 0.0f, 130.0f, 36.0f, openBg);
    drawBoxOutline(10.0f, 192.0f, 130.0f, 36.0f, 1.5f, openBorder);

    C2D_Text txtDeploy;
    tempadParseText(&txtDeploy, textBufDynamic, openLabel);
    C2D_DrawText(&txtDeploy, C2D_WithColor | C2D_AlignCenter, 75.0f, 199.0f, 0.0f, 0.54f, 0.60f, openTxtCol);

    // -----------------------------------------------------
    // RIGHT COLUMN: STEP 1 ("SAVED TIMES") OR STEP 2 ("SET DATE")
    // -----------------------------------------------------
    if (s_step == TIMEDOOR_STEP_LOCATION) {
        C2D_Text txtListHeader;
        tempadParseText(&txtListHeader, textBufDynamic, "SAVED TIMES //////");
        C2D_DrawText(&txtListHeader, C2D_WithColor, LIST_X, 26.0f, 0.0f, 0.60f, 0.66f, COLOR_TVA_ORANGE);

        for (int i = 0; i < LOCATION_COUNT; i++) {
            float cellY = LIST_START_Y + i * LIST_CELL_PITCH;
            bool isSelected = (s_selectedDestination == i);
            bool isPressed  = (s_pressedDestCell == i);

            u32 cellBg    = isSelected ? (isPressed ? COLOR_DASH_GLOW_CORE : COLOR_TVA_ORANGE) : COLOR_BTN_NORMAL;
            u32 borderCol = isSelected ? COLOR_DASH_GLOW_CORE : COLOR_BORDER_MID;
            u32 textCol   = isSelected ? COLOR_TEXT_INVERTED : COLOR_TVA_ORANGE;

            C2D_DrawRectSolid(LIST_X, cellY, 0.0f, LIST_CELL_W, LIST_CELL_H, cellBg);
            drawBoxOutline(LIST_X, cellY, LIST_CELL_W, LIST_CELL_H, 1.4f, borderCol);

            if (isSelected) {
                C2D_Text txtMarker;
                tempadParseText(&txtMarker, textBufDynamic, ">");
                C2D_DrawText(&txtMarker, C2D_WithColor, LIST_X + 5.0f, cellY + 5.0f, 0.0f, 0.60f, 0.64f, textCol);
            }

            C2D_Text txtItem;
            tempadParseText(&txtItem, textBufDynamic, s_locations[i].name);
            C2D_DrawText(&txtItem, C2D_WithColor, LIST_X + (isSelected ? 17.0f : 8.0f), cellY + 5.0f, 0.0f, 0.58f, 0.64f, textCol);
        }

        // Right Scrollbar Simulation (tracks all 5 locations cleanly)
        const float sbX = 296.0f, sbY = 48.0f, sbW = 16.0f, sbH = 176.0f;
        C2D_DrawRectSolid(sbX, sbY, 0.0f, sbW, sbH, COLOR_PANEL_INSET);
        drawBoxOutline(sbX, sbY, sbW, sbH, 1.0f, COLOR_BORDER_MID);

        u32 upBg = (s_pressedScrollBtn == 1) ? COLOR_TVA_ORANGE : COLOR_BTN_NORMAL;
        C2D_DrawRectSolid(sbX, sbY, 0.0f, sbW, 16.0f, upBg);
        drawBoxOutline(sbX, sbY, sbW, 16.0f, 1.0f, COLOR_BORDER_MID);
        C2D_Text txtUp;
        tempadParseText(&txtUp, textBufDynamic, "^");
        C2D_DrawText(&txtUp, C2D_WithColor | C2D_AlignCenter, sbX + 8.0f, sbY - 1.0f, 0.0f, 0.58f, 0.58f, COLOR_TVA_ORANGE);

        u32 downBg = (s_pressedScrollBtn == 2) ? COLOR_TVA_ORANGE : COLOR_BTN_NORMAL;
        C2D_DrawRectSolid(sbX, sbY + sbH - 16.0f, 0.0f, sbW, 16.0f, downBg);
        drawBoxOutline(sbX, sbY + sbH - 16.0f, sbW, 16.0f, 1.0f, COLOR_BORDER_MID);
        C2D_Text txtDown;
        tempadParseText(&txtDown, textBufDynamic, "v");
        C2D_DrawText(&txtDown, C2D_WithColor | C2D_AlignCenter, sbX + 8.0f, sbY + sbH - 17.0f, 0.0f, 0.58f, 0.58f, COLOR_TVA_ORANGE);

        float thumbRange = (LOCATION_COUNT > 1) ? ((sbH - 60.0f) / (float)(LOCATION_COUNT - 1)) : 0.0f;
        float thumbY = sbY + 18.0f + (float)s_selectedDestination * thumbRange;
        C2D_DrawRectSolid(sbX + 2.0f, thumbY, 0.0f, sbW - 4.0f, 24.0f, COLOR_TVA_ORANGE);
    } else {
        // STEP 2: SET DATE ////// (DAY, MONTH, FIXED YEAR)
        C2D_Text txtDateHeader;
        tempadParseText(&txtDateHeader, textBufDynamic, "SET DATE //////");
        drawGlowTextSimple(&txtDateHeader, 0, LIST_X, 26.0f, 0.0f,
                           0.62f, 0.66f, COLOR_DASH_GLOW_HALO, COLOR_DASH_GLOW_CORE);

        bool dayFocused   = (s_dateFocus == 0);
        bool monthFocused = (s_dateFocus == 1);

        // Column Labels: DAY | MONTH | YEAR
        C2D_Text txtDayLbl, txtMonLbl, txtYearLbl;
        tempadParseText(&txtDayLbl, textBufDynamic, "DAY");
        tempadParseText(&txtMonLbl, textBufDynamic, "MONTH");
        tempadParseText(&txtYearLbl, textBufDynamic, "YEAR");

        float dayCx   = DATE_DAY_X   + (DATE_COL_W  * 0.5f);
        float monthCx = DATE_MONTH_X + (DATE_COL_W  * 0.5f);
        float yearCx  = DATE_YEAR_X  + (DATE_YEAR_W * 0.5f);

        if (dayFocused) {
            drawGlowTextSimple(&txtDayLbl, C2D_AlignCenter, dayCx, 52.0f, 0.0f,
                               0.54f, 0.58f, COLOR_DASH_GLOW_HALO, COLOR_DASH_GLOW_CORE);
        } else {
            C2D_DrawText(&txtDayLbl, C2D_WithColor | C2D_AlignCenter, dayCx, 52.0f, 0.0f,
                         0.54f, 0.58f, COLOR_DASH_DIM);
        }

        if (monthFocused) {
            drawGlowTextSimple(&txtMonLbl, C2D_AlignCenter, monthCx, 52.0f, 0.0f,
                               0.50f, 0.56f, COLOR_DASH_GLOW_HALO, COLOR_DASH_GLOW_CORE);
        } else {
            C2D_DrawText(&txtMonLbl, C2D_WithColor | C2D_AlignCenter, monthCx, 52.0f, 0.0f,
                         0.50f, 0.56f, COLOR_DASH_DIM);
        }

        C2D_DrawText(&txtYearLbl, C2D_WithColor | C2D_AlignCenter, yearCx, 52.0f, 0.0f,
                     0.52f, 0.56f, COLOR_AMBER_DIM);

        // Top Up-Arrow Buttons (DAY & MONTH only; YEAR is fixed with no arrows)
        drawArrowButton(DATE_DAY_X,   DATE_UP_Y, DATE_COL_W, DATE_BTN_H, true, dayFocused,   s_pressedDateBtn == 1);
        drawArrowButton(DATE_MONTH_X, DATE_UP_Y, DATE_COL_W, DATE_BTN_H, true, monthFocused, s_pressedDateBtn == 3);

        // Middle Value Boxes (DD | MM | YYYY)
        char dayStr[8], monStr[8], yearStr[8];
        snprintf(dayStr,  sizeof(dayStr),  "%02d", s_day);
        snprintf(monStr,  sizeof(monStr),  "%02d", s_month);
        snprintf(yearStr, sizeof(yearStr), "%04d", curLoc->year);

        // DAY Value Box
        C2D_DrawRectSolid(DATE_DAY_X, DATE_VAL_Y, 0.0f, DATE_COL_W, DATE_VAL_H, COLOR_BTN_NORMAL);
        drawBoxOutline(DATE_DAY_X, DATE_VAL_Y, DATE_COL_W, DATE_VAL_H,
                       dayFocused ? 1.8f : 1.2f, dayFocused ? COLOR_TVA_ORANGE : COLOR_BORDER_MID);
        C2D_Text txtDayVal;
        tempadParseText(&txtDayVal, textBufDynamic, dayStr);
        if (dayFocused) {
            drawGlowTextSimple(&txtDayVal, C2D_AlignCenter, dayCx, DATE_VAL_Y + 8.0f, 0.0f,
                               0.86f, 0.90f, COLOR_DASH_GLOW_HALO, COLOR_DASH_GLOW_CORE);
        } else {
            C2D_DrawText(&txtDayVal, C2D_WithColor | C2D_AlignCenter, dayCx, DATE_VAL_Y + 8.0f, 0.0f,
                         0.86f, 0.90f, COLOR_TVA_ORANGE);
        }

        // MONTH Value Box
        C2D_DrawRectSolid(DATE_MONTH_X, DATE_VAL_Y, 0.0f, DATE_COL_W, DATE_VAL_H, COLOR_BTN_NORMAL);
        drawBoxOutline(DATE_MONTH_X, DATE_VAL_Y, DATE_COL_W, DATE_VAL_H,
                       monthFocused ? 1.8f : 1.2f, monthFocused ? COLOR_TVA_ORANGE : COLOR_BORDER_MID);
        C2D_Text txtMonVal;
        tempadParseText(&txtMonVal, textBufDynamic, monStr);
        if (monthFocused) {
            drawGlowTextSimple(&txtMonVal, C2D_AlignCenter, monthCx, DATE_VAL_Y + 8.0f, 0.0f,
                               0.86f, 0.90f, COLOR_DASH_GLOW_HALO, COLOR_DASH_GLOW_CORE);
        } else {
            C2D_DrawText(&txtMonVal, C2D_WithColor | C2D_AlignCenter, monthCx, DATE_VAL_Y + 8.0f, 0.0f,
                         0.86f, 0.90f, COLOR_TVA_ORANGE);
        }

        // YEAR Value Box (Fixed / Locked, dim styling, NO arrows)
        C2D_DrawRectSolid(DATE_YEAR_X, DATE_VAL_Y, 0.0f, DATE_YEAR_W, DATE_VAL_H, COLOR_PANEL_INSET);
        drawBoxOutline(DATE_YEAR_X, DATE_VAL_Y, DATE_YEAR_W, DATE_VAL_H, 1.2f, COLOR_BORDER_DIM);

        C2D_Text txtYearVal, txtLocked;
        tempadParseText(&txtYearVal, textBufDynamic, yearStr);
        C2D_DrawText(&txtYearVal, C2D_WithColor | C2D_AlignCenter, yearCx, DATE_VAL_Y + 11.0f, 0.0f,
                     0.72f, 0.78f, COLOR_DASH_DIM);

        tempadParseText(&txtLocked, textBufDynamic, "FIXED");
        C2D_DrawText(&txtLocked, C2D_WithColor | C2D_AlignCenter, yearCx, DATE_DOWN_Y + 9.0f, 0.0f,
                     0.48f, 0.52f, COLOR_AMBER_DIM);

        // Bottom Down-Arrow Buttons (DAY & MONTH only)
        drawArrowButton(DATE_DAY_X,   DATE_DOWN_Y, DATE_COL_W, DATE_BTN_H, false, dayFocused,   s_pressedDateBtn == 2);
        drawArrowButton(DATE_MONTH_X, DATE_DOWN_Y, DATE_COL_W, DATE_BTN_H, false, monthFocused, s_pressedDateBtn == 4);

        // Footer Navigation Hint
        const char* hintStr = s_portalOpen ? "TAP SCREEN OR [B] TO CLOSE" : "[B] DEST  //  [A] OPEN";
        u32 hintCol = s_portalOpen ? COLOR_DASH_GLOW_CORE : COLOR_AMBER_DIM;
        C2D_Text txtHint;
        tempadParseText(&txtHint, textBufDynamic, hintStr);
        C2D_DrawText(&txtHint, C2D_WithColor | C2D_AlignCenter, 232.0f, 208.0f, 0.0f,
                     0.46f, 0.52f, hintCol);
    }
}

void timedoorRenderActive(C2D_TextBuf textBufDynamic, float animPhase) {
    // 1. Quick Close / Return Button
    float backW = 148.0f, backH = 22.0f;
    u32 backBg = s_pressedBackButton ? COLOR_TVA_ORANGE : COLOR_BTN_NORMAL;
    u32 backTxt = s_pressedBackButton ? COLOR_TEXT_INVERTED : COLOR_TVA_ORANGE;

    C2D_DrawRectSolid(8.0f, 6.0f, 0.0f, backW, backH, backBg);
    drawBoxOutline(8.0f, 6.0f, backW, backH, 1.0f, COLOR_BORDER_MID);

    C2D_Text txtBack;
    tempadParseText(&txtBack, textBufDynamic, "< [B] CLOSE TIMEDOOR");
    C2D_DrawText(&txtBack, C2D_WithColor | C2D_AlignCenter, 8.0f + (backW / 2.0f), 6.0f, 0.5f, 0.53f, 0.58f, backTxt);

    // 2. Retro Open Timedoor Portal Graphic
    float portalX = 160.0f, portalY = 115.0f;
    float pW = 110.0f, pH = 130.0f;

    float pulse = 3.0f * sinf(animPhase * 5.0f);
    C2D_DrawRectSolid(portalX - (pW / 2.0f) - 4.0f, portalY - (pH / 2.0f) - 4.0f, 0.0f, pW + 8.0f, pH + 8.0f, COLOR_BRANCH_AMBER_GLOW);

    C2D_DrawRectSolid(portalX - (pW / 2.0f), portalY - (pH / 2.0f), 0.0f, pW, pH, COLOR_TVA_ORANGE);
    drawBoxOutline(portalX - (pW / 2.0f), portalY - (pH / 2.0f), pW, pH, 2.5f, COLOR_BRANCH_SELECTED);

    C2D_DrawRectSolid(portalX - 35.0f - pulse, portalY - 45.0f - pulse, 0.0f, 70.0f + pulse * 2.0f, 90.0f + pulse * 2.0f, COLOR_BRANCH_SELECTED);

    // 3. Safe Passage Telemetry
    const Location* curLoc = &s_locations[s_selectedDestination];

    C2D_Text txtStatus;
    tempadParseText(&txtStatus, textBufDynamic, "TIMEDOOR OPEN // STABLE LINK");
    C2D_DrawText(&txtStatus, C2D_WithColor | C2D_AlignCenter, 160.0f, 32.0f, 0.5f, 0.62f, 0.68f, COLOR_TVA_ORANGE);

    C2D_Text txtDest;
    tempadParseText(&txtDest, textBufDynamic, curLoc->name);
    C2D_DrawText(&txtDest, C2D_WithColor | C2D_AlignCenter, 160.0f, 188.0f, 0.5f, 0.76f, 0.80f, COLOR_DASH_GLOW_CORE);

    char liveCoords[32];
    snprintf(liveCoords, sizeof(liveCoords), "%02d.%02d.%04d // %s",
             s_day, s_month, curLoc->year, curLoc->loc_code);

    C2D_Text txtCoord;
    tempadParseText(&txtCoord, textBufDynamic, liveCoords);
    C2D_DrawText(&txtCoord, C2D_WithColor | C2D_AlignCenter, 160.0f, 211.0f, 0.5f, 0.58f, 0.64f, COLOR_DASH_DIM);
}
