#include "timeline.h"
#include "assets.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------
// Internal Timeline State
// ---------------------------------------------------------
static float s_timelinePhase = 0.0f;
static float s_marqueeX = 0.0f;
static TimelineBranch s_branches[MAX_BRANCHES];
static int s_selected_branch_index = -1;
static int s_spawnTimer = 0;

static float getSacredTimelineY(float x, float phase) {
    return TIMELINE_BASELINE_Y + (0.70f * SACRED_WAVE_AMPLITUDE) * sinf(x * 0.032f + phase)
                               + (0.30f * SACRED_WAVE_AMPLITUDE) * sinf(x * 0.068f - phase * 1.15f);
}

static u32 makeBranchColorWithAlpha(bool isCritical, bool isSelected, u8 alpha) {
    if (isCritical) {
        return C2D_Color32(250, 48, 24, alpha);
    } else if (isSelected) {
        return C2D_Color32(255, 240, 145, alpha);
    }
    return C2D_Color32(255, 220, 75, alpha);
}

static u32 makeGlowColorWithAlpha(bool isCritical, u8 alpha) {
    if (isCritical) {
        return C2D_Color32(220, 35, 15, alpha);
    }
    return C2D_Color32(245, 175, 25, alpha);
}

static void ensureValidSelection(void) {
    if (s_selected_branch_index >= 0 && s_selected_branch_index < MAX_BRANCHES &&
        s_branches[s_selected_branch_index].active) {
        return;
    }

    // Select the leftmost (earliest chronological) active branch by default
    int bestIdx = -1;
    float bestX = 9999.0f;
    for (int i = 0; i < MAX_BRANCHES; i++) {
        if (s_branches[i].active && s_branches[i].start_x < bestX) {
            bestX = s_branches[i].start_x;
            bestIdx = i;
        }
    }
    s_selected_branch_index = bestIdx;
}

void timelineInit(void) {
    s_timelinePhase = 0.0f;
    s_marqueeX = 0.0f;
    s_selected_branch_index = -1;
    // Spawn first branch ~1.5s after launch; subsequent ones every 1080 frames (~18s)
    s_spawnTimer = BRANCH_SPAWN_INTERVAL_FRAMES - 90;

    for (int i = 0; i < MAX_BRANCHES; i++) {
        s_branches[i].active = false;
        s_branches[i].variant_index = 0;
        s_branches[i].start_x = 0.0f;
        s_branches[i].start_y = TIMELINE_BASELINE_Y;
        s_branches[i].direction = -1;
        s_branches[i].current_length = 0.0f;
        s_branches[i].max_length = 70.0f;
        s_branches[i].curve_factor = 0.045f;
        s_branches[i].units_remaining = BRANCH_INITIAL_UNITS;
        s_branches[i].energy_gc = 12.8f;
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

    // Gather all variants from g_variantCatalog not currently active in another branch
    int availableVariants[VARIANT_CATALOG_COUNT];
    int availableCount = 0;

    for (int v = 0; v < VARIANT_CATALOG_COUNT; v++) {
        bool inUse = false;
        for (int b = 0; b < MAX_BRANCHES; b++) {
            if (s_branches[b].active && s_branches[b].variant_index == v) {
                inUse = true;
                break;
            }
        }
        if (!inUse) {
            availableVariants[availableCount++] = v;
        }
    }

    if (availableCount == 0) {
        return;
    }

    int chosenVarIdx = availableVariants[rand() % availableCount];
    const VariantProfile* varProfile = &g_variantCatalog[chosenVarIdx];

    // Map variant year chronologically to start_x along the Sacred Timeline graph
    float normYear = (float)(varProfile->year - TIMELINE_YEAR_MIN) /
                     (float)(TIMELINE_YEAR_MAX - TIMELINE_YEAR_MIN);
    if (normYear < 0.0f) normYear = 0.0f;
    if (normYear > 1.0f) normYear = 1.0f;

    TimelineBranch* br = &s_branches[slot];
    br->active = true;
    br->variant_index = chosenVarIdx;
    br->start_x = TIMELINE_SPAWN_X_MIN + normYear * (TIMELINE_SPAWN_X_MAX - TIMELINE_SPAWN_X_MIN);
    br->start_y = getSacredTimelineY(br->start_x, s_timelinePhase);
    br->direction = (rand() % 2 == 0) ? -1 : 1;
    br->current_length = 0.5f;

    float maxLen = 58.0f + (float)(rand() % 18); // 58..75 px
    if (br->start_x + maxLen > TIMELINE_BRANCH_MAX_X) {
        maxLen = TIMELINE_BRANCH_MAX_X - br->start_x;
    }
    br->max_length = maxLen;

    // Calibrate curve_factor so the branch crosses the red line and grows all the way
    // until touching the exact top (y = 10) or bottom (y = 216) border of the monitor box
    float borderDistY = (br->direction < 0)
        ? (br->start_y - GRAPH_BOX_Y)
        : (GRAPH_BOTTOM_Y - br->start_y);
    br->curve_factor = borderDistY / powf(br->max_length, BRANCH_CURVE_EXPONENT);

    // Random temporal energy reading between 10.0Gc and 89.9Gc
    br->energy_gc = 10.0f + ((float)(rand() % 800) / 10.0f);

    br->units_remaining = BRANCH_INITIAL_UNITS;
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

    float curX = s_branches[s_selected_branch_index].start_x;
    int nextIdx = -1;
    float bestDelta = 9999.0f;
    int leftmostIdx = s_selected_branch_index;
    float leftmostX = curX;

    for (int i = 0; i < MAX_BRANCHES; i++) {
        if (!s_branches[i].active || i == s_selected_branch_index) continue;
        float x = s_branches[i].start_x;
        if (x < leftmostX) {
            leftmostX = x;
            leftmostIdx = i;
        }
        if (x > curX && (x - curX) < bestDelta) {
            bestDelta = x - curX;
            nextIdx = i;
        }
    }

    s_selected_branch_index = (nextIdx >= 0) ? nextIdx : leftmostIdx;
}

void timelineSelectPrevBranch(void) {
    ensureValidSelection();
    if (s_selected_branch_index < 0) return;

    float curX = s_branches[s_selected_branch_index].start_x;
    int prevIdx = -1;
    float bestDelta = 9999.0f;
    int rightmostIdx = s_selected_branch_index;
    float rightmostX = curX;

    for (int i = 0; i < MAX_BRANCHES; i++) {
        if (!s_branches[i].active || i == s_selected_branch_index) continue;
        float x = s_branches[i].start_x;
        if (x > rightmostX) {
            rightmostX = x;
            rightmostIdx = i;
        }
        if (x < curX && (curX - x) < bestDelta) {
            bestDelta = curX - x;
            prevIdx = i;
        }
    }

    s_selected_branch_index = (prevIdx >= 0) ? prevIdx : rightmostIdx;
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

const VariantProfile* timelineGetSelectedVariant(void) {
    if (s_selected_branch_index >= 0 && s_selected_branch_index < MAX_BRANCHES &&
        s_branches[s_selected_branch_index].active) {
        int vIdx = s_branches[s_selected_branch_index].variant_index;
        if (vIdx >= 0 && vIdx < VARIANT_CATALOG_COUNT) {
            return &g_variantCatalog[vIdx];
        }
    }
    return NULL;
}

float timelineGetPhase(void) {
    return s_timelinePhase;
}

void timelineUpdate(void) {
    s_timelinePhase += TIMELINE_WAVE_SPEED;

    // Advance marquee offset by MARQUEE_SCROLL_SPEED (0.75f px/frame);
    // exact unitWidth wrap is applied in timelineRenderTopScreen using C2D_TextGetDimensions.
    s_marqueeX -= MARQUEE_SCROLL_SPEED;

    // Automatic branch generation (~every 18 seconds / 1080 frames at 60 FPS)
    s_spawnTimer++;
    if (s_spawnTimer >= BRANCH_SPAWN_INTERVAL_FRAMES) {
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
            // 4x slower pruning retraction (0.375 px/frame) to showcase tip fade-out
            br->current_length -= BRANCH_PRUNE_SPEED;
            if (br->current_length <= 0.0f) {
                br->current_length = 0.0f;
                br->active = false;
                br->is_pruning = false;
                br->is_critical = false;
                br->units_remaining = BRANCH_INITIAL_UNITS;
                continue;
            }
        } else {
            // 4x slower growth along X (0.026 px/frame) up to the top/bottom monitor border
            if (br->current_length < br->max_length) {
                br->current_length += BRANCH_GROWTH_SPEED;
                if (br->current_length > br->max_length) {
                    br->current_length = br->max_length;
                }
            }
        }

        // Evaluate tip Y position against solid red threshold lines
        float tipDy = (float)br->direction * br->curve_factor * powf(br->current_length, BRANCH_CURVE_EXPONENT);
        float tipY = br->start_y + tipDy;
        br->is_critical = (tipY <= TIMELINE_THRESHOLD_TOP || tipY >= TIMELINE_THRESHOLD_BOTTOM);

        // Calculate proportional UNITS countdown (5.00 at main line -> 0.00 at redline threshold)
        float distToThreshold = (br->direction < 0)
            ? (br->start_y - TIMELINE_THRESHOLD_TOP)
            : (TIMELINE_THRESHOLD_BOTTOM - br->start_y);
        if (distToThreshold < 1.0f) distToThreshold = 1.0f;

        float progress = fabsf(tipDy) / distToThreshold;
        if (progress >= 1.0f || br->is_critical) {
            br->units_remaining = 0.00f;
        } else {
            br->units_remaining = BRANCH_INITIAL_UNITS * (1.0f - progress);
        }
    }

    ensureValidSelection();
}

static void drawLeftColumnGlyphs(void) {
    bool hasSprites = assetsHasUiSheet();

    for (int g = 0; g < 5; g++) {
        float gx = GLYPH_COL_X;
        float gy = GLYPH_START_Y + (float)g * GLYPH_STEP_Y;

        if (hasSprites) {
            C2D_Image glyphImg = assetsGetUiImage((UiSpriteId)(UI_SPRITE_GLYPH_1 + g));
            if (glyphImg.tex != NULL) {
                C2D_DrawImageAt(glyphImg, gx, gy, 0.0f, NULL, 1.0f, 1.0f);
                continue;
            }
        }

        // Fallback seguro si el spritesheet no cargo
        float gs = GLYPH_BOX_SIZE;
        u32 col = COLOR_BRANCH_AMBER;
        drawBoxOutline(gx, gy, gs, gs, 1.2f, col);
    }
}

static void drawMonitorScaleNumbers(C2D_TextBuf textBufDynamic) {
    // Only clean "02", "03", "04" labels without any "00" or solid background box
    struct {
        const char* label;
        float y;
    } scaleMarks[] = {
        { "04", 48.0f  },
        { "03", 68.0f  },
        { "02", 88.0f  },
        { "02", 124.0f },
        { "03", 144.0f },
        { "04", 164.0f }
    };

    for (int i = 0; i < 6; i++) {
        C2D_Text txtNum;
        tempadParseText(&txtNum, textBufDynamic, scaleMarks[i].label);
        C2D_DrawText(&txtNum, C2D_WithColor, GRAPH_SCALE_NUM_X, scaleMarks[i].y, 0.0f,
                     SCALE_GRID_AXIS_NUM, SCALE_GRID_AXIS_NUM, COLOR_AMBER_DIM);
    }
}

// ---------------------------------------------------------
// Logo TVA (Sprite pre-coloreado en naranja TVA, sin tinte por hardware)
// ---------------------------------------------------------
static void drawTvaLogoPlaceholder(C2D_TextBuf textBufDynamic, float x, float y, float w, float h) {
    if (assetsHasUiSheet()) {
        C2D_Image logoImg = assetsGetUiImage(UI_SPRITE_TVA_LOGO);
        if (logoImg.tex != NULL) {
            C2D_DrawImageAt(logoImg, x, y, 0.0f, NULL, 1.0f, 1.0f);
            return;
        }
    }

    // Fallback seguro si el spritesheet no cargo
    C2D_DrawRectSolid(x, y, 0.0f, w, h, COLOR_TVA_LOGO_BG);
    C2D_Text txtLogo;
    tempadParseText(&txtLogo, textBufDynamic, "TVA");
    C2D_DrawText(&txtLogo, C2D_WithColor | C2D_AlignCenter, x + (w * 0.5f), y + 2.0f, 0.0f,
                 SCALE_TVA_LOGO_X, SCALE_TVA_LOGO_Y, COLOR_TVA_LOGO_TEXT);
}

// =============================================================================
// FUNCION EDITABLE A MANO: MARCOS PRINCIPALES Y CAJAS DE VARIANTE
// =============================================================================
// Edita aqui directamente las coordenadas (x1, y1) -> (x2, y2) del Segundo
// Rectangulo Grande, de las 4 Cajas de la Variante y del Logo de la TVA.
// =============================================================================
static void drawTopScreenMainFrames(C2D_TextBuf textBufDynamic, const VariantProfile* selVar, int selVarIdx, bool selCritical, u32 outerFrameCol) {
    u32 frameCol = COLOR_TVA_ORANGE;

    // -------------------------------------------------------------------------
    // A. CAJAS DE LA VARIANTE ADENTRO DEL SEGUNDO RECTANGULO (x = 287, w = 99)
    //    CONFIRM IDENTITY y Nombre sin fondo opaco para que la grilla pase detras.
    // -------------------------------------------------------------------------

    // 1. Caja 1 ("CONFIRM IDENTITY"): x = 287, y = 13, w = 99, h = 15 (transparente)
    const float box1X = 287.0f, box1Y = 13.0f, box1W = 99.0f, box1H = 15.0f;
    drawBoxOutline(box1X, box1Y, box1W, box1H, THICK_CARD_BORDER, frameCol);

    C2D_Text txtConfirm;
    tempadParseText(&txtConfirm, textBufDynamic, "CONFIRM IDENTITY");
    C2D_DrawText(&txtConfirm, C2D_WithColor | C2D_AlignCenter,
                 box1X + (box1W * 0.5f), box1Y + 0.5f, 0.0f,
                 SCALE_IDENTITY_HEADER_X, SCALE_IDENTITY_HEADER_Y, frameCol);

    // 2. Caja 2 (Visor de Foto): x = 287, y = 30, w = 99, h = 104
    const float box2X = 287.0f, box2Y = 30.0f, box2W = 99.0f, box2H = 104.0f;
    float cardCenterX = box2X + (box2W * 0.5f);

    if (selVar != NULL && selVarIdx >= 0 && assetsHasVariantSheet()) {
        C2D_Image varImg = assetsGetVariantImage(selVarIdx);
        if (varImg.tex != NULL) {
            // Centrar la foto de 94x98 px limpiamente dentro del borde de 99x104 px
            float imgX = box2X + (box2W - (float)varImg.subtex->width) * 0.5f;
            float imgY = box2Y + (box2H - (float)varImg.subtex->height) * 0.5f;
            C2D_DrawImageAt(varImg, imgX, imgY, 0.0f, NULL, 1.0f, 1.0f);
        }
    } else if (selVar != NULL && !assetsHasVariantSheet()) {
        // Fallback seguro unicamente si romfs:/gfx/variants.t3x no cargo
        u32 mugCol = selCritical ? COLOR_BRANCH_RED : COLOR_AMBER_TEXT;
        C2D_Text txtNoImg;
        tempadParseText(&txtNoImg, textBufDynamic, "NO FEED");
        C2D_DrawText(&txtNoImg, C2D_WithColor | C2D_AlignCenter,
                     cardCenterX, box2Y + 44.0f, 0.0f,
                     SCALE_IDENTITY_PHOTO_TAG, SCALE_IDENTITY_PHOTO_TAG, mugCol);
    }

    drawBoxOutline(box2X, box2Y, box2W, box2H, THICK_CARD_BORDER, selCritical ? COLOR_BRANCH_RED : frameCol);

    // 3. Caja 3 (Nombre de Variante): x = 287, y = 136, w = 99, h = 20 (transparente)
    const float box3X = 287.0f, box3Y = 136.0f, box3W = 99.0f, box3H = 20.0f;
    drawBoxOutline(box3X, box3Y, box3W, box3H, THICK_CARD_BORDER, frameCol);

    const char* varName = selVar ? selVar->name : "NO VARIANT";
    size_t nameLen = strlen(varName);
    float nameScaleY = SCALE_IDENTITY_NAME_Y;
    float nameScaleX = 0.52f;
    if ((float)nameLen * 13.0f * nameScaleX > 93.0f) {
        nameScaleX = 93.0f / ((float)nameLen * 13.0f);
        if (nameScaleX < 0.85f * nameScaleY) {
            nameScaleY = nameScaleX / 0.85f;
        }
    }
    u32 nameCol = selVar ? COLOR_TVA_ORANGE : COLOR_AMBER_DIM;

    C2D_Text txtVarName;
    tempadParseText(&txtVarName, textBufDynamic, varName);
    C2D_DrawText(&txtVarName, C2D_WithColor | C2D_AlignCenter,
                 cardCenterX, box3Y + 2.0f, 0.0f,
                 nameScaleX, nameScaleY, nameCol);

    // 4. Caja 4 (Etiqueta "VARIANT# XXXX"): x = 287, y = 158, w = 99, h = 20
    const float box4X = 287.0f, box4Y = 158.0f, box4W = 99.0f, box4H = 20.0f;
    u32 badgeBg = selVar ? COLOR_TVA_ORANGE : COLOR_BTN_NORMAL;
    u32 badgeTxt = selVar ? COLOR_TEXT_INVERTED : COLOR_AMBER_DIM;

    C2D_DrawRectSolid(box4X, box4Y, 0.0f, box4W, box4H, badgeBg);
    drawBoxOutline(box4X, box4Y, box4W, box4H, THICK_CARD_BORDER, frameCol);

    char badgeStr[32];
    snprintf(badgeStr, sizeof(badgeStr), "VARIANT# %s", selVar ? selVar->variant_id : "----");

    C2D_Text txtBadge;
    tempadParseText(&txtBadge, textBufDynamic, badgeStr);
    C2D_DrawText(&txtBadge, C2D_WithColor | C2D_AlignCenter,
                 cardCenterX, box4Y + 2.0f, 0.0f,
                 SCALE_IDENTITY_BADGE_X, SCALE_IDENTITY_BADGE_Y, badgeTxt);

    // -------------------------------------------------------------------------
    // B. SPRITE DEL LOGO TVA: x = 346, y = 196, w = 44, h = 20
    //    Esquina superior derecha: (390, 196)
    //    Esquina inferior izquierda: (346, 216)
    // -------------------------------------------------------------------------
    drawTvaLogoPlaceholder(textBufDynamic, CARD_TVA_LOGO_X, CARD_TVA_LOGO_Y, CARD_TVA_LOGO_W, CARD_TVA_LOGO_H);

    // -------------------------------------------------------------------------
    // C. LINEAS DEL SEGUNDO RECTANGULO GRANDE (x = 26, y = 10 -> x = 390, y = 216)
    //    Engloba tanto el monitor como las cajas de la variante y corta al
    //    chocar con el sprite del logo de la TVA:
    // -------------------------------------------------------------------------
    // 1. Linea Superior: desde (26, 10) hasta el fondo a la derecha en (390, 10)
    C2D_DrawLine(26.0f, 10.0f, frameCol, 390.0f, 10.0f, frameCol, THICK_INNER_FRAME, 0.0f);

    // 2. Linea Izquierda: desde (26, 10) hasta (26, 216)
    C2D_DrawLine(26.0f, 10.0f, frameCol, 26.0f, 216.0f, frameCol, THICK_INNER_FRAME, 0.0f);

    // 3. Linea Derecha: baja desde (390, 10) y corta justo encima del logo TVA en (390, 193)
    C2D_DrawLine(390.0f, 10.0f, frameCol, 390.0f, 193.0f, frameCol, THICK_INNER_FRAME, 0.0f);

    // 4. Linea Inferior: va desde (26, 216) y corta justo antes del logo TVA en (343, 216)
    C2D_DrawLine(26.0f, 216.0f, frameCol, 343.0f, 216.0f, frameCol, THICK_INNER_FRAME, 0.0f);

    // -------------------------------------------------------------------------
    // D. RECTANGULO 1 (MARCO EXTERIOR GIGANTE): x = 4, y = 4, w = 392, h = 232
    // -------------------------------------------------------------------------
    drawBoxOutline(OUTER_FRAME_X, OUTER_FRAME_Y, OUTER_FRAME_W, OUTER_FRAME_H, THICK_OUTER_FRAME, outerFrameCol);
}

void timelineRenderTopScreen(C3D_RenderTarget* topScreen, C2D_TextBuf textBufDynamic, AppState currentState, const char* activeLocationName) {
    (void)activeLocationName;

    // Pre-evaluar estado de alertas sobre ramas activas
    int criticalCount = 0;
    bool anyUnderThreeUnits = false;
    for (int i = 0; i < MAX_BRANCHES; i++) {
        if (!s_branches[i].active || s_branches[i].current_length <= 0.05f) continue;
        if (s_branches[i].is_critical) criticalCount++;
        if (s_branches[i].units_remaining < BRANCH_REDLINE_ALERT_UNITS) anyUnderThreeUnits = true;
    }

    // =========================================================
    // CAPA 1: Fondo negro (#080400)
    // =========================================================
    C2D_TargetClear(topScreen, COLOR_BG_DARK);
    C2D_SceneBegin(topScreen);

    // =========================================================
    // CAPA 2: Marquesina "REDLINE IMMINENT" + mascaras negras SOLO en franja superior (y = 10..32)
    // =========================================================
    bool blinkMultiverse = false;
    if (criticalCount >= 3) {
        blinkMultiverse = (((int)(s_timelinePhase * 3.0f)) % 2 == 0);
        if (blinkMultiverse) {
            C2D_Text txtDanger;
            tempadParseText(&txtDanger, textBufDynamic, "DANGER: MULTIVERSE");
            C2D_DrawText(&txtDanger, C2D_WithColor | C2D_AlignCenter,
                         GRAPH_BOX_X + (GRAPH_BOX_W * 0.5f), GRAPH_BOX_Y + 7.0f, 0.0f,
                         SCALE_REDLINE_ALERT_X, SCALE_REDLINE_ALERT_Y, COLOR_BRANCH_RED);
        }
    } else if (anyUnderThreeUnits) {
        const char* singleUnitStr = "REDLINE IMMINENT     ";
        C2D_Text txtUnit;
        tempadParseText(&txtUnit, textBufDynamic, singleUnitStr);

        float unitWidth = 0.0f, unitHeight = 0.0f;
        C2D_TextGetDimensions(&txtUnit, SCALE_REDLINE_ALERT_X, SCALE_REDLINE_ALERT_Y, &unitWidth, &unitHeight);
        if (unitWidth < 20.0f) {
            unitWidth = 21.0f * 13.0f * SCALE_REDLINE_ALERT_X;
        }

        while (s_marqueeX <= -unitWidth) {
            s_marqueeX += unitWidth;
        }

        for (int rep = 0; rep < 4; rep++) {
            float drawX = 28.0f + s_marqueeX + ((float)rep * unitWidth);
            C2D_DrawText(&txtUnit, C2D_WithColor, drawX, 11.5f, 0.0f,
                         SCALE_REDLINE_ALERT_X, SCALE_REDLINE_ALERT_Y, COLOR_BRANCH_RED);
        }

        // Mascaras negras estrictas unicamente en la franja superior (y = 10..32)
        C2D_DrawRectSolid(0.0f, 10.0f, 0.0f, GRAPH_BOX_X, 22.0f, COLOR_BG_DARK);
        C2D_DrawRectSolid(GRAPH_RIGHT_X, 10.0f, 0.0f, TOP_WIDTH - GRAPH_RIGHT_X, 22.0f, COLOR_BG_DARK);
    }

    // =========================================================
    // CAPA 3: Grilla completa sobre todo el interior del Rectangulo 1 (4..396 x 4..236)
    // =========================================================
    for (float x = OUTER_FRAME_X + 22.0f; x < (OUTER_FRAME_X + OUTER_FRAME_W); x += 28.0f) {
        C2D_DrawLine(x, OUTER_FRAME_Y + 1.0f, COLOR_GRID_LINE, x, OUTER_FRAME_Y + OUTER_FRAME_H - 1.0f, COLOR_GRID_LINE, THICK_GRID_LINE, 0.0f);
    }
    for (float y = OUTER_FRAME_Y + 20.0f; y < (OUTER_FRAME_Y + OUTER_FRAME_H); y += 28.0f) {
        C2D_DrawLine(OUTER_FRAME_X + 1.0f, y, COLOR_GRID_LINE, OUTER_FRAME_X + OUTER_FRAME_W - 1.0f, y, COLOR_GRID_LINE, THICK_GRID_LINE, 0.0f);
    }

    // =========================================================
    // CAPA 4: Lineas rojas, ramas, Sagrada Linea Temporal, reticula '[ ]' y telemetria
    // =========================================================
    // Solid, Continuous, Thick Red Threshold Lines across the graph width (x = 26..284)
    C2D_DrawLine(GRAPH_BOX_X + 1.0f, TIMELINE_THRESHOLD_TOP, COLOR_REDLINE_SOLID,
                 GRAPH_RIGHT_X, TIMELINE_THRESHOLD_TOP, COLOR_REDLINE_SOLID, THICK_THRESHOLD_LINE, 0.0f);
    C2D_DrawLine(GRAPH_BOX_X + 1.0f, TIMELINE_THRESHOLD_BOTTOM, COLOR_REDLINE_SOLID,
                 GRAPH_RIGHT_X, TIMELINE_THRESHOLD_BOTTOM, COLOR_REDLINE_SOLID, THICK_THRESHOLD_LINE, 0.0f);

    // Clean Scale Numbers ("02", "03", "04" only, no "00")
    drawMonitorScaleNumbers(textBufDynamic);

    // Decorative Bottom-Left Grid Telemetry (shortened to "MAG" and "SEG" for larger crisp font)
    C2D_Text txtMag, txtSeg;
    tempadParseText(&txtMag, textBufDynamic, "MAG: 70.62.0738");
    C2D_DrawText(&txtMag, C2D_WithColor, GRID_DECOR_X, GRID_DECOR_MAG_Y, 0.0f, SCALE_GRID_DECOR, SCALE_GRID_DECOR, COLOR_AMBER_DIM);

    tempadParseText(&txtSeg, textBufDynamic, "SEG: 616.432");
    C2D_DrawText(&txtSeg, C2D_WithColor, GRID_DECOR_X, GRID_DECOR_SEG_Y, 0.0f, SCALE_GRID_DECOR, SCALE_GRID_DECOR, COLOR_AMBER_DIM);

    // Variant Branches & Clamped '[ ]' Reticle
    for (int i = 0; i < MAX_BRANCHES; i++) {
        TimelineBranch* br = &s_branches[i];
        if (!br->active || br->current_length <= 0.05f) continue;

        bool isSelected = (i == s_selected_branch_index);
        float baseCoreThick = isSelected ? THICK_BRANCH_SEL_CORE : THICK_BRANCH_CORE;
        float baseGlowThick = isSelected ? THICK_BRANCH_SEL_GLOW : THICK_BRANCH_GLOW;

        float bPrevX = br->start_x;
        float bPrevY = getSacredTimelineY(br->start_x, s_timelinePhase);

        float fadeZoneLen = (float)BRANCH_FADE_SEGMENTS * BRANCH_STEP_X;
        float dx = 0.0f;

        while (dx < br->current_length) {
            float nextDx = dx + BRANCH_STEP_X;
            if (nextDx > br->current_length) {
                nextDx = br->current_length;
            }

            float bx = br->start_x + nextDx;
            float dy = (float)br->direction * br->curve_factor * powf(nextDx, BRANCH_CURVE_EXPONENT);
            float by = br->start_y + dy;

            if (by < GRAPH_BOX_Y) by = GRAPH_BOX_Y;
            if (by > GRAPH_BOTTOM_Y) by = GRAPH_BOTTOM_Y;

            u8 coreAlpha = BRANCH_FADE_MAX_ALPHA;
            u8 glowAlpha = 115;
            float segCoreThick = baseCoreThick;
            float segGlowThick = baseGlowThick;

            if (br->is_pruning) {
                float distFromTip = br->current_length - nextDx;
                if (distFromTip < fadeZoneLen) {
                    float fadeT = distFromTip / fadeZoneLen;
                    coreAlpha = (u8)(BRANCH_FADE_MIN_ALPHA +
                                     (float)(BRANCH_FADE_MAX_ALPHA - BRANCH_FADE_MIN_ALPHA) * fadeT);
                    glowAlpha = (u8)(10.0f + 105.0f * fadeT);
                    segCoreThick = THICK_BRANCH_PRUNE_MIN + (baseCoreThick - THICK_BRANCH_PRUNE_MIN) * fadeT;
                    segGlowThick = segCoreThick + 1.2f * fadeT;
                }
            }

            u32 segGlowCol = makeGlowColorWithAlpha(br->is_critical, glowAlpha);
            u32 segCoreCol = makeBranchColorWithAlpha(br->is_critical, isSelected, coreAlpha);

            C2D_DrawLine(bPrevX, bPrevY, segGlowCol, bx, by, segGlowCol, segGlowThick, 0.0f);
            C2D_DrawLine(bPrevX, bPrevY, segCoreCol, bx, by, segCoreCol, segCoreThick, 0.0f);

            bPrevX = bx;
            bPrevY = by;
            dx = nextDx;
        }

        // Discreet Target Reticle '[ ]' with Y clamped between 15 and 211
        if (isSelected) {
            float tipX = br->start_x + br->current_length;
            float tipDy = (float)br->direction * br->curve_factor * powf(br->current_length, BRANCH_CURVE_EXPONENT);
            float tipY = br->start_y + tipDy;
            if (tipY < RETICLE_CLAMP_MIN_Y) tipY = RETICLE_CLAMP_MIN_Y;
            if (tipY > RETICLE_CLAMP_MAX_Y) tipY = RETICLE_CLAMP_MAX_Y;

            u32 retCol = br->is_critical ? COLOR_BRANCH_RED : COLOR_BRANCH_GOLD_SEL;
            float r = RETICLE_HALF_SIZE;
            float arm = RETICLE_ARM_LEN;
            float rt = RETICLE_THICKNESS;

            // Left bracket '['
            C2D_DrawLine(tipX - r, tipY - r, retCol, tipX - r + arm, tipY - r, retCol, rt, 0.0f);
            C2D_DrawLine(tipX - r, tipY - r, retCol, tipX - r, tipY + r, retCol, rt, 0.0f);
            C2D_DrawLine(tipX - r, tipY + r, retCol, tipX - r + arm, tipY + r, retCol, rt, 0.0f);
            // Right bracket ']'
            C2D_DrawLine(tipX + r - arm, tipY - r, retCol, tipX + r, tipY - r, retCol, rt, 0.0f);
            C2D_DrawLine(tipX + r, tipY - r, retCol, tipX + r, tipY + r, retCol, rt, 0.0f);
            C2D_DrawLine(tipX + r - arm, tipY + r, retCol, tipX + r, tipY + r, retCol, rt, 0.0f);
        }
    }

    // Sacred Timeline (Muted Silver / Gray across Monitor x = 26..284)
    float prevX = GRAPH_BOX_X + 2.0f;
    float prevY = getSacredTimelineY(prevX, s_timelinePhase);

    for (float x = prevX + SACRED_STEP_X; x <= GRAPH_RIGHT_X; x += SACRED_STEP_X) {
        float curY = getSacredTimelineY(x, s_timelinePhase);

        C2D_DrawLine(prevX, prevY, COLOR_SACRED_GLOW, x, curY, COLOR_SACRED_GLOW, THICK_SACRED_GLOW, 0.0f);
        C2D_DrawLine(prevX, prevY, COLOR_SACRED_CORE, x, curY, COLOR_SACRED_CORE, THICK_SACRED_CORE, 0.0f);

        prevX = x;
        prevY = curY;
    }

    // UNITS Countdown (z = 0.0f)
    float displayUnits = BRANCH_INITIAL_UNITS;
    u32 unitsCol = COLOR_AMBER_DIM;

    if (s_selected_branch_index >= 0 && s_branches[s_selected_branch_index].active) {
        TimelineBranch* selBr = &s_branches[s_selected_branch_index];
        displayUnits = selBr->units_remaining;
        unitsCol = selBr->is_critical ? COLOR_BRANCH_RED : COLOR_TVA_ORANGE;
    }

    char unitsBuf[16];
    snprintf(unitsBuf, sizeof(unitsBuf), "%.2f", displayUnits);

    C2D_Text txtUnitsVal, txtUnitsLbl;
    tempadParseText(&txtUnitsVal, textBufDynamic, unitsBuf);
    C2D_DrawText(&txtUnitsVal, C2D_WithColor, GRAPH_BOX_X + 6.0f, TIMELINE_THRESHOLD_TOP + 3.0f, 0.0f,
                 SCALE_UNITS_VALUE, SCALE_UNITS_VALUE, unitsCol);

    tempadParseText(&txtUnitsLbl, textBufDynamic, "UNITS");
    C2D_DrawText(&txtUnitsLbl, C2D_WithColor, GRAPH_BOX_X + 6.0f, TIMELINE_THRESHOLD_TOP + 26.0f, 0.0f,
                 SCALE_UNITS_LABEL, SCALE_UNITS_LABEL, unitsCol);

    // Visual Timedoor Breach when Active
    if (currentState == STATE_TIMEDOOR_ACTIVE) {
        float breachX = GRAPH_BOX_X + (GRAPH_BOX_W * 0.5f);
        float pulse = 4.0f * sinf(s_timelinePhase * 4.0f);

        C2D_DrawLine(breachX, 54.0f, COLOR_BRANCH_AMBER_GLOW, breachX, 172.0f, COLOR_BRANCH_AMBER_GLOW, 5.0f, 0.0f);
        C2D_DrawLine(breachX, 54.0f, COLOR_BRANCH_SELECTED, breachX, 172.0f, COLOR_BRANCH_SELECTED, 2.0f, 0.0f);

        float ringRadius = 13.0f + pulse;
        drawBoxOutline(breachX - ringRadius, TIMELINE_BASELINE_Y - ringRadius, ringRadius * 2.0f, ringRadius * 2.0f, 1.5f, COLOR_TVA_ORANGE);
    }

    // =========================================================
    // CAPA 5: Glifos, Cajas de Variante, Foto, Barra VARIANT#, Logo TVA, Marcos y Telemetria Inferior
    // =========================================================
    const VariantProfile* selVar = timelineGetSelectedVariant();
    TimelineBranch* selBr = (s_selected_branch_index >= 0 && s_branches[s_selected_branch_index].active)
                            ? &s_branches[s_selected_branch_index] : NULL;
    bool selCritical = (selBr != NULL && selBr->is_critical);

    // 5 Glifos de la columna izquierda
    drawLeftColumnGlyphs();

    // Cajas de la variante + Logo TVA + Segundo Rectangulo que corta en TVA + Marco Exterior
    int selVarIdx = (selBr != NULL) ? selBr->variant_index : -1;
    u32 outerFrameCol = (criticalCount >= 3 && blinkMultiverse) ? COLOR_BRANCH_RED : COLOR_TVA_ORANGE;
    drawTopScreenMainFrames(textBufDynamic, selVar, selVarIdx, selCritical, outerFrameCol);

    // Barra inferior de Fecha, Coordenadas y Energia (z = 0.0f)
    const char* dateStr = selVar ? selVar->date_str : "--.--.----";
    const char* coordsStr = selVar ? selVar->coords_str : "0000.000 // 0000.000";
    u32 telemCol = selVar ? COLOR_TVA_ORANGE : COLOR_AMBER_DIM;

    char terminalLine[64];
    snprintf(terminalLine, sizeof(terminalLine), "%s   %s", dateStr, coordsStr);

    C2D_Text txtTerminal;
    tempadParseText(&txtTerminal, textBufDynamic, terminalLine);
    C2D_DrawText(&txtTerminal, C2D_WithColor, TERMINAL_DATE_X, TERMINAL_LINE_Y, 0.0f,
                 SCALE_TERMINAL_BOTTOM_X, SCALE_TERMINAL_BOTTOM_Y, telemCol);

    float energyVal = 13.0f;
    if (selBr != NULL) {
        float jitter = 0.2f * sinf(s_timelinePhase * 1.7f + (float)s_selected_branch_index);
        energyVal = selBr->energy_gc + jitter;
        if (energyVal < 10.0f) energyVal = 10.0f;
        if (energyVal > 89.9f) energyVal = 89.9f;
    }

    char energyBuf[24];
    snprintf(energyBuf, sizeof(energyBuf), "E: %.1fGc", energyVal);

    C2D_Text txtEnergy;
    tempadParseText(&txtEnergy, textBufDynamic, energyBuf);
    C2D_DrawText(&txtEnergy, C2D_WithColor, TERMINAL_ENERGY_X, TERMINAL_LINE_Y, 0.0f,
                 SCALE_TVA_CODE_X, SCALE_TVA_CODE_Y, telemCol);
}
