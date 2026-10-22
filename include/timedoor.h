#ifndef TIMEDOOR_H
#define TIMEDOOR_H

#include "tempad.h"
#include "tva_catalog.h"

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
