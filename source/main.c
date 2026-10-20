#include <3ds.h>
#include <citro2d.h>
#include "tempad.h"
#include "timeline.h"
#include "dashboard.h"
#include "timedoor.h"

// ---------------------------------------------------------
// Bottom Screen Dispatcher
// ---------------------------------------------------------
static void renderBottomScreen(C3D_RenderTarget* bottomScreen, C2D_TextBuf textBufDynamic, AppState currentState) {
    C2D_TargetClear(bottomScreen, CLR_BG_DARK);
    C2D_SceneBegin(bottomScreen);

    // Subtle Retro Background Grid
    for (float x = 20.0f; x < (float)BOTTOM_WIDTH; x += 30.0f) {
        C2D_DrawLine(x, 0.0f, CLR_GRID, x, (float)BOTTOM_HEIGHT, CLR_GRID, 1.0f, 0.0f);
    }
    for (float y = 20.0f; y < (float)BOTTOM_HEIGHT; y += 30.0f) {
        C2D_DrawLine(0.0f, y, CLR_GRID, (float)BOTTOM_WIDTH, y, CLR_GRID, 1.0f, 0.0f);
    }

    // Outer Chassis Frame
    drawBoxOutline(2.0f, 2.0f, BOTTOM_WIDTH - 4.0f, BOTTOM_HEIGHT - 4.0f, 2.0f, CLR_BORDER_DIM);

    // State View Dispatch
    switch (currentState) {
        case STATE_DASHBOARD:
            dashboardRender(textBufDynamic);
            break;
        case STATE_TIMEDOOR_SELECT:
            timedoorRenderSelect(textBufDynamic, timelineGetPhase());
            break;
        case STATE_TIMEDOOR_ACTIVE:
            timedoorRenderActive(textBufDynamic, timelineGetPhase());
            break;
    }
}

// ---------------------------------------------------------
// Application Entry Point
// ---------------------------------------------------------
int main(int argc, char* argv[]) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    C2D_TextBuf textBufDynamic = C2D_TextBufNew(2048);
    if (!textBufDynamic) {
        C2D_Fini();
        C3D_Fini();
        gfxExit();
        return 1;
    }

    AppState currentState = STATE_DASHBOARD;

    timelineInit();
    dashboardInit();
    timedoorInit();

    // Main Loop synchronized at 60 FPS
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_START) {
            break; // Return to Homebrew Launcher
        }

        touchPosition touch = { 0, 0 };
        if ((kDown | kHeld) & KEY_TOUCH) {
            hidTouchRead(&touch);
        }

        // Dispatch Input by Current State
        switch (currentState) {
            case STATE_DASHBOARD:
                dashboardHandleInput(kDown, kHeld, touch, &currentState);
                break;
            case STATE_TIMEDOOR_SELECT:
            case STATE_TIMEDOOR_ACTIVE:
                timedoorHandleInput(kDown, kHeld, touch, &currentState);
                break;
        }

        // Update Modules
        timelineUpdate();
        dashboardUpdate();
        timedoorUpdate();

        // Synchronized Frame Rendering
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TextBufClear(textBufDynamic);

        const Location* activeLoc = timedoorGetSelectedLocation();
        timelineRenderTopScreen(topScreen, textBufDynamic, currentState, activeLoc ? activeLoc->name : "UNKNOWN");
        renderBottomScreen(bottomScreen, textBufDynamic, currentState);

        C3D_FrameEnd(0);
    }

    C2D_TextBufDelete(textBufDynamic);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
