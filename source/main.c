#include <3ds.h>
#include <citro2d.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------
// Dimensiones de pantalla de Nintendo 3DS
// ---------------------------------------------------------
#define TOP_WIDTH        400
#define TOP_HEIGHT       240
#define BOTTOM_WIDTH     320
#define BOTTOM_HEIGHT    240

// ---------------------------------------------------------
// Paleta TVA (Time Variance Authority) - Fósforo Ámbar CRT
// ---------------------------------------------------------
#define CLR_BG_DARK             C2D_Color32(12, 8, 4, 255)
#define CLR_PANEL_BG            C2D_Color32(22, 15, 9, 255)
#define CLR_PANEL_INSET         C2D_Color32(14, 9, 5, 255)
#define CLR_BORDER_DIM          C2D_Color32(65, 40, 16, 255)
#define CLR_BORDER_MID          C2D_Color32(110, 68, 24, 255)
#define CLR_GRID                C2D_Color32(36, 22, 8, 140)

#define CLR_AMBER_BRIGHT        C2D_Color32(255, 195, 45, 255)
#define CLR_AMBER_CORE          C2D_Color32(255, 245, 190, 255)
#define CLR_AMBER_GLOW          C2D_Color32(255, 125, 0, 110)
#define CLR_AMBER_TEXT          C2D_Color32(255, 175, 35, 255)
#define CLR_AMBER_DARK          C2D_Color32(120, 65, 15, 255)
#define CLR_TEXT_INVERTED       C2D_Color32(18, 10, 4, 255)

#define CLR_CRITICAL_RED        C2D_Color32(245, 45, 25, 255)
#define CLR_RED_GLOW            C2D_Color32(220, 35, 15, 130)

#define CLR_BTN_NORMAL          C2D_Color32(46, 28, 15, 255)
#define CLR_BTN_NORMAL_BORDER   C2D_Color32(140, 85, 25, 255)
#define CLR_BTN_PRESSED         C2D_Color32(255, 150, 10, 255)
#define CLR_BTN_PRESSED_BORDER  C2D_Color32(255, 225, 120, 255)

#define CLR_PURGE_BTN_NORMAL    C2D_Color32(65, 20, 14, 255)
#define CLR_PURGE_BTN_BORDER    C2D_Color32(180, 45, 20, 255)
#define CLR_PURGE_BTN_PRESSED   C2D_Color32(245, 45, 25, 255)

// ---------------------------------------------------------
// 1. MÁQUINA DE ESTADOS (UI STATE)
// ---------------------------------------------------------
typedef enum {
    STATE_DASHBOARD,
    STATE_TIMEDOOR_SELECT,
    STATE_TIMEDOOR_ACTIVE
} AppState;

// ---------------------------------------------------------
// Estructuras: Dashboard (Matriz 2x2 + Action List)
// ---------------------------------------------------------
typedef enum {
    DASH_APP_TIMEDOOR = 0,
    DASH_APP_MISS_MINUTES,
    DASH_APP_SETTINGS,
    DASH_APP_DIRECTORY,
    DASH_APP_COUNT
} DashboardAppId;

typedef struct {
    float x, y, w, h;
    const char* title;
    const char* code;
} DashboardTileDef;

typedef enum {
    DASH_ACT_RUN = 0,
    DASH_ACT_STATUS,
    DASH_ACT_PURGE,
    DASH_ACT_COUNT
} DashboardActionId;

typedef struct {
    float x, y, w, h;
    const char* label;
} DashboardActionDef;

// ---------------------------------------------------------
// Estructuras: Submenú "Saved Times"
// ---------------------------------------------------------
#define DESTINATION_COUNT 4

typedef struct {
    const char* name;
    const char* coords;
    const char* era;
} DestinationDef;

static const DestinationDef g_destinations[DESTINATION_COUNT] = {
    { "1705, LAMENTIS", "0742.11.LMT1.9021", "SECTOR C-2 // EXTINCION" },
    { "2012, EARTH",    "1255.92.FGV3.8534", "NUEVA YORK // BATTLE OF NY" },
    { "1412, TITAN",    "0412.88.TTN9.1102", "GALAXIA M-31 // PRE-COLAPSO" },
    { "2077, ASGARD",   "2077.01.ASG4.6549", "REINO DORADO // CATACLISMO" }
};

// ---------------------------------------------------------
// Estructura de la Sagrada Línea Temporal
// ---------------------------------------------------------
typedef struct {
    bool active;
    float startX;           // Coordenada X donde inicia la divergencia
    float deviation;        // Desviación en píxeles
    float targetDeviation;  // Desviación objetivo hacia el umbral
    float direction;        // +1.0f (arriba) o -1.0f (abajo)
} NexusBranch;

// ---------------------------------------------------------
// Estado Global de la Aplicación TemPad
// ---------------------------------------------------------
typedef struct {
    AppState currentState;

    C3D_RenderTarget* topScreen;
    C3D_RenderTarget* bottomScreen;

    // Buffers de texto citro2d
    C2D_TextBuf textBufStatic;
    C2D_TextBuf textBufDynamic;

    // Elementos del Dashboard
    DashboardTileDef dashTiles[DASH_APP_COUNT];
    DashboardActionDef dashActions[DASH_ACT_COUNT];
    int selectedDashApp;
    int pressedDashTile;    // Para feedback táctil continuo (-1 si ninguno)
    int pressedDashAction;  // Para feedback táctil en lista de acciones (-1 si ninguno)

    // Elementos de Saved Times
    int selectedDestination;
    int pressedDestCell;    // Celda de destino bajo el stylus (-1 si ninguno)
    int pressedScrollBtn;   // 1: Arriba, 2: Abajo, 0: Ninguno
    bool pressedBackButton; // Botón táctil '< VOLVER' presionado

    // Detección de doble toque táctil
    int lastTappedApp;
    int lastTappedDest;
    int tapTimer;           // Contador de frames entre toques

    // Dinámica de la Sagrada Línea Temporal
    float timelinePhase;
    NexusBranch branches[2];
    int purgeFlashFrames;   // Efecto visual de Carga de Reinicio TVA

    // Telemetría en pantalla táctil
    char statusMessage[64];
} TemPadApp;

// ---------------------------------------------------------
// Primitivas de Dibujo Auxiliares
// ---------------------------------------------------------
static void drawBoxOutline(float x, float y, float w, float h, float thickness, u32 col) {
    C2D_DrawLine(x, y, col, x + w, y, col, thickness, 0.0f);             // Arriba
    C2D_DrawLine(x, y + h, col, x + w, y + h, col, thickness, 0.0f);     // Abajo
    C2D_DrawLine(x, y, col, x, y + h, col, thickness, 0.0f);             // Izquierda
    C2D_DrawLine(x + w, y, col, x + w, y + h, col, thickness, 0.0f);     // Derecha
}

// ---------------------------------------------------------
// Inicialización y Configuración de Componentes UI
// ---------------------------------------------------------
static void setupDashboard(TemPadApp* app) {
    app->selectedDashApp = DASH_APP_TIMEDOOR;
    app->pressedDashTile = -1;
    app->pressedDashAction = -1;

    // Matriz 2x2 en la zona izquierda (x: 10 a 206)
    const float tileW = 94.0f;
    const float tileH = 76.0f;
    const float col0X = 10.0f;
    const float col1X = 110.0f;
    const float row0Y = 44.0f;
    const float row1Y = 126.0f;

    app->dashTiles[DASH_APP_TIMEDOOR]    = (DashboardTileDef){ col0X, row0Y, tileW, tileH, "TIMEDOOR",     "PRG // 001" };
    app->dashTiles[DASH_APP_MISS_MINUTES]= (DashboardTileDef){ col1X, row0Y, tileW, tileH, "MISS MINUTES", "AI // SYS"  };
    app->dashTiles[DASH_APP_SETTINGS]    = (DashboardTileDef){ col0X, row1Y, tileW, tileH, "SETTINGS",     "CFG // 042" };
    app->dashTiles[DASH_APP_DIRECTORY]   = (DashboardTileDef){ col1X, row1Y, tileW, tileH, "DIRECTORY",    "LOG // TVA" };

    // Action List en la zona derecha (x: 212 a 310)
    const float actX = 212.0f;
    const float actW = 98.0f;
    const float actH = 38.0f;

    app->dashActions[DASH_ACT_RUN]    = (DashboardActionDef){ actX, 66.0f,  actW, actH, "RUN PROGRAM" };
    app->dashActions[DASH_ACT_STATUS] = (DashboardActionDef){ actX, 110.0f, actW, actH, "INFO ESTADO" };
    app->dashActions[DASH_ACT_PURGE]  = (DashboardActionDef){ actX, 154.0f, actW, actH, "PURGAR RAMA" };
}

static void initTimeline(TemPadApp* app) {
    app->timelinePhase = 0.0f;
    app->purgeFlashFrames = 0;

    app->branches[0].active = false;
    app->branches[0].startX = 130.0f;
    app->branches[0].deviation = 0.0f;
    app->branches[0].targetDeviation = 0.0f;
    app->branches[0].direction = 1.0f;

    app->branches[1].active = false;
    app->branches[1].startX = 180.0f;
    app->branches[1].deviation = 0.0f;
    app->branches[1].targetDeviation = 0.0f;
    app->branches[1].direction = -1.0f;

    strncpy(app->statusMessage, "SISTEMA NOMINAL // SAGRADA LINEA", sizeof(app->statusMessage) - 1);
}

static bool initApp(TemPadApp* app) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    app->topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    app->bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    app->textBufStatic = C2D_TextBufNew(2048);
    app->textBufDynamic = C2D_TextBufNew(2048);

    if (!app->textBufStatic || !app->textBufDynamic) {
        return false;
    }

    app->currentState = STATE_DASHBOARD;
    app->selectedDestination = 1; // Por defecto: 2012, EARTH
    app->pressedDestCell = -1;
    app->pressedScrollBtn = 0;
    app->pressedBackButton = false;

    app->lastTappedApp = -1;
    app->lastTappedDest = -1;
    app->tapTimer = 99;

    setupDashboard(app);
    initTimeline(app);

    return true;
}

static void freeApp(TemPadApp* app) {
    if (app->textBufDynamic) C2D_TextBufDelete(app->textBufDynamic);
    if (app->textBufStatic)  C2D_TextBufDelete(app->textBufStatic);

    C2D_Fini();
    C3D_Fini();
    gfxExit();
}

// ---------------------------------------------------------
// Manejo de Entrada y Máquina de Estados
// ---------------------------------------------------------
static void handleInput(TemPadApp* app) {
    hidScanInput();
    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();

    // 1. Botón físico 'B': Retorno al estado anterior o Dashboard
    if (kDown & KEY_B) {
        if (app->currentState == STATE_TIMEDOOR_ACTIVE) {
            app->currentState = STATE_TIMEDOOR_SELECT;
            strncpy(app->statusMessage, "TIMEDOOR CERRADA // SELECT TIME", sizeof(app->statusMessage) - 1);
            return;
        } else if (app->currentState == STATE_TIMEDOOR_SELECT) {
            app->currentState = STATE_DASHBOARD;
            strncpy(app->statusMessage, "DASHBOARD // MENU PRINCIPAL", sizeof(app->statusMessage) - 1);
            return;
        }
    }

    // 2. Botón físico 'A': Confirmar o avanzar estado
    if (kDown & KEY_A) {
        if (app->currentState == STATE_DASHBOARD) {
            if (app->selectedDashApp == DASH_APP_TIMEDOOR) {
                app->currentState = STATE_TIMEDOOR_SELECT;
                strncpy(app->statusMessage, "SUBMENU: SAVED TIMES ACTIVADO", sizeof(app->statusMessage) - 1);
                return;
            } else if (app->selectedDashApp == DASH_APP_MISS_MINUTES) {
                strncpy(app->statusMessage, "MISS MINUTES: 'HEY Y'ALL! FOR ALL TIME.'", sizeof(app->statusMessage) - 1);
            } else if (app->selectedDashApp == DASH_APP_SETTINGS) {
                strncpy(app->statusMessage, "AJUSTES: TEMPAD OS v2.4 NOMINAL", sizeof(app->statusMessage) - 1);
            } else if (app->selectedDashApp == DASH_APP_DIRECTORY) {
                strncpy(app->statusMessage, "DIRECTORIO: ARCHIVOS CLASIFICADOS TVA", sizeof(app->statusMessage) - 1);
            }
        } else if (app->currentState == STATE_TIMEDOOR_SELECT) {
            app->currentState = STATE_TIMEDOOR_ACTIVE;
            strncpy(app->statusMessage, "TIMEDOOR DESPLEGADA // PORTAL ACTIVO", sizeof(app->statusMessage) - 1);
            return;
        } else if (app->currentState == STATE_TIMEDOOR_ACTIVE) {
            app->currentState = STATE_DASHBOARD;
            strncpy(app->statusMessage, "TIMEDOOR CERRADA // RETORNO A BASE", sizeof(app->statusMessage) - 1);
            return;
        }
    }

    // 3. D-Pad para navegación rápida
    if (kDown & KEY_UP) {
        if (app->currentState == STATE_TIMEDOOR_SELECT) {
            if (app->selectedDestination > 0) app->selectedDestination--;
        } else if (app->currentState == STATE_DASHBOARD) {
            if (app->selectedDashApp >= 2) app->selectedDashApp -= 2;
        }
    }
    if (kDown & KEY_DOWN) {
        if (app->currentState == STATE_TIMEDOOR_SELECT) {
            if (app->selectedDestination < DESTINATION_COUNT - 1) app->selectedDestination++;
        } else if (app->currentState == STATE_DASHBOARD) {
            if (app->selectedDashApp < 2) app->selectedDashApp += 2;
        }
    }
    if (kDown & KEY_LEFT) {
        if (app->currentState == STATE_DASHBOARD) {
            if (app->selectedDashApp % 2 == 1) app->selectedDashApp--;
        }
    }
    if (kDown & KEY_RIGHT) {
        if (app->currentState == STATE_DASHBOARD) {
            if (app->selectedDashApp % 2 == 0) app->selectedDashApp++;
        }
    }

    // 4. Manejo Táctil Continuo (Feedback visual en tiempo real)
    app->pressedDashTile = -1;
    app->pressedDashAction = -1;
    app->pressedDestCell = -1;
    app->pressedScrollBtn = 0;
    app->pressedBackButton = false;

    touchPosition touch;
    if (kHeld & KEY_TOUCH) {
        hidTouchRead(&touch);

        if (app->currentState == STATE_DASHBOARD) {
            // Comprobar matriz 2x2
            for (int i = 0; i < DASH_APP_COUNT; i++) {
                const DashboardTileDef* t = &app->dashTiles[i];
                if (touch.px >= t->x && touch.px <= (t->x + t->w) &&
                    touch.py >= t->y && touch.py <= (t->y + t->h)) {
                    app->pressedDashTile = i;
                    break;
                }
            }
            // Comprobar Action List
            for (int i = 0; i < DASH_ACT_COUNT; i++) {
                const DashboardActionDef* a = &app->dashActions[i];
                if (touch.px >= a->x && touch.px <= (a->x + a->w) &&
                    touch.py >= a->y && touch.py <= (a->y + a->h)) {
                    app->pressedDashAction = i;
                    break;
                }
            }
        } else if (app->currentState == STATE_TIMEDOOR_SELECT) {
            // Botón '< VOLVER'
            if (touch.px >= 8 && touch.px <= 75 && touch.py >= 6 && touch.py <= 26) {
                app->pressedBackButton = true;
            }

            // Celdas de destinos (columna derecha)
            for (int i = 0; i < DESTINATION_COUNT; i++) {
                float cellY = 50.0f + i * 40.0f;
                if (touch.px >= 152 && touch.px <= 290 && touch.py >= cellY && touch.py <= (cellY + 36.0f)) {
                    app->pressedDestCell = i;
                    break;
                }
            }

            // Flechas de scrollbar
            if (touch.px >= 294 && touch.px <= 310) {
                if (touch.py >= 50 && touch.py <= 66) app->pressedScrollBtn = 1; // Arriba
                else if (touch.py >= 190 && touch.py <= 206) app->pressedScrollBtn = 2; // Abajo
            }

            // Botón [ ABRIR TIMEDOOR ] (columna izquierda abajo)
            if (touch.px >= 10 && touch.px <= 140 && touch.py >= 190 && touch.py <= 226) {
                app->pressedBackButton = false; // Se reutiliza como área táctil de abrir
            }
        } else if (app->currentState == STATE_TIMEDOOR_ACTIVE) {
            if (touch.px >= 8 && touch.px <= 140 && touch.py >= 6 && touch.py <= 28) {
                app->pressedBackButton = true;
            }
        }
    }

    // 5. Manejo Táctil de Disparo de Acción (Frame del toque kDown)
    if (kDown & KEY_TOUCH) {
        hidTouchRead(&touch);

        if (app->currentState == STATE_DASHBOARD) {
            // Toque en matriz 2x2
            for (int i = 0; i < DASH_APP_COUNT; i++) {
                const DashboardTileDef* t = &app->dashTiles[i];
                if (touch.px >= t->x && touch.px <= (t->x + t->w) &&
                    touch.py >= t->y && touch.py <= (t->y + t->h)) {

                    // Doble toque en TIMEDOOR abre directamente el submenú
                    if (i == DASH_APP_TIMEDOOR && app->lastTappedApp == DASH_APP_TIMEDOOR && app->tapTimer < 25) {
                        app->currentState = STATE_TIMEDOOR_SELECT;
                        strncpy(app->statusMessage, "DOBLE TOQUE: SELECT TIME ABIERTO", sizeof(app->statusMessage) - 1);
                        app->lastTappedApp = -1;
                        return;
                    }

                    app->selectedDashApp = i;
                    app->lastTappedApp = i;
                    app->tapTimer = 0;

                    if (i == DASH_APP_TIMEDOOR) {
                        strncpy(app->statusMessage, "TIMEDOOR LISTA // TOCA 'RUN PROGRAM'", sizeof(app->statusMessage) - 1);
                    } else if (i == DASH_APP_MISS_MINUTES) {
                        strncpy(app->statusMessage, "MISS MINUTES: 'BIENVENIDO AL TEMPAD!'", sizeof(app->statusMessage) - 1);
                    } else if (i == DASH_APP_SETTINGS) {
                        strncpy(app->statusMessage, "AJUSTES: CONFIGURACION DE INTERFAZ", sizeof(app->statusMessage) - 1);
                    } else if (i == DASH_APP_DIRECTORY) {
                        strncpy(app->statusMessage, "DIRECTORIO: INDICE DE VARIANTES TVA", sizeof(app->statusMessage) - 1);
                    }
                    break;
                }
            }

            // Toque en Action List
            for (int i = 0; i < DASH_ACT_COUNT; i++) {
                const DashboardActionDef* a = &app->dashActions[i];
                if (touch.px >= a->x && touch.px <= (a->x + a->w) &&
                    touch.py >= a->y && touch.py <= (a->y + a->h)) {

                    if (i == DASH_ACT_RUN) {
                        if (app->selectedDashApp == DASH_APP_TIMEDOOR) {
                            app->currentState = STATE_TIMEDOOR_SELECT;
                            strncpy(app->statusMessage, "SUBMENU: SAVED TIMES ACTIVADO", sizeof(app->statusMessage) - 1);
                        } else {
                            strncpy(app->statusMessage, "PROGRAMA EJECUTADO CON EXITO", sizeof(app->statusMessage) - 1);
                        }
                    } else if (i == DASH_ACT_STATUS) {
                        // Genera una ramificación temporal (Nexus Event) en la pantalla superior
                        if (!app->branches[0].active) {
                            app->branches[0].active = true;
                            app->branches[0].startX = 125.0f + (rand() % 35);
                            app->branches[0].targetDeviation = 48.0f;
                            strncpy(app->statusMessage, "ALERTA: RAMIFICACION DETECTADA", sizeof(app->statusMessage) - 1);
                        } else {
                            app->branches[1].active = true;
                            app->branches[1].startX = 175.0f + (rand() % 30);
                            app->branches[1].targetDeviation = 55.0f;
                            app->branches[0].targetDeviation = 64.0f;
                            strncpy(app->statusMessage, "PELIGRO: EVENTO NEXUS CRITICO", sizeof(app->statusMessage) - 1);
                        }
                    } else if (i == DASH_ACT_PURGE) {
                        // Carga de reinicio TVA
                        app->branches[0].active = false;
                        app->branches[0].deviation = 0.0f;
                        app->branches[0].targetDeviation = 0.0f;
                        app->branches[1].active = false;
                        app->branches[1].deviation = 0.0f;
                        app->branches[1].targetDeviation = 0.0f;
                        app->purgeFlashFrames = 16;
                        strncpy(app->statusMessage, "PURGA: CARGA DE REINICIO APLICADA", sizeof(app->statusMessage) - 1);
                    }
                    break;
                }
            }
        } else if (app->currentState == STATE_TIMEDOOR_SELECT) {
            // Botón de retorno '< VOLVER'
            if (touch.px >= 8 && touch.px <= 75 && touch.py >= 6 && touch.py <= 26) {
                app->currentState = STATE_DASHBOARD;
                strncpy(app->statusMessage, "DASHBOARD // MENU PRINCIPAL", sizeof(app->statusMessage) - 1);
                return;
            }

            // Selección de celdas en la lista de destinos
            for (int i = 0; i < DESTINATION_COUNT; i++) {
                float cellY = 50.0f + i * 40.0f;
                if (touch.px >= 152 && touch.px <= 290 && touch.py >= cellY && touch.py <= (cellY + 36.0f)) {

                    // Si se toca dos veces el mismo destino activo, activa la Timedoor
                    if (app->selectedDestination == i && app->lastTappedDest == i && app->tapTimer < 25) {
                        app->currentState = STATE_TIMEDOOR_ACTIVE;
                        strncpy(app->statusMessage, "TIMEDOOR DESPLEGADA // PORTAL ACTIVO", sizeof(app->statusMessage) - 1);
                        app->lastTappedDest = -1;
                        return;
                    }

                    app->selectedDestination = i;
                    app->lastTappedDest = i;
                    app->tapTimer = 0;
                    break;
                }
            }

            // Scrollbar táctil
            if (touch.px >= 294 && touch.px <= 310) {
                if (touch.py >= 50 && touch.py <= 66) {
                    if (app->selectedDestination > 0) app->selectedDestination--;
                } else if (touch.py >= 190 && touch.py <= 206) {
                    if (app->selectedDestination < DESTINATION_COUNT - 1) app->selectedDestination++;
                }
            }

            // Botón [ ABRIR TIMEDOOR ] (columna izquierda)
            if (touch.px >= 10 && touch.px <= 140 && touch.py >= 190 && touch.py <= 226) {
                app->currentState = STATE_TIMEDOOR_ACTIVE;
                strncpy(app->statusMessage, "TIMEDOOR DESPLEGADA // PORTAL ACTIVO", sizeof(app->statusMessage) - 1);
                return;
            }
        } else if (app->currentState == STATE_TIMEDOOR_ACTIVE) {
            // Tocar botón de cerrar o la pantalla para volver
            if (touch.px >= 8 && touch.px <= 140 && touch.py >= 6 && touch.py <= 28) {
                app->currentState = STATE_DASHBOARD;
                strncpy(app->statusMessage, "DASHBOARD // MENU PRINCIPAL", sizeof(app->statusMessage) - 1);
                return;
            }
        }
    }
}

// ---------------------------------------------------------
// Actualización de la Dinámica Temporal y Animaciones
// ---------------------------------------------------------
static void updateSimulation(TemPadApp* app) {
    app->timelinePhase += 0.045f;
    if (app->tapTimer < 100) app->tapTimer++;

    // Interpolación suave (lerp) de las ramificaciones Nexus
    for (int i = 0; i < 2; i++) {
        NexusBranch* br = &app->branches[i];
        if (br->active || br->deviation > 0.1f) {
            float delta = br->targetDeviation - br->deviation;
            br->deviation += delta * 0.09f;

            if (br->targetDeviation <= 0.01f && br->deviation < 0.4f) {
                br->deviation = 0.0f;
                br->active = false;
            }
        }
    }

    if (app->purgeFlashFrames > 0) {
        app->purgeFlashFrames--;
    }
}

// ---------------------------------------------------------
// Renderizado: Pantalla Superior (Sagrada Línea Temporal)
// ---------------------------------------------------------
static void renderTopScreen(TemPadApp* app) {
    C2D_TargetClear(app->topScreen, CLR_BG_DARK);
    C2D_SceneBegin(app->topScreen);

    // 1. Rejilla CRT de fondo
    for (float x = 40.0f; x < (float)TOP_WIDTH; x += 40.0f) {
        C2D_DrawLine(x, 26.0f, CLR_GRID, x, 214.0f, CLR_GRID, 1.0f, 0.0f);
    }
    for (float y = 50.0f; y <= 190.0f; y += 35.0f) {
        C2D_DrawLine(10.0f, y, CLR_GRID, 390.0f, y, CLR_GRID, 1.0f, 0.0f);
    }

    // 2. Líneas de Umbral Rojo Crítico
    const float thresholdTop = 52.0f;
    const float thresholdBottom = 188.0f;
    for (float x = 12.0f; x < 388.0f; x += 14.0f) {
        C2D_DrawLine(x, thresholdTop, CLR_RED_GLOW, x + 8.0f, thresholdTop, CLR_RED_GLOW, 1.5f, 0.0f);
        C2D_DrawLine(x, thresholdBottom, CLR_RED_GLOW, x + 8.0f, thresholdBottom, CLR_RED_GLOW, 1.5f, 0.0f);
    }

    // 3. Sagrada Línea Temporal viva
    const float baseY = 120.0f;
    float prevX = 10.0f;
    float prevY = baseY + 2.5f * sinf(prevX * 0.035f + app->timelinePhase)
                         + 1.2f * sinf(prevX * 0.075f - app->timelinePhase * 1.3f);

    for (float x = 14.0f; x <= 390.0f; x += 4.0f) {
        float curY = baseY + 2.5f * sinf(x * 0.035f + app->timelinePhase)
                           + 1.2f * sinf(x * 0.075f - app->timelinePhase * 1.3f);

        C2D_DrawLine(prevX, prevY, CLR_AMBER_GLOW, x, curY, CLR_AMBER_GLOW, 4.5f, 0.0f);
        C2D_DrawLine(prevX, prevY, CLR_AMBER_BRIGHT, x, curY, CLR_AMBER_BRIGHT, 2.0f, 0.0f);
        C2D_DrawLine(prevX, prevY, CLR_AMBER_CORE, x, curY, CLR_AMBER_CORE, 1.0f, 0.0f);

        prevX = x;
        prevY = curY;
    }

    // 4. Ramificaciones Temporales (Nexus Events)
    for (int b = 0; b < 2; b++) {
        NexusBranch* br = &app->branches[b];
        if (br->deviation > 0.1f) {
            float bPrevX = br->startX;
            float bPrevY = baseY + 2.5f * sinf(bPrevX * 0.035f + app->timelinePhase);

            u32 branchColor = (br->deviation > 54.0f) ? CLR_CRITICAL_RED : CLR_AMBER_BRIGHT;
            u32 branchGlow  = (br->deviation > 54.0f) ? CLR_RED_GLOW      : CLR_AMBER_GLOW;

            for (float bx = br->startX + 4.0f; bx <= 390.0f; bx += 4.0f) {
                float progress = (bx - br->startX) / (390.0f - br->startX);
                float curve = progress * progress * (3.0f - 2.0f * progress);
                float wave = 2.0f * sinf(bx * 0.08f + app->timelinePhase * 1.5f);

                float bCurY = baseY - (br->deviation * curve * br->direction) + wave;

                C2D_DrawLine(bPrevX, bPrevY, branchGlow, bx, bCurY, branchGlow, 3.5f, 0.0f);
                C2D_DrawLine(bPrevX, bPrevY, branchColor, bx, bCurY, branchColor, 1.8f, 0.0f);

                bPrevX = bx;
                bPrevY = bCurY;
            }
        }
    }

    // 5. Representación visual de Timedoor activa en la línea de tiempo
    if (app->currentState == STATE_TIMEDOOR_ACTIVE) {
        float breachX = 200.0f;
        float pulse = 4.0f * sinf(app->timelinePhase * 4.0f);

        // Haz vertical de portal
        C2D_DrawLine(breachX, 60.0f, CLR_AMBER_GLOW, breachX, 180.0f, CLR_AMBER_GLOW, 5.0f, 0.0f);
        C2D_DrawLine(breachX, 60.0f, CLR_AMBER_CORE, breachX, 180.0f, CLR_AMBER_CORE, 2.0f, 0.0f);

        // Anillos de expansión temporal
        float ringRadius = 14.0f + pulse;
        drawBoxOutline(breachX - ringRadius, 120.0f - ringRadius, ringRadius * 2.0f, ringRadius * 2.0f, 1.5f, CLR_AMBER_BRIGHT);
    }

    // 6. Destello de purga
    if (app->purgeFlashFrames > 0) {
        float alpha = ((float)app->purgeFlashFrames / 16.0f) * 170.0f;
        u32 flashClr = C2D_Color32(210, 120, 255, (u8)alpha);
        C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, TOP_WIDTH, TOP_HEIGHT, flashClr);
    }

    // 7. Textos y Telemetría Superior
    C2D_TextBufClear(app->textBufDynamic);

    C2D_Text txtTopHeader;
    C2D_TextParse(&txtTopHeader, app->textBufDynamic, "TVA // SAGRADA LINEA TEMPORAL");
    C2D_TextOptimize(&txtTopHeader);
    C2D_DrawText(&txtTopHeader, C2D_WithColor, 12.0f, 8.0f, 0.5f, 0.44f, 0.44f, CLR_AMBER_TEXT);

    // Muestra del estado actual del TemPad en la esquina superior derecha
    const char* modeText = "[MODE: DASHBOARD]";
    if (app->currentState == STATE_TIMEDOOR_SELECT) {
        modeText = "[MODE: TIMEDOOR SELECT]";
    } else if (app->currentState == STATE_TIMEDOOR_ACTIVE) {
        modeText = "[MODE: TIMEDOOR ACTIVE]";
    }

    C2D_Text txtMode;
    C2D_TextParse(&txtMode, app->textBufDynamic, modeText);
    C2D_TextOptimize(&txtMode);
    C2D_DrawText(&txtMode, C2D_WithColor | C2D_AlignRight, 388.0f, 8.0f, 0.5f, 0.40f, 0.40f, CLR_AMBER_BRIGHT);

    // Telemetría inferior de la pantalla superior
    char telemetria[64];
    if (app->currentState == STATE_TIMEDOOR_ACTIVE) {
        snprintf(telemetria, sizeof(telemetria), "PORTAL TEMPORAL VINCULADO: %s", g_destinations[app->selectedDestination].name);
    } else {
        float maxDev = fmaxf(app->branches[0].deviation, app->branches[1].deviation);
        if (maxDev > 54.0f) {
            snprintf(telemetria, sizeof(telemetria), "ALERTA: DIVERGENCIA CRITICA (+%.1f%%)", (maxDev / 66.0f) * 100.0f);
        } else if (maxDev > 1.0f) {
            snprintf(telemetria, sizeof(telemetria), "MONITOR: RAMIFICACION EN PROCESO (+%.1f%%)", (maxDev / 66.0f) * 100.0f);
        } else {
            snprintf(telemetria, sizeof(telemetria), "CONTINUO TEMPORAL ESTABLE [1.00x]");
        }
    }

    C2D_Text txtDyn;
    C2D_TextParse(&txtDyn, app->textBufDynamic, telemetria);
    C2D_TextOptimize(&txtDyn);

    u32 statusClr = (app->currentState == STATE_TIMEDOOR_ACTIVE) ? CLR_AMBER_CORE : CLR_AMBER_TEXT;
    C2D_DrawText(&txtDyn, C2D_WithColor, 12.0f, 218.0f, 0.5f, 0.40f, 0.40f, statusClr);
}

// ---------------------------------------------------------
// Renderizado: Pantalla Inferior - Estado 1: DASHBOARD
// ---------------------------------------------------------
static void renderDashboardView(TemPadApp* app) {
    // 1. Barra superior de título
    C2D_Text txtTitle;
    C2D_TextParse(&txtTitle, app->textBufDynamic, "TEMPAD // TVA MAIN DASHBOARD");
    C2D_TextOptimize(&txtTitle);
    C2D_DrawText(&txtTitle, C2D_WithColor, 12.0f, 10.0f, 0.5f, 0.44f, 0.44f, CLR_AMBER_BRIGHT);

    // Telemetría rápida
    C2D_Text txtQuick;
    C2D_TextParse(&txtQuick, app->textBufDynamic, app->statusMessage);
    C2D_TextOptimize(&txtQuick);
    C2D_DrawText(&txtQuick, C2D_WithColor, 12.0f, 26.0f, 0.5f, 0.35f, 0.35f, CLR_AMBER_DARK);

    // 2. Matriz 2x2 (TIMEDOOR, MISS MINUTES, SETTINGS, DIRECTORY)
    for (int i = 0; i < DASH_APP_COUNT; i++) {
        const DashboardTileDef* t = &app->dashTiles[i];
        bool isSelected = (app->selectedDashApp == i);
        bool isPressed  = (app->pressedDashTile == i);

        float drawX = t->x;
        float drawY = isPressed ? (t->y + 2.0f) : t->y;

        u32 bgCol     = isSelected ? (isPressed ? CLR_AMBER_CORE : CLR_AMBER_BRIGHT) : CLR_BTN_NORMAL;
        u32 borderCol = isSelected ? CLR_AMBER_CORE : (isPressed ? CLR_AMBER_BRIGHT : CLR_BORDER_MID);
        u32 textCol   = isSelected ? CLR_TEXT_INVERTED : CLR_AMBER_TEXT;
        u32 subCol    = isSelected ? C2D_Color32(40, 20, 5, 255) : CLR_AMBER_DARK;

        C2D_DrawRectSolid(drawX, drawY, 0.0f, t->w, t->h, bgCol);
        drawBoxOutline(drawX, drawY, t->w, t->h, 1.5f, borderCol);

        // Icono / glifo estilizado retro
        float iconBoxX = drawX + 8.0f;
        float iconBoxY = drawY + 8.0f;
        u32 iconCol = isSelected ? CLR_TEXT_INVERTED : CLR_AMBER_BRIGHT;

        if (i == DASH_APP_TIMEDOOR) {
            // Marco de puerta Timedoor
            drawBoxOutline(iconBoxX, iconBoxY, 18.0f, 22.0f, 1.5f, iconCol);
            C2D_DrawLine(iconBoxX + 9.0f, iconBoxY, iconCol, iconBoxX + 9.0f, iconBoxY + 22.0f, iconCol, 1.0f, 0.0f);
        } else if (i == DASH_APP_MISS_MINUTES) {
            // Reloj circular estilizado
            drawBoxOutline(iconBoxX, iconBoxY + 2.0f, 18.0f, 18.0f, 1.5f, iconCol);
            C2D_DrawLine(iconBoxX + 9.0f, iconBoxY + 5.0f, iconCol, iconBoxX + 9.0f, iconBoxY + 11.0f, iconCol, 1.5f, 0.0f);
            C2D_DrawLine(iconBoxX + 9.0f, iconBoxY + 11.0f, iconCol, iconBoxX + 14.0f, iconBoxY + 11.0f, iconCol, 1.5f, 0.0f);
        } else if (i == DASH_APP_SETTINGS) {
            // Barras deslizantes / sliders
            drawBoxOutline(iconBoxX, iconBoxY + 2.0f, 18.0f, 18.0f, 1.0f, iconCol);
            C2D_DrawLine(iconBoxX + 4.0f, iconBoxY + 7.0f, iconCol, iconBoxX + 14.0f, iconBoxY + 7.0f, iconCol, 1.5f, 0.0f);
            C2D_DrawLine(iconBoxX + 4.0f, iconBoxY + 13.0f, iconCol, iconBoxX + 14.0f, iconBoxY + 13.0f, iconCol, 1.5f, 0.0f);
        } else {
            // Directorio / ficha de datos
            drawBoxOutline(iconBoxX, iconBoxY + 2.0f, 18.0f, 18.0f, 1.5f, iconCol);
            C2D_DrawLine(iconBoxX + 3.0f, iconBoxY + 6.0f, iconCol, iconBoxX + 15.0f, iconBoxY + 6.0f, iconCol, 1.0f, 0.0f);
            C2D_DrawLine(iconBoxX + 3.0f, iconBoxY + 10.0f, iconCol, iconBoxX + 15.0f, iconBoxY + 10.0f, iconCol, 1.0f, 0.0f);
            C2D_DrawLine(iconBoxX + 3.0f, iconBoxY + 14.0f, iconCol, iconBoxX + 10.0f, iconBoxY + 14.0f, iconCol, 1.0f, 0.0f);
        }

        // Título del módulo
        C2D_Text txtTile;
        C2D_TextParse(&txtTile, app->textBufDynamic, t->title);
        C2D_TextOptimize(&txtTile);
        C2D_DrawText(&txtTile, C2D_WithColor, drawX + 8.0f, drawY + 36.0f, 0.5f, 0.40f, 0.40f, textCol);

        // Código de subtítulo
        C2D_Text txtSub;
        C2D_TextParse(&txtSub, app->textBufDynamic, t->code);
        C2D_TextOptimize(&txtSub);
        C2D_DrawText(&txtSub, C2D_WithColor, drawX + 8.0f, drawY + 54.0f, 0.5f, 0.32f, 0.32f, subCol);
    }

    // 3. Action List (Columna Derecha)
    C2D_Text txtActHeader;
    C2D_TextParse(&txtActHeader, app->textBufDynamic, "ACTIONS //////");
    C2D_TextOptimize(&txtActHeader);
    C2D_DrawText(&txtActHeader, C2D_WithColor, 214.0f, 46.0f, 0.5f, 0.38f, 0.38f, CLR_AMBER_BRIGHT);

    for (int i = 0; i < DASH_ACT_COUNT; i++) {
        const DashboardActionDef* a = &app->dashActions[i];
        bool isPressed = (app->pressedDashAction == i);

        float drawX = a->x;
        float drawY = isPressed ? (a->y + 2.0f) : a->y;

        u32 btnBg     = isPressed ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
        u32 btnBorder = isPressed ? CLR_AMBER_CORE   : CLR_BORDER_MID;
        u32 btnText   = isPressed ? CLR_TEXT_INVERTED: CLR_AMBER_TEXT;

        if (i == DASH_ACT_PURGE) {
            btnBg     = isPressed ? CLR_PURGE_BTN_PRESSED : CLR_PURGE_BTN_NORMAL;
            btnBorder = isPressed ? CLR_AMBER_CORE        : CLR_PURGE_BTN_BORDER;
            btnText   = isPressed ? CLR_TEXT_INVERTED     : C2D_Color32(255, 120, 90, 255);
        }

        C2D_DrawRectSolid(drawX, drawY, 0.0f, a->w, a->h, btnBg);
        drawBoxOutline(drawX, drawY, a->w, a->h, 1.5f, btnBorder);

        C2D_Text txtLabel;
        C2D_TextParse(&txtLabel, app->textBufDynamic, a->label);
        C2D_TextOptimize(&txtLabel);
        C2D_DrawText(&txtLabel, C2D_WithColor | C2D_AlignCenter,
                     drawX + (a->w / 2.0f), drawY + 11.0f, 0.5f, 0.40f, 0.40f, btnText);
    }

    // 4. Pie de pantalla con instrucciones
    C2D_Text txtFooter;
    C2D_TextParse(&txtFooter, app->textBufDynamic, "TOCA TIMEDOOR O 'RUN PROGRAM' // [START] SALIR");
    C2D_TextOptimize(&txtFooter);
    C2D_DrawText(&txtFooter, C2D_WithColor | C2D_AlignCenter, 160.0f, 216.0f, 0.5f, 0.36f, 0.36f, CLR_AMBER_DARK);
}

// ---------------------------------------------------------
// Renderizado: Pantalla Inferior - Estado 2: SELECT TIME
// ---------------------------------------------------------
static void renderTimedoorSelectView(TemPadApp* app) {
    // 1. Barra superior con botón de retorno
    float backW = 68.0f, backH = 20.0f;
    u32 backBg = app->pressedBackButton ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
    u32 backTxt = app->pressedBackButton ? CLR_TEXT_INVERTED : CLR_AMBER_TEXT;

    C2D_DrawRectSolid(8.0f, 6.0f, 0.0f, backW, backH, backBg);
    drawBoxOutline(8.0f, 6.0f, backW, backH, 1.0f, CLR_BORDER_MID);

    C2D_Text txtBack;
    C2D_TextParse(&txtBack, app->textBufDynamic, "< VOLVER");
    C2D_TextOptimize(&txtBack);
    C2D_DrawText(&txtBack, C2D_WithColor | C2D_AlignCenter, 8.0f + (backW / 2.0f), 9.0f, 0.5f, 0.38f, 0.38f, backTxt);

    C2D_Text txtSubHeader;
    C2D_TextParse(&txtSubHeader, app->textBufDynamic, "TVA // TEMPORAL COORDINATES");
    C2D_TextOptimize(&txtSubHeader);
    C2D_DrawText(&txtSubHeader, C2D_WithColor | C2D_AlignRight, 312.0f, 9.0f, 0.5f, 0.38f, 0.38f, CLR_AMBER_DARK);

    // -----------------------------------------------------
    // COLUMNA IZQUIERDA (Preview de Destino)
    // -----------------------------------------------------
    C2D_Text txtLeftTitle;
    C2D_TextParse(&txtLeftTitle, app->textBufDynamic, "SELECT TIME");
    C2D_TextOptimize(&txtLeftTitle);
    C2D_DrawText(&txtLeftTitle, C2D_WithColor, 10.0f, 32.0f, 0.5f, 0.44f, 0.44f, CLR_AMBER_BRIGHT);

    // Placeholder para visor de cámara / thumbnail (130x65 px exactos)
    const float viewX = 10.0f, viewY = 48.0f, viewW = 130.0f, viewH = 65.0f;
    C2D_DrawRectSolid(viewX, viewY, 0.0f, viewW, viewH, C2D_Color32(28, 18, 8, 180));
    drawBoxOutline(viewX, viewY, viewW, viewH, 1.5f, CLR_AMBER_BRIGHT);

    // Retícula y detalles fósforo CRT dentro del visor
    float vMidX = viewX + viewW / 2.0f;
    float vMidY = viewY + viewH / 2.0f;
    C2D_DrawLine(vMidX - 12.0f, vMidY, CLR_AMBER_DARK, vMidX - 3.0f, vMidY, CLR_AMBER_DARK, 1.0f, 0.0f);
    C2D_DrawLine(vMidX + 3.0f, vMidY, CLR_AMBER_DARK, vMidX + 12.0f, vMidY, CLR_AMBER_DARK, 1.0f, 0.0f);
    C2D_DrawLine(vMidX, vMidY - 10.0f, CLR_AMBER_DARK, vMidX, vMidY - 3.0f, CLR_AMBER_DARK, 1.0f, 0.0f);
    C2D_DrawLine(vMidX, vMidY + 3.0f, CLR_AMBER_DARK, vMidX, vMidY + 10.0f, CLR_AMBER_DARK, 1.0f, 0.0f);

    // Mini scanline animada del visor
    float scanlineY = viewY + fmodf(app->timelinePhase * 25.0f, viewH);
    C2D_DrawLine(viewX + 2.0f, scanlineY, C2D_Color32(255, 180, 40, 70), viewX + viewW - 2.0f, scanlineY, C2D_Color32(255, 180, 40, 70), 1.0f, 0.0f);

    C2D_Text txtCamFeed;
    C2D_TextParse(&txtCamFeed, app->textBufDynamic, "[ ARCHIVE FEED ]");
    C2D_TextOptimize(&txtCamFeed);
    C2D_DrawText(&txtCamFeed, C2D_WithColor, viewX + 6.0f, viewY + 5.0f, 0.5f, 0.30f, 0.30f, CLR_AMBER_TEXT);

    // Nombre del destino seleccionado en grande
    const DestinationDef* curDest = &g_destinations[app->selectedDestination];

    C2D_Text txtDestName;
    C2D_TextParse(&txtDestName, app->textBufDynamic, curDest->name);
    C2D_TextOptimize(&txtDestName);
    C2D_DrawText(&txtDestName, C2D_WithColor, 10.0f, 120.0f, 0.5f, 0.55f, 0.55f, CLR_AMBER_BRIGHT);

    // Código de coordenadas
    C2D_Text txtCoordsLabel;
    C2D_TextParse(&txtCoordsLabel, app->textBufDynamic, "COORDENADAS:");
    C2D_TextOptimize(&txtCoordsLabel);
    C2D_DrawText(&txtCoordsLabel, C2D_WithColor, 10.0f, 142.0f, 0.5f, 0.32f, 0.32f, CLR_AMBER_DARK);

    C2D_Text txtCoords;
    C2D_TextParse(&txtCoords, app->textBufDynamic, curDest->coords);
    C2D_TextOptimize(&txtCoords);
    C2D_DrawText(&txtCoords, C2D_WithColor, 10.0f, 155.0f, 0.5f, 0.38f, 0.38f, CLR_AMBER_TEXT);

    C2D_Text txtEra;
    C2D_TextParse(&txtEra, app->textBufDynamic, curDest->era);
    C2D_TextOptimize(&txtEra);
    C2D_DrawText(&txtEra, C2D_WithColor, 10.0f, 172.0f, 0.5f, 0.32f, 0.32f, CLR_AMBER_DARK);

    // Botón para desplegar Timedoor
    C2D_DrawRectSolid(10.0f, 192.0f, 0.0f, 130.0f, 36.0f, CLR_BTN_NORMAL);
    drawBoxOutline(10.0f, 192.0f, 130.0f, 36.0f, 1.5f, CLR_AMBER_BRIGHT);

    C2D_Text txtDeploy;
    C2D_TextParse(&txtDeploy, app->textBufDynamic, "[ ABRIR TIMEDOOR ]");
    C2D_TextOptimize(&txtDeploy);
    C2D_DrawText(&txtDeploy, C2D_WithColor | C2D_AlignCenter, 75.0f, 202.0f, 0.5f, 0.40f, 0.40f, CLR_AMBER_CORE);

    // -----------------------------------------------------
    // COLUMNA DERECHA (Lista de destinos "SAVED TIMES")
    // -----------------------------------------------------
    C2D_Text txtListHeader;
    C2D_TextParse(&txtListHeader, app->textBufDynamic, "SAVED TIMES //////");
    C2D_TextOptimize(&txtListHeader);
    C2D_DrawText(&txtListHeader, C2D_WithColor, 152.0f, 32.0f, 0.5f, 0.44f, 0.44f, CLR_AMBER_BRIGHT);

    const float listX = 152.0f;
    const float cellW = 138.0f;
    const float cellH = 36.0f;

    for (int i = 0; i < DESTINATION_COUNT; i++) {
        float cellY = 50.0f + i * 40.0f;
        bool isSelected = (app->selectedDestination == i);
        bool isPressed  = (app->pressedDestCell == i);

        u32 cellBg  = isSelected ? (isPressed ? CLR_AMBER_CORE : CLR_AMBER_BRIGHT) : CLR_BTN_NORMAL;
        u32 borderCol = isSelected ? CLR_AMBER_CORE : CLR_BORDER_MID;
        u32 textCol = isSelected ? CLR_TEXT_INVERTED : CLR_AMBER_TEXT;

        C2D_DrawRectSolid(listX, cellY, 0.0f, cellW, cellH, cellBg);
        drawBoxOutline(listX, cellY, cellW, cellH, 1.5f, borderCol);

        // Indicador '>' para el elemento activo
        if (isSelected) {
            C2D_Text txtMarker;
            C2D_TextParse(&txtMarker, app->textBufDynamic, ">");
            C2D_TextOptimize(&txtMarker);
            C2D_DrawText(&txtMarker, C2D_WithColor, listX + 6.0f, cellY + 9.0f, 0.5f, 0.45f, 0.45f, textCol);
        }

        C2D_Text txtItem;
        C2D_TextParse(&txtItem, app->textBufDynamic, g_destinations[i].name);
        C2D_TextOptimize(&txtItem);
        C2D_DrawText(&txtItem, C2D_WithColor, listX + (isSelected ? 20.0f : 10.0f), cellY + 9.0f, 0.5f, 0.42f, 0.42f, textCol);
    }

    // Barra lateral derecha que simula el scrollbar
    const float sbX = 296.0f, sbY = 50.0f, sbW = 16.0f, sbH = 156.0f;
    C2D_DrawRectSolid(sbX, sbY, 0.0f, sbW, sbH, CLR_PANEL_INSET);
    drawBoxOutline(sbX, sbY, sbW, sbH, 1.0f, CLR_BORDER_MID);

    // Flecha Arriba
    u32 upBg = (app->pressedScrollBtn == 1) ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
    C2D_DrawRectSolid(sbX, sbY, 0.0f, sbW, 16.0f, upBg);
    drawBoxOutline(sbX, sbY, sbW, 16.0f, 1.0f, CLR_BORDER_MID);
    C2D_Text txtUp;
    C2D_TextParse(&txtUp, app->textBufDynamic, "^");
    C2D_TextOptimize(&txtUp);
    C2D_DrawText(&txtUp, C2D_WithColor | C2D_AlignCenter, sbX + 8.0f, sbY + 1.0f, 0.5f, 0.40f, 0.40f, CLR_AMBER_TEXT);

    // Flecha Abajo
    u32 downBg = (app->pressedScrollBtn == 2) ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
    C2D_DrawRectSolid(sbX, sbY + sbH - 16.0f, 0.0f, sbW, 16.0f, downBg);
    drawBoxOutline(sbX, sbY + sbH - 16.0f, 1.0f, 16.0f, 1.0f, CLR_BORDER_MID);
    C2D_Text txtDown;
    C2D_TextParse(&txtDown, app->textBufDynamic, "v");
    C2D_TextOptimize(&txtDown);
    C2D_DrawText(&txtDown, C2D_WithColor | C2D_AlignCenter, sbX + 8.0f, sbY + sbH - 15.0f, 0.5f, 0.40f, 0.40f, CLR_AMBER_TEXT);

    // Thumb deslizador según destino seleccionado
    float thumbY = sbY + 18.0f + (float)app->selectedDestination * ((sbH - 60.0f) / (DESTINATION_COUNT - 1));
    C2D_DrawRectSolid(sbX + 2.0f, thumbY, 0.0f, sbW - 4.0f, 24.0f, CLR_AMBER_BRIGHT);
}

// ---------------------------------------------------------
// Renderizado: Pantalla Inferior - Estado 3: TIMEDOOR ACTIVE
// ---------------------------------------------------------
static void renderTimedoorActiveView(TemPadApp* app) {
    // 1. Botón de retorno rápido
    float backW = 120.0f, backH = 22.0f;
    u32 backBg = app->pressedBackButton ? CLR_AMBER_BRIGHT : CLR_BTN_NORMAL;
    u32 backTxt = app->pressedBackButton ? CLR_TEXT_INVERTED : CLR_AMBER_TEXT;

    C2D_DrawRectSolid(8.0f, 6.0f, 0.0f, backW, backH, backBg);
    drawBoxOutline(8.0f, 6.0f, backW, backH, 1.0f, CLR_BORDER_MID);

    C2D_Text txtBack;
    C2D_TextParse(&txtBack, app->textBufDynamic, "< [B] CERRAR TIMEDOOR");
    C2D_TextOptimize(&txtBack);
    C2D_DrawText(&txtBack, C2D_WithColor | C2D_AlignCenter, 8.0f + (backW / 2.0f), 9.0f, 0.5f, 0.38f, 0.38f, backTxt);

    // 2. Gráfico retro del portal Timedoor abierto
    float portalX = 160.0f, portalY = 115.0f;
    float pW = 110.0f, pH = 130.0f;

    // Resplandor de umbral
    float pulse = 3.0f * sinf(app->timelinePhase * 5.0f);
    C2D_DrawRectSolid(portalX - (pW / 2.0f) - 4.0f, portalY - (pH / 2.0f) - 4.0f, 0.0f, pW + 8.0f, pH + 8.0f, CLR_AMBER_GLOW);

    // Umbral de puerta principal
    C2D_DrawRectSolid(portalX - (pW / 2.0f), portalY - (pH / 2.0f), 0.0f, pW, pH, C2D_Color32(240, 150, 20, 255));
    drawBoxOutline(portalX - (pW / 2.0f), portalY - (pH / 2.0f), pW, pH, 2.5f, CLR_AMBER_CORE);

    // Núcleo energético del portal
    C2D_DrawRectSolid(portalX - 35.0f - pulse, portalY - 45.0f - pulse, 0.0f, 70.0f + pulse * 2.0f, 90.0f + pulse * 2.0f, CLR_AMBER_CORE);

    // 3. Telemetría de paso seguro
    const DestinationDef* curDest = &g_destinations[app->selectedDestination];

    C2D_Text txtStatus;
    C2D_TextParse(&txtStatus, app->textBufDynamic, "TIMEDOOR ABIERTA // VINCULO ESTABLE");
    C2D_TextOptimize(&txtStatus);
    C2D_DrawText(&txtStatus, C2D_WithColor | C2D_AlignCenter, 160.0f, 36.0f, 0.5f, 0.44f, 0.44f, CLR_AMBER_BRIGHT);

    C2D_Text txtDest;
    C2D_TextParse(&txtDest, app->textBufDynamic, curDest->name);
    C2D_TextOptimize(&txtDest);
    C2D_DrawText(&txtDest, C2D_WithColor | C2D_AlignCenter, 160.0f, 192.0f, 0.5f, 0.52f, 0.52f, CLR_AMBER_CORE);

    C2D_Text txtCoord;
    C2D_TextParse(&txtCoord, app->textBufDynamic, curDest->coords);
    C2D_TextOptimize(&txtCoord);
    C2D_DrawText(&txtCoord, C2D_WithColor | C2D_AlignCenter, 160.0f, 214.0f, 0.5f, 0.38f, 0.38f, CLR_AMBER_DARK);
}

// ---------------------------------------------------------
// Renderizado: Pantalla Inferior (Despachador según Estado)
// ---------------------------------------------------------
static void renderBottomScreen(TemPadApp* app) {
    C2D_TargetClear(app->bottomScreen, CLR_BG_DARK);
    C2D_SceneBegin(app->bottomScreen);

    // Cuadrícula ámbar tenue de fondo retro
    for (float x = 20.0f; x < (float)BOTTOM_WIDTH; x += 30.0f) {
        C2D_DrawLine(x, 0.0f, CLR_GRID, x, (float)BOTTOM_HEIGHT, CLR_GRID, 1.0f, 0.0f);
    }
    for (float y = 20.0f; y < (float)BOTTOM_HEIGHT; y += 30.0f) {
        C2D_DrawLine(0.0f, y, CLR_GRID, (float)BOTTOM_WIDTH, y, CLR_GRID, 1.0f, 0.0f);
    }

    // Marco exterior retro del chasis
    drawBoxOutline(2.0f, 2.0f, BOTTOM_WIDTH - 4.0f, BOTTOM_HEIGHT - 4.0f, 2.0f, CLR_BORDER_DIM);

    // Despacho según el estado actual
    switch (app->currentState) {
        case STATE_DASHBOARD:
            renderDashboardView(app);
            break;
        case STATE_TIMEDOOR_SELECT:
            renderTimedoorSelectView(app);
            break;
        case STATE_TIMEDOOR_ACTIVE:
            renderTimedoorActiveView(app);
            break;
    }
}

// ---------------------------------------------------------
// Punto de Entrada (Main Loop)
// ---------------------------------------------------------
int main(int argc, char* argv[]) {
    TemPadApp app;

    if (!initApp(&app)) {
        freeApp(&app);
        return 1;
    }

    // Bucle sincronizado a 60 FPS
    while (aptMainLoop()) {
        handleInput(&app);

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) {
            break; // Retorno al Homebrew Menu
        }

        updateSimulation(&app);

        // Renderizado sincronizado
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        renderTopScreen(&app);
        renderBottomScreen(&app);

        C3D_FrameEnd(0);
    }

    freeApp(&app);
    return 0;
}
