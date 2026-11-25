#ifndef TIMELINE_H
#define TIMELINE_H

#include "tempad.h"
#include "tva_catalog.h"

// ---------------------------------------------------------
// Multi-Branch System Configuration
// ---------------------------------------------------------
#define MAX_BRANCHES 10

typedef struct {
    bool active;
    int variant_index;      // Index into g_variantCatalog[0..9]
    float start_x;          // Chronological X point where it spawns on the main line
    float start_y;          // Fixed Y position where it spawned (no floating wave)
    int direction;          // -1 upwards, +1 downwards
    float current_length;   // Current length along X axis as it grows
    float max_length;       // Maximum length inside graph area
    float curve_factor;     // Slope / exponential growth factor
    float units_remaining;  // Countdown from 5.00 to 0.00 before touching redline
    float energy_gc;        // Random temporal energy reading (e.g. 10.0 to 89.9 Gc)
    bool is_critical;       // True when tip crosses solid red threshold line
    bool is_pruning;        // True when branch is being pruned/disintegrated
} TimelineBranch;

// ---------------------------------------------------------
// Timeline Module API
// ---------------------------------------------------------
void timelineInit(void);
void timelineUpdate(void);
void timelineTriggerBranch(void);
void timelinePruneAll(void);
void timelineSelectNextBranch(void);
void timelineSelectPrevBranch(void);
bool timelinePruneSelected(void);
int timelineGetActiveCount(void);
int timelineGetSelectedBranchIndex(void);
const VariantProfile* timelineGetSelectedVariant(void);
float timelineGetPhase(void);
void timelineRenderTopScreen(C3D_RenderTarget* topScreen, C2D_TextBuf textBufDynamic, AppState currentState, const char* activeLocationName);

#endif // TIMELINE_H
