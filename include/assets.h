#ifndef ASSETS_H
#define ASSETS_H

#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>

// ---------------------------------------------------------
// UI Sprite Sheet Indices (Exact order of gfx/ui_sprites.t3s)
// ---------------------------------------------------------
typedef enum {
    UI_SPRITE_TVA_LOGO = 0,
    UI_SPRITE_GLYPH_1,
    UI_SPRITE_GLYPH_2,
    UI_SPRITE_GLYPH_3,
    UI_SPRITE_GLYPH_4,
    UI_SPRITE_GLYPH_5,
    UI_SPRITE_ICON_TIMEDOOR_DIM,
    UI_SPRITE_ICON_MISS_MINUTES_DIM,
    UI_SPRITE_ICON_SETTINGS_DIM,
    UI_SPRITE_ICON_DIRECTORY_DIM,
    UI_SPRITE_ICON_TIMEDOOR_GLOW,
    UI_SPRITE_ICON_MISS_MINUTES_GLOW,
    UI_SPRITE_ICON_SETTINGS_GLOW,
    UI_SPRITE_ICON_DIRECTORY_GLOW,
    UI_SPRITE_COUNT
} UiSpriteId;

// ---------------------------------------------------------
// Assets Lifecycle & Lookup API
// ---------------------------------------------------------
void assetsInit(void);
void assetsExit(void);

bool assetsHasUiSheet(void);
bool assetsHasVariantSheet(void);

C2D_Image assetsGetUiImage(UiSpriteId id);
C2D_Image assetsGetDashboardIcon(int appIndex, bool isSelected);
C2D_Image assetsGetVariantImage(int variantIndex);

#endif // ASSETS_H
