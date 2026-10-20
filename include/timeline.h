#ifndef TIMELINE_H
#define TIMELINE_H

#include "tempad.h"

// ---------------------------------------------------------
// Nexus Branch Structure
// ---------------------------------------------------------
typedef struct {
    bool active;
    float startX;           // X coordinate where divergence begins
    float deviation;        // Current deviation in pixels
    float targetDeviation;  // Target deviation toward threshold
    float direction;        // +1.0f (up) or -1.0f (down)
} NexusBranch;

// ---------------------------------------------------------
// Timeline Module API
// ---------------------------------------------------------
void timelineInit(void);
void timelineUpdate(void);
void timelineTriggerBranch(void);
void timelinePruneAll(void);
float timelineGetPhase(void);
void timelineRenderTopScreen(C3D_RenderTarget* topScreen, C2D_TextBuf textBufDynamic, AppState currentState, const char* activeLocationName);

#endif // TIMELINE_H

