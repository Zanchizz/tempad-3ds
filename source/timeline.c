#include "timeline.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------
// Monitor Geometry Constants
// ---------------------------------------------------------
#define BASELINE_Y       120.0f
#define THRESHOLD_TOP    52.0f
#define THRESHOLD_BOTTOM 188.0f

// ---------------------------------------------------------
// Internal Timeline State
// ---------------------------------------------------------
static float s_timelinePhase = 0.0f;
static TimelineBranch s_branches[MAX_BRANCHES];
static int s_selected_branch_index = -1;
static int s_spawnTimer = 0;

static float getSacredTimelineY(float x, float phase) {
    return BASELINE_Y + 2.5f * sinf(x * 0.035f + phase)
                      + 1.2f * sinf(x * 0.075f - phase * 1.3f);
}

static void ensureValidSelection(void) {
    if (s_selected_branch_index >= 0 && s_selected_branch_index < MAX_BRANCHES &&
        s_branches[s_selected_branch_index].active) {
        return;
    }

    s_selected_branch_index = -1;
    for (int i = 0; i < MAX_BRANCHES; i++) {
        if (s_branches[i].active) {
            s_selected_branch_index = i;
            break;
        }
    }
}

void timelineInit(void) {
    s_timelinePhase = 0.0f;
    s_selected_branch_index = -1;
    s_spawnTimer = 0;

    for (int i = 0; i < MAX_BRANCHES; i++) {
        s_branches[i].active = false;
        s_branches[i].start_x = 0.0f;
        s_branches[i].start_y = BASELINE_Y;
        s_branches[i].direction = -1;
        s_branches[i].current_length = 0.0f;
        s_branches[i].max_length = 100.0f;
        s_branches[i].curve_factor = 0.045f;
        s_branches[i].is_critical = false;
        s_branches[i].is_pruning = false;
    }
}

void timelineTriggerBranch(void) {
    int slot = -1;
    for (int i = 0; i < MAX_BRANCHES; i++) {
        if (!s_branches[i].active) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        return; // All 10 branch slots are currently active
    }

    TimelineBranch* br = &s_branches[slot];
    br->active = true;
    br->start_x = 35.0f + (float)(rand() % 210); // Spawns between X=35 and X=245
    br->start_y = getSacredTimelineY(br->start_x, s_timelinePhase);
    br->direction = (rand() % 2 == 0) ? -1 : 1;
    br->current_length = 2.0f;

    float maxLen = 95.0f + (float)(rand() % 45); // 95..139 px
    if (br->start_x + maxLen > 386.0f) {
        maxLen = 386.0f - br->start_x;
    }
    br->max_length = maxLen;

    // Curve factor calibrated so branches cross the +/-68px redline and reach 84..96px
    float targetMaxDy = 84.0f + (float)(rand() % 13);
    br->curve_factor = targetMaxDy / powf(br->max_length, 1.6f);

    br->is_critical = false;
    br->is_pruning = false;

    if (s_selected_branch_index < 0) {
        s_selected_branch_index = slot;
    }
}

void timelinePruneAll(void) {
    for (int i = 0; i < MAX_BRANCHES; i++) {
        if (s_branches[i].active) {
            s_branches[i].is_pruning = true;
        }
    }
}

void timelineSelectNextBranch(void) {
    ensureValidSelection();
    if (s_selected_branch_index < 0) return;

    for (int step = 1; step <= MAX_BRANCHES; step++) {
        int idx = (s_selected_branch_index + step) % MAX_BRANCHES;
        if (s_branches[idx].active) {
            s_selected_branch_index = idx;
            break;
        }
    }
}

void timelineSelectPrevBranch(void) {
    ensureValidSelection();
    if (s_selected_branch_index < 0) return;

    for (int step = 1; step <= MAX_BRANCHES; step++) {
        int idx = (s_selected_branch_index - step + MAX_BRANCHES) % MAX_BRANCHES;
        if (s_branches[idx].active) {
            s_selected_branch_index = idx;
            break;
        }
    }
}

bool timelinePruneSelected(void) {
    ensureValidSelection();
    if (s_selected_branch_index >= 0 && s_branches[s_selected_branch_index].active) {
        s_branches[s_selected_branch_index].is_pruning = true;
        return true;
    }
    return false;
}

int timelineGetActiveCount(void) {
    int count = 0;
    for (int i = 0; i < MAX_BRANCHES; i++) {
        if (s_branches[i].active) count++;
    }
    return count;
}

int timelineGetSelectedBranchIndex(void) {
    return s_selected_branch_index;
}

float timelineGetPhase(void) {
    return s_timelinePhase;
}

void timelineUpdate(void) {
    s_timelinePhase += 0.045f;

    // Organic branch generation over time (~every 4.5 seconds if slots available)
    s_spawnTimer++;
    if (s_spawnTimer >= 270) {
        s_spawnTimer = 0;
        if (timelineGetActiveCount() < MAX_BRANCHES) {
            timelineTriggerBranch();
        }
    }

    // Update all active branches
    for (int i = 0; i < MAX_BRANCHES; i++) {
        TimelineBranch* br = &s_branches[i];
        if (!br->active) continue;

        if (br->is_pruning) {
            // Progressive pruning from outer tip back to 0 on the main timeline
            br->current_length -= 2.2f;
            if (br->current_length <= 0.0f) {
                br->current_length = 0.0f;
                br->active = false;
                br->is_pruning = false;
                br->is_critical = false;
                continue;
            }
        } else {
            // Steady exponential/linear growth along X
            if (br->current_length < br->max_length) {
                br->current_length += 0.42f;
                if (br->current_length > br->max_length) {
                    br->current_length = br->max_length;
                }
            }
        }

        // Evaluate tip Y position against redline thresholds (no cap at threshold)
        float tipDy = (float)br->direction * br->curve_factor * powf(br->current_length, 1.6f);
        float tipY = br->start_y + tipDy;
        br->is_critical = (tipY < THRESHOLD_TOP || tipY > THRESHOLD_BOTTOM);
    }

    ensureValidSelection();
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
    for (float x = 12.0f; x < 388.0f; x += 14.0f) {
        C2D_DrawLine(x, THRESHOLD_TOP, CLR_RED_GLOW, x + 8.0f, THRESHOLD_TOP, CLR_RED_GLOW, 1.5f, 0.0f);
        C2D_DrawLine(x, THRESHOLD_BOTTOM, CLR_RED_GLOW, x + 8.0f, THRESHOLD_BOTTOM, CLR_RED_GLOW, 1.5f, 0.0f);
    }

    // 3. Variant Branches (Firm exponential curves, step = 5.0f px for GPU efficiency)
    int activeCount = 0;
    int criticalCount = 0;

    for (int i = 0; i < MAX_BRANCHES; i++) {
        TimelineBranch* br = &s_branches[i];
        if (!br->active || br->current_length <= 0.1f) continue;

        activeCount++;
        if (br->is_critical) {
            criticalCount++;
        }

        bool isSelected = (i == s_selected_branch_index);

        u32 branchColor = br->is_critical ? CLR_CRITICAL_RED : (isSelected ? CLR_AMBER_CORE : CLR_AMBER_BRIGHT);
        u32 branchGlow  = br->is_critical ? CLR_RED_GLOW      : CLR_AMBER_GLOW;
        float coreThick = isSelected ? 2.4f : 1.7f;
        float glowThick = isSelected ? 4.2f : 3.2f;

        // Root connects to the main timeline at start_x; body uses fixed start_y (no floating wave)
        float bPrevX = br->start_x;
        float bPrevY = getSacredTimelineY(br->start_x, s_timelinePhase);

        const float stepX = 5.0f;
        float dx = stepX;
        while (dx < br->current_length) {
            float bx = br->start_x + dx;
            float dy = (float)br->direction * br->curve_factor * powf(dx, 1.6f);
            float by = br->start_y + dy;

            C2D_DrawLine(bPrevX, bPrevY, branchGlow, bx, by, branchGlow, glowThick, 0.0f);
            C2D_DrawLine(bPrevX, bPrevY, branchColor, bx, by, branchColor, coreThick, 0.0f);

            bPrevX = bx;
            bPrevY = by;
            dx += stepX;
        }

        // Final segment to exact tip
        float tipX = br->start_x + br->current_length;
        float tipDy = (float)br->direction * br->curve_factor * powf(br->current_length, 1.6f);
        float tipY = br->start_y + tipDy;

        C2D_DrawLine(bPrevX, bPrevY, branchGlow, tipX, tipY, branchGlow, glowThick, 0.0f);
        C2D_DrawLine(bPrevX, bPrevY, branchColor, tipX, tipY, branchColor, coreThick, 0.0f);

        // Disintegration Spark on receding tip when pruning
        if (br->is_pruning) {
            u32 pruneGlow = C2D_Color32(190, 80, 255, 210);
            C2D_DrawRectSolid(tipX - 4.0f, tipY - 4.0f, 0.0f, 8.0f, 8.0f, pruneGlow);
            C2D_DrawRectSolid(tipX - 2.0f, tipY - 2.0f, 0.0f, 4.0f, 4.0f, CLR_AMBER_CORE);
        }

        // Target Reticle '[ ]' on the currently selected branch tip
        if (isSelected) {
            u32 retCol = br->is_critical ? CLR_CRITICAL_RED : CLR_AMBER_CORE;
            float r = 6.0f;
            // Left bracket '['
            C2D_DrawLine(tipX - r, tipY - r, retCol, tipX - r + 3.0f, tipY - r, retCol, 1.5f, 0.0f);
            C2D_DrawLine(tipX - r, tipY - r, retCol, tipX - r, tipY + r, retCol, 1.5f, 0.0f);
            C2D_DrawLine(tipX - r, tipY + r, retCol, tipX - r + 3.0f, tipY + r, retCol, 1.5f, 0.0f);
            // Right bracket ']'
            C2D_DrawLine(tipX + r - 3.0f, tipY - r, retCol, tipX + r, tipY - r, retCol, 1.5f, 0.0f);
            C2D_DrawLine(tipX + r, tipY - r, retCol, tipX + r, tipY + r, retCol, 1.5f, 0.0f);
            C2D_DrawLine(tipX + r - 3.0f, tipY + r, retCol, tipX + r, tipY + r, retCol, 1.5f, 0.0f);
        }
    }

    // 4. Sacred Timeline (Muted Silver / Gray with darker soft edge)
    float prevX = 10.0f;
    float prevY = getSacredTimelineY(prevX, s_timelinePhase);

    for (float x = 14.0f; x <= 390.0f; x += 4.0f) {
        float curY = getSacredTimelineY(x, s_timelinePhase);

        C2D_DrawLine(prevX, prevY, CLR_TIMELINE_GLOW, x, curY, CLR_TIMELINE_GLOW, 4.0f, 0.0f);
        C2D_DrawLine(prevX, prevY, CLR_TIMELINE_CORE, x, curY, CLR_TIMELINE_CORE, 2.0f, 0.0f);

        prevX = x;
        prevY = curY;
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

    // 6. Top Screen Telemetry & Labels
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

    // Selected Branch Target Readout (Bottom Right)
    if (s_selected_branch_index >= 0 && s_branches[s_selected_branch_index].active) {
        char targetBuf[32];
        snprintf(targetBuf, sizeof(targetBuf), "TARGET: BRANCH #%02d", s_selected_branch_index + 1);
        C2D_Text txtTarget;
        tempadParseText(&txtTarget, textBufDynamic, targetBuf);
        u32 targetClr = s_branches[s_selected_branch_index].is_critical ? CLR_CRITICAL_RED : CLR_AMBER_BRIGHT;
        C2D_DrawText(&txtTarget, C2D_WithColor | C2D_AlignRight, 388.0f, 218.0f, 0.5f, 0.34f, 0.34f, targetClr);
    }

    // Bottom-Left Status Telemetry & Multiverse Emergency Alert
    char telemetry[64];
    u32 statusClr = CLR_AMBER_TEXT;

    if (currentState == STATE_TIMEDOOR_ACTIVE) {
        snprintf(telemetry, sizeof(telemetry), "TIMEDOOR LINKED: %s", activeLocationName ? activeLocationName : "UNKNOWN");
        statusClr = CLR_AMBER_CORE;
    } else if (criticalCount >= 3) {
        snprintf(telemetry, sizeof(telemetry), "DANGER: MULTIVERSE");
        bool blinkOn = (((int)(s_timelinePhase * 3.5f)) % 2 == 0);
        statusClr = blinkOn ? CLR_CRITICAL_RED : C2D_Color32(120, 20, 10, 255);
        if (blinkOn) {
            drawBoxOutline(2.0f, 2.0f, TOP_WIDTH - 4.0f, TOP_HEIGHT - 4.0f, 2.0f, CLR_CRITICAL_RED);
        }
    } else if (criticalCount >= 1) {
        snprintf(telemetry, sizeof(telemetry), "WARNING: CRITICAL DIVERGENCE");
        statusClr = CLR_CRITICAL_RED;
    } else if (activeCount >= 1) {
        snprintf(telemetry, sizeof(telemetry), "MONITOR: NEXUS EVENT IN PROGRESS");
        statusClr = CLR_AMBER_TEXT;
    } else {
        snprintf(telemetry, sizeof(telemetry), "TIMELINE STABLE [1.00x]");
        statusClr = CLR_AMBER_TEXT;
    }

    C2D_Text txtDyn;
    tempadParseText(&txtDyn, textBufDynamic, telemetry);
    C2D_DrawText(&txtDyn, C2D_WithColor, 12.0f, 218.0f, 0.5f, 0.36f, 0.36f, statusClr);
}
