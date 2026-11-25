#include "assets.h"
#include <stddef.h>

static C2D_SpriteSheet s_uiSheet = NULL;
static C2D_SpriteSheet s_variantSheet = NULL;

void assetsInit(void) {
    s_uiSheet = C2D_SpriteSheetLoad("romfs:/gfx/ui_sprites.t3x");
    s_variantSheet = C2D_SpriteSheetLoad("romfs:/gfx/variants.t3x");
}

void assetsExit(void) {
    if (s_uiSheet) {
        C2D_SpriteSheetFree(s_uiSheet);
        s_uiSheet = NULL;
    }
    if (s_variantSheet) {
        C2D_SpriteSheetFree(s_variantSheet);
        s_variantSheet = NULL;
    }
}

bool assetsHasUiSheet(void) {
    return (s_uiSheet != NULL);
}

bool assetsHasVariantSheet(void) {
    return (s_variantSheet != NULL);
}

C2D_Image assetsGetUiImage(UiSpriteId id) {
    if (s_uiSheet && (int)id >= 0 && (size_t)id < C2D_SpriteSheetCount(s_uiSheet)) {
        return C2D_SpriteSheetGetImage(s_uiSheet, (size_t)id);
    }
    C2D_Image empty = { NULL, NULL };
    return empty;
}

C2D_Image assetsGetDashboardIcon(int appIndex, bool isSelected) {
    if (appIndex < 0 || appIndex >= 4) {
        C2D_Image empty = { NULL, NULL };
        return empty;
    }
    UiSpriteId base = isSelected ? UI_SPRITE_ICON_TIMEDOOR_GLOW : UI_SPRITE_ICON_TIMEDOOR_DIM;
    return assetsGetUiImage((UiSpriteId)(base + appIndex));
}

C2D_Image assetsGetVariantImage(int variantIndex) {
    if (s_variantSheet && variantIndex >= 0 && (size_t)variantIndex < C2D_SpriteSheetCount(s_variantSheet)) {
        return C2D_SpriteSheetGetImage(s_variantSheet, (size_t)variantIndex);
    }
    C2D_Image empty = { NULL, NULL };
    return empty;
}
