#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "tempad.h"

// ---------------------------------------------------------
// Dashboard Structures (2x2 Matrix + Action List)
// ---------------------------------------------------------
typedef enum {
    DASH_APP_TIMEDOOR = 0,
    DASH_APP_MISS_MINUTES,
    DASH_APP_SETTINGS,
    DASH_APP_DIRECTORY,
    DASH_APP_COUNT
} DashboardAppId;

typedef struct {
    float x, y, w, h;
    const char* title;
    const char* code;
} DashboardTileDef;

typedef enum {
    DASH_ACT_RUN = 0,
    DASH_ACT_STATUS,
    DASH_ACT_PURGE,
    DASH_ACT_COUNT
} DashboardActionId;

typedef struct {
    float x, y, w, h;
    const char* label;
} DashboardActionDef;

// ---------------------------------------------------------
// Dashboard Module API
// ---------------------------------------------------------
void dashboardInit(void);
void dashboardUpdate(void);
void dashboardSetStatus(const char* msg);
void dashboardHandleInput(u32 kDown, u32 kHeld, touchPosition touch, AppState* currentState);
void dashboardRender(C2D_TextBuf textBufDynamic);

#endif // DASHBOARD_H

