#include <3ds.h>
#include <citro2d.h>
#include "tempad.h"
#include "assets.h"
#include "timeline.h"
#include "dashboard.h"
#include "timedoor.h"

C2D_Font g_tvaFont = NULL;

// ---------------------------------------------------------
// Bottom Screen Dispatcher
// ---------------------------------------------------------
static void renderBottomScreen(C3D_RenderTarget* bottomScreen, C2D_TextBuf textBufDynamic, AppState currentState) {
    C2D_TargetClear(bottomScreen, CLR_BG_DARK);
    C2D_SceneBegin(bottomScreen);

    // Full-screen Subtle Orange CRT Grid (0..320 x 0..240) with integer C2D_DrawRectSolid
    for (int x = 16; x < BOTTOM_WIDTH; x += 24) {
        C2D_DrawRectSolid((float)x, 0.0f, 0.0f, GRID_LINE_THICKNESS, (float)BOTTOM_HEIGHT, CLR_GRID);
    }
    for (int y = 16; y < BOTTOM_HEIGHT; y += 24) {
        C2D_DrawRectSolid(0.0f, (float)y, 0.0f, (float)BOTTOM_WIDTH, GRID_LINE_THICKNESS, CLR_GRID);
    }

    // Outer Chassis Frame
    drawBoxOutline(2.0f, 2.0f, BOTTOM_WIDTH - 4.0f, BOTTOM_HEIGHT - 4.0f, 1.8f, CLR_BORDER_MID);

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
    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    g_tvaFont = C2D_FontLoad("romfs:/tva_font.bcfnt");
    assetsInit();

    C3D_RenderTarget* topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    C2D_TextBuf textBufDynamic = C2D_TextBufNew(2048);
    if (!textBufDynamic) {
        assetsExit();
        if (g_tvaFont) C2D_FontFree(g_tvaFont);
        C2D_Fini();
        C3D_Fini();
        gfxExit();
        romfsExit();
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
        AppState topState = timedoorIsPortalOpen() ? STATE_TIMEDOOR_ACTIVE : currentState;
        timelineRenderTopScreen(topScreen, textBufDynamic, topState, activeLoc ? activeLoc->name : "UNKNOWN");
        renderBottomScreen(bottomScreen, textBufDynamic, currentState);

        C3D_FrameEnd(0);
    }

    C2D_TextBufDelete(textBufDynamic);
    assetsExit();
    if (g_tvaFont) {
        C2D_FontFree(g_tvaFont);
    }
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
    return 0;
}
