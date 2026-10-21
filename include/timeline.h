#ifndef TIMELINE_H
#define TIMELINE_H

#include "tempad.h"

// ---------------------------------------------------------
// Multi-Branch System Configuration
// ---------------------------------------------------------
#define MAX_BRANCHES 10

typedef struct {
    bool active;
    float start_x;        // X point where it spawns on the main line
    float start_y;        // Fixed Y position where it spawned (no floating wave)
    int direction;        // -1 upwards, +1 downwards
    float current_length; // Current length along X axis as it grows
    float max_length;     // Maximum length up to edge or outside grid
    float curve_factor;   // Slope / growth factor
    bool is_critical;     // True when tip crosses red dotted threshold line
    bool is_pruning;      // True when branch is being pruned/disintegrated
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
float timelineGetPhase(void);
void timelineRenderTopScreen(C3D_RenderTarget* topScreen, C2D_TextBuf textBufDynamic, AppState currentState, const char* activeLocationName);

#endif // TIMELINE_H
