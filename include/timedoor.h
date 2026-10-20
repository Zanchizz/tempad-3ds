#ifndef TIMEDOOR_H
#define TIMEDOOR_H

#include "tempad.h"

// ---------------------------------------------------------
// Temporal Location Structure ("Saved Times")
// ---------------------------------------------------------
typedef struct Location {
    const char* name;
    const char* coords;
    const char* era;
} Location;

// ---------------------------------------------------------
// Timedoor Module API
// ---------------------------------------------------------
void timedoorInit(void);
void timedoorUpdate(void);
void timedoorHandleInput(u32 kDown, u32 kHeld, touchPosition touch, AppState* currentState);
void timedoorRenderSelect(C2D_TextBuf textBufDynamic, float animPhase);
void timedoorRenderActive(C2D_TextBuf textBufDynamic, float animPhase);
const Location* timedoorGetSelectedLocation(void);

#endif // TIMEDOOR_H

