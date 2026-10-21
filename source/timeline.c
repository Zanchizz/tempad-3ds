#include "timeline.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------
// Internal Timeline State
// ---------------------------------------------------------
static float s_timelinePhase = 0.0f;
static NexusBranch s_branches[2];
static int s_purgeFlashFrames = 0;

void timelineInit(void) {
    s_timelinePhase = 0.0f;
    s_purgeFlashFrames = 0;

    s_branches[0].active = false;
    s_branches[0].startX = 130.0f;
    s_branches[0].deviation = 0.0f;
    s_branches[0].targetDeviation = 0.0f;
    s_branches[0].direction = 1.0f;

    s_branches[1].active = false;
    s_branches[1].startX = 180.0f;
    s_branches[1].deviation = 0.0f;
    s_branches[1].targetDeviation = 0.0f;
    s_branches[1].direction = -1.0f;
}

void timelineUpdate(void) {
    s_timelinePhase += 0.045f;

    // Smooth interpolation (lerp) for Nexus branches
    for (int i = 0; i < 2; i++) {
        NexusBranch* br = &s_branches[i];
        if (br->active || br->deviation > 0.1f) {
            float delta = br->targetDeviation - br->deviation;
            br->deviation += delta * 0.09f;

            if (br->targetDeviation <= 0.01f && br->deviation < 0.4f) {
                br->deviation = 0.0f;
                br->active = false;
            }
        }
    }

    if (s_purgeFlashFrames > 0) {
        s_purgeFlashFrames--;
    }
}

void timelineTriggerBranch(void) {
    if (!s_branches[0].active) {
        s_branches[0].active = true;
        s_branches[0].startX = 125.0f + (rand() % 35);
        s_branches[0].targetDeviation = 48.0f;
    } else {
        s_branches[1].active = true;
        s_branches[1].startX = 175.0f + (rand() % 30);
        s_branches[1].targetDeviation = 55.0f;
        s_branches[0].targetDeviation = 64.0f;
    }
}

void timelinePruneAll(void) {
    s_branches[0].active = false;
    s_branches[0].deviation = 0.0f;
    s_branches[0].targetDeviation = 0.0f;

    s_branches[1].active = false;
    s_branches[1].deviation = 0.0f;
    s_branches[1].targetDeviation = 0.0f;

    s_purgeFlashFrames = 16;
}

float timelineGetPhase(void) {
    return s_timelinePhase;
}

void timelineRenderTopScreen(C3D_RenderTarget* topScreen, C2D_TextBuf textBufDynamic, AppState currentState, const char* activeLocationName) {
    C2D_TargetClear(topScreen, CLR_BG_DARK);
    C2D_SceneBegin(topScreen);

    // 1. Background CRT Grid
    for (float x = 40.0f; x < (float)TOP_WIDTH; x += 40.0f) {
        C2D_DrawLine(x, 26.0f, CLR_GRID, x, 214.0f, CLR_GRID, 1.0f, 0.0f);
    }
    for (float y = 50.0f; y <= 190.0f; y += 35.0f) {
        C2D_DrawLine(10.0f, y, CLR_GRID, 390.0f, y, CLR_GRID, 1.0f, 0.0f);
    }

    // 2. Critical Redline Thresholds
    const float thresholdTop = 52.0f;
    const float thresholdBottom = 188.0f;
    for (float x = 12.0f; x < 388.0f; x += 14.0f) {
        C2D_DrawLine(x, thresholdTop, CLR_RED_GLOW, x + 8.0f, thresholdTop, CLR_RED_GLOW, 1.5f, 0.0f);
        C2D_DrawLine(x, thresholdBottom, CLR_RED_GLOW, x + 8.0f, thresholdBottom, CLR_RED_GLOW, 1.5f, 0.0f);
    }

    // 3. Sacred Timeline (Canonical Bright White / Light Gray)
    const float baseY = 120.0f;
    float prevX = 10.0f;
    float prevY = baseY + 2.5f * sinf(prevX * 0.035f + s_timelinePhase)
                         + 1.2f * sinf(prevX * 0.075f - s_timelinePhase * 1.3f);

    for (float x = 14.0f; x <= 390.0f; x += 4.0f) {
        float curY = baseY + 2.5f * sinf(x * 0.035f + s_timelinePhase)
                           + 1.2f * sinf(x * 0.075f - s_timelinePhase * 1.3f);

        C2D_DrawLine(prevX, prevY, CLR_TIMELINE_GLOW, x, curY, CLR_TIMELINE_GLOW, 4.5f, 0.0f);
        C2D_DrawLine(prevX, prevY, CLR_TIMELINE_MAIN, x, curY, CLR_TIMELINE_MAIN, 2.0f, 0.0f);
        C2D_DrawLine(prevX, prevY, CLR_TIMELINE_CORE, x, curY, CLR_TIMELINE_CORE, 1.0f, 0.0f);

        prevX = x;
        prevY = curY;
    }

    // 4. Variant Branches (Nexus Events: TVA Amber -> Critical Red)
    for (int b = 0; b < 2; b++) {
        NexusBranch* br = &s_branches[b];
        if (br->deviation > 0.1f) {
            float bPrevX = br->startX;
            float bPrevY = baseY + 2.5f * sinf(bPrevX * 0.035f + s_timelinePhase);

            u32 branchColor = (br->deviation > 54.0f) ? CLR_CRITICAL_RED : CLR_AMBER_BRIGHT;
            u32 branchGlow  = (br->deviation > 54.0f) ? CLR_RED_GLOW      : CLR_AMBER_GLOW;

            for (float bx = br->startX + 4.0f; bx <= 390.0f; bx += 4.0f) {
                float progress = (bx - br->startX) / (390.0f - br->startX);
                float curve = progress * progress * (3.0f - 2.0f * progress);
                float wave = 2.0f * sinf(bx * 0.08f + s_timelinePhase * 1.5f);

                float bCurY = baseY - (br->deviation * curve * br->direction) + wave;

                C2D_DrawLine(bPrevX, bPrevY, branchGlow, bx, bCurY, branchGlow, 3.5f, 0.0f);
                C2D_DrawLine(bPrevX, bPrevY, branchColor, bx, bCurY, branchColor, 1.8f, 0.0f);

                bPrevX = bx;
                bPrevY = bCurY;
            }
        }
    }

    // 5. Visual Timedoor Breach when Active
    if (currentState == STATE_TIMEDOOR_ACTIVE) {
        float breachX = 200.0f;
        float pulse = 4.0f * sinf(s_timelinePhase * 4.0f);

        C2D_DrawLine(breachX, 60.0f, CLR_AMBER_GLOW, breachX, 180.0f, CLR_AMBER_GLOW, 5.0f, 0.0f);
        C2D_DrawLine(breachX, 60.0f, CLR_AMBER_CORE, breachX, 180.0f, CLR_AMBER_CORE, 2.0f, 0.0f);

        float ringRadius = 14.0f + pulse;
        drawBoxOutline(breachX - ringRadius, 120.0f - ringRadius, ringRadius * 2.0f, ringRadius * 2.0f, 1.5f, CLR_AMBER_BRIGHT);
    }

    // 6. Reset Charge Purge Flash
    if (s_purgeFlashFrames > 0) {
        float alpha = ((float)s_purgeFlashFrames / 16.0f) * 170.0f;
        u32 flashClr = C2D_Color32(210, 120, 255, (u8)alpha);
        C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, TOP_WIDTH, TOP_HEIGHT, flashClr);
    }

    // 7. Top Screen Telemetry & Labels
    C2D_Text txtTopHeader;
    tempadParseText(&txtTopHeader, textBufDynamic, "TVA // SACRED TIMELINE");
    C2D_DrawText(&txtTopHeader, C2D_WithColor, 12.0f, 8.0f, 0.5f, 0.40f, 0.40f, CLR_AMBER_TEXT);

    const char* modeText = "[MODE: DASHBOARD]";
    if (currentState == STATE_TIMEDOOR_SELECT) {
        modeText = "[MODE: TIMEDOOR SELECT]";
    } else if (currentState == STATE_TIMEDOOR_ACTIVE) {
        modeText = "[MODE: TIMEDOOR ACTIVE]";
    }

    C2D_Text txtMode;
    tempadParseText(&txtMode, textBufDynamic, modeText);
    C2D_DrawText(&txtMode, C2D_WithColor | C2D_AlignRight, 388.0f, 8.0f, 0.5f, 0.36f, 0.36f, CLR_AMBER_BRIGHT);

    // Bottom Status Telemetry
    char telemetry[64];
    float maxDev = fmaxf(s_branches[0].deviation, s_branches[1].deviation);
    u32 statusClr = CLR_AMBER_TEXT;

    if (currentState == STATE_TIMEDOOR_ACTIVE) {
        snprintf(telemetry, sizeof(telemetry), "TIMEDOOR LINKED: %s", activeLocationName ? activeLocationName : "UNKNOWN");
        statusClr = CLR_AMBER_CORE;
    } else {
        if (maxDev > 54.0f) {
            snprintf(telemetry, sizeof(telemetry), "WARNING: CRITICAL DIVERGENCE");
            statusClr = CLR_CRITICAL_RED;
        } else if (maxDev > 1.0f) {
            snprintf(telemetry, sizeof(telemetry), "MONITOR: NEXUS EVENT IN PROGRESS (+%.1f%%)", (maxDev / 66.0f) * 100.0f);
            statusClr = CLR_AMBER_TEXT;
        } else {
            snprintf(telemetry, sizeof(telemetry), "TIMELINE STABLE [1.00x]");
            statusClr = CLR_AMBER_TEXT;
        }
    }

    C2D_Text txtDyn;
    tempadParseText(&txtDyn, textBufDynamic, telemetry);
    C2D_DrawText(&txtDyn, C2D_WithColor, 12.0f, 218.0f, 0.5f, 0.36f, 0.36f, statusClr);
}

