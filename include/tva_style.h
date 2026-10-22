#ifndef TVA_STYLE_H
#define TVA_STYLE_H

#include <3ds.h>
#include <citro2d.h>

// =============================================================================
// TVA_STYLE.H — CONFIGURACION VISUAL, LAYOUT Y VELOCIDADES (ESTILO "CSS")
// =============================================================================
// Edita este archivo para ajustar colores, grosores de linea, escalas de texto,
// velocidades de animacion y posiciones de la interfaz sin tocar la logica.
// =============================================================================

// -----------------------------------------------------------------------------
// 1. DIMENSIONES DE PANTALLA (NINTENDO 3DS NATIVO)
// -----------------------------------------------------------------------------
#define TOP_WIDTH                       400
#define TOP_HEIGHT                      240
#define BOTTOM_WIDTH                    320
#define BOTTOM_HEIGHT                   240

// -----------------------------------------------------------------------------
// 2. PALETA DE COLORES CANONICA DE LA TVA (RGBA: 0..255)
// -----------------------------------------------------------------------------
// Fondos y Paneles CRT
#define COLOR_BG_DARK                   C2D_Color32(12, 8, 4, 255)
#define COLOR_PANEL_BG                  C2D_Color32(22, 15, 9, 255)
#define COLOR_PANEL_INSET               C2D_Color32(14, 9, 5, 255)
#define COLOR_BORDER_DIM                C2D_Color32(65, 40, 16, 255)
#define COLOR_BORDER_MID                C2D_Color32(135, 82, 28, 255)
#define COLOR_GRID_LINE                 C2D_Color32(42, 26, 10, 150)

// Bloque Placeholder del Logo TVA (Marron / Ambar muy oscuro sin borde brillante)
#define COLOR_TVA_LOGO_BG               C2D_Color32(35, 18, 4, 255)
#define COLOR_TVA_LOGO_TEXT             C2D_Color32(175, 95, 22, 255)

// Sagrada Linea Temporal (Gris / Plateado Apagado)
#define COLOR_SACRED_CORE               C2D_Color32(175, 180, 190, 255)
#define COLOR_SACRED_GLOW               C2D_Color32(100, 105, 115, 180)
#define COLOR_SACRED_MAIN               C2D_Color32(150, 155, 165, 255)

// Ramificaciones y UI Ambar / Naranja TVA
#define COLOR_BRANCH_AMBER              C2D_Color32(255, 190, 40, 255)
#define COLOR_BRANCH_SELECTED           C2D_Color32(255, 245, 190, 255)
#define COLOR_BRANCH_AMBER_GLOW         C2D_Color32(255, 125, 0, 110)
#define COLOR_AMBER_TEXT                C2D_Color32(255, 178, 38, 255)
#define COLOR_AMBER_DIM                 C2D_Color32(130, 72, 18, 255)
#define COLOR_TEXT_INVERTED             C2D_Color32(18, 10, 4, 255)

// Alerta Critica Nexus y Lineas de Umbral Rojas Solidas
#define COLOR_BRANCH_RED                C2D_Color32(250, 48, 24, 255)
#define COLOR_REDLINE_SOLID             C2D_Color32(235, 45, 20, 245)
#define COLOR_BRANCH_RED_GLOW           C2D_Color32(220, 35, 15, 130)
#define COLOR_RED_DIM                   C2D_Color32(120, 20, 10, 255)

// Botones del Panel Inferior
#define COLOR_BTN_NORMAL                C2D_Color32(46, 28, 15, 255)
#define COLOR_BTN_NORMAL_BORDER         C2D_Color32(140, 85, 25, 255)
#define COLOR_BTN_PRESSED               C2D_Color32(255, 150, 10, 255)
#define COLOR_BTN_PRESSED_BORDER        C2D_Color32(255, 225, 120, 255)

#define COLOR_PURGE_BTN_NORMAL          C2D_Color32(65, 20, 14, 255)
#define COLOR_PURGE_BTN_BORDER          C2D_Color32(180, 45, 20, 255)
#define COLOR_PURGE_BTN_PRESSED         C2D_Color32(245, 45, 25, 255)

// -----------------------------------------------------------------------------
// 3. VELOCIDADES Y TIEMPOS DEL MONITOR TEMPORAL (60 FPS)
// -----------------------------------------------------------------------------
// Velocidad de ondulacion de la Sagrada Linea Temporal
#define TIMELINE_WAVE_SPEED             0.040f

// Intervalo de aparicion automatica de ramas: 1080 frames = ~18.0 segundos
#define BRANCH_SPAWN_INTERVAL_FRAMES    1080

// Velocidad de crecimiento horizontal de cada rama por frame (0.026 px/frame)
#define BRANCH_GROWTH_SPEED             0.026f

// Velocidad de retraccion al podar una rama (0.375 px/frame)
#define BRANCH_PRUNE_SPEED              0.375f

// Velocidad de desplazamiento horizontal de la marquesina "REDLINE IMMINENT"
#define MARQUEE_SCROLL_SPEED            0.75f

// Cantidad de segmentos finales en la punta que se desvanecen al podar
#define BRANCH_FADE_SEGMENTS            5
#define BRANCH_FADE_MIN_ALPHA           20
#define BRANCH_FADE_MAX_ALPHA           255

// Valor inicial del contador UNITS (5.00 en linea central -> 0.00 en linea roja)
#define BRANCH_INITIAL_UNITS            5.00f
// Umbral (< 3.00 UNITS) para activar la marquesina "REDLINE IMMINENT"
#define BRANCH_REDLINE_ALERT_UNITS      3.00f

// Exponente de curvatura de las ramificaciones
#define BRANCH_CURVE_EXPONENT           1.6f

// -----------------------------------------------------------------------------
// 4. GROSORES DE LINEA Y RETICULA '[ ]'
// -----------------------------------------------------------------------------
#define THICK_OUTER_FRAME               1.8f
#define THICK_INNER_FRAME               1.5f
#define THICK_CARD_BORDER               1.3f
#define THICK_GRID_LINE                 1.0f

// Lineas rojas de umbral (solidas, continuas y gruesas)
#define THICK_THRESHOLD_LINE            2.8f

#define THICK_SACRED_CORE               2.0f
#define THICK_SACRED_GLOW               4.0f

#define THICK_BRANCH_CORE               1.6f
#define THICK_BRANCH_GLOW               3.0f
#define THICK_BRANCH_SEL_CORE           2.2f
#define THICK_BRANCH_SEL_GLOW           3.8f
#define THICK_BRANCH_PRUNE_MIN          0.6f

// Reticula discreta '[ ]' en la punta de la rama seleccionada
#define RETICLE_HALF_SIZE               3.5f
#define RETICLE_ARM_LEN                 2.0f
#define RETICLE_THICKNESS               1.0f
#define RETICLE_CLAMP_MIN_Y             15.0f
#define RETICLE_CLAMP_MAX_Y             211.0f

// Paso en pixeles para el dibujado de curvas (optimizacion de vertices GPU)
#define BRANCH_STEP_X                   4.0f
#define SACRED_STEP_X                   4.0f

// -----------------------------------------------------------------------------
// 5. ESCALAS DE TEXTO (+2..3px DE ALTURA EN RESOLUCION NATIVA)
// -----------------------------------------------------------------------------
// Alerta superior dentro del monitor ("REDLINE IMMINENT" / "DANGER: MULTIVERSE")
#define SCALE_REDLINE_ALERT_X           0.35f
#define SCALE_REDLINE_ALERT_Y           0.40f

// Contador UNITS (Esquina superior izquierda debajo de la linea roja superior)
#define SCALE_UNITS_VALUE               0.58f
#define SCALE_UNITS_LABEL               0.35f

// Numeros de escala vertical a la derecha de la grilla ("02", "03", "04")
#define SCALE_GRID_AXIS_NUM             0.33f

// Texto decorativo interno de la grilla (TIMELINE MAG / SEGMENT)
#define SCALE_GRID_DECOR                0.28f

// Sub-rectangulos de Variante (CONFIRM IDENTITY en w = 89px)
#define SCALE_IDENTITY_HEADER_X         0.29f
#define SCALE_IDENTITY_HEADER_Y         0.33f
#define SCALE_IDENTITY_PHOTO_TAG        0.34f
#define SCALE_IDENTITY_NAME_Y           0.35f
#define SCALE_IDENTITY_BADGE_X          0.32f
#define SCALE_IDENTITY_BADGE_Y          0.36f

// Barra Inferior Estilo Terminal y Logo TVA
#define SCALE_TERMINAL_BOTTOM_X         0.38f
#define SCALE_TERMINAL_BOTTOM_Y         0.39f
#define SCALE_TVA_LOGO_X                0.40f
#define SCALE_TVA_LOGO_Y                0.40f
#define SCALE_TVA_CODE_X                0.35f
#define SCALE_TVA_CODE_Y                0.39f

// -----------------------------------------------------------------------------
// 6. LAYOUT GEOMETRICO DE LA PANTALLA SUPERIOR (400x240)
// -----------------------------------------------------------------------------
// Rectangulo 1 (Marco Exterior Unico: x = 4, y = 4, w = 392, h = 232 -> base y = 236)
#define OUTER_FRAME_X                   4.0f
#define OUTER_FRAME_Y                   4.0f
#define OUTER_FRAME_W                   392.0f
#define OUTER_FRAME_H                   232.0f

// Columna Izquierda de 5 Glifos Placeholder (x = 9..21)
#define GLYPH_COL_X                     10.0f
#define GLYPH_BOX_SIZE                  10.0f
#define GLYPH_START_Y                   22.0f
#define GLYPH_STEP_Y                    38.0f

// Area del Grafico de la Linea Temporal (x = 26 hasta x = 293, dejando 4px antes de x = 297)
#define GRAPH_BOX_X                     26.0f
#define GRAPH_BOX_Y                     10.0f
#define GRAPH_RIGHT_X                   293.0f
#define GRAPH_BOX_W                     (GRAPH_RIGHT_X - GRAPH_BOX_X)   // 267.0f
#define GRAPH_BOX_H                     206.0f
#define GRAPH_BOTTOM_Y                  216.0f

// Posiciones Y clave dentro del monitor (simetricas respecto a 113.0f)
#define TIMELINE_BASELINE_Y             113.0f
#define TIMELINE_THRESHOLD_TOP          44.0f
#define TIMELINE_THRESHOLD_BOTTOM       182.0f

// Columna de numeros de escala ("02", "03", "04") junto al borde derecho del grafico
#define GRAPH_SCALE_NUM_X               275.0f

// Mapeo cronologico en el eje X (Anios -> Posicion X de nacimiento de rama)
#define TIMELINE_YEAR_MIN               1880
#define TIMELINE_YEAR_MAX               2060
#define TIMELINE_SPAWN_X_MIN            40.0f
#define TIMELINE_SPAWN_X_MAX            196.0f
#define TIMELINE_BRANCH_MAX_X           270.0f

// Cajas de Variante ADENTRO del Segundo Rectangulo (x = 297, w = 89)
#define CARD_COL_X                      297.0f
#define CARD_COL_W                      89.0f
#define CARD_CENTER_X                   (CARD_COL_X + (CARD_COL_W * 0.5f)) // 341.5f

// Caja 1: CONFIRM IDENTITY (x = 297, y = 13, w = 89, h = 15)
#define CARD_BOX1_Y                     13.0f
#define CARD_BOX1_H                     15.0f

// Caja 2: Foto (x = 297, y = 30, w = 89, h = 104)
#define CARD_BOX2_Y                     30.0f
#define CARD_BOX2_H                     104.0f

// Caja 3: Nombre (x = 297, y = 136, w = 89, h = 20)
#define CARD_BOX3_Y                     136.0f
#define CARD_BOX3_H                     20.0f

// Caja 4: VARIANT# (x = 297, y = 158, w = 89, h = 20)
#define CARD_BOX4_Y                     158.0f
#define CARD_BOX4_H                     20.0f

// Caja del Placeholder del Logo TVA achicado 4 veces en area (w=46, h=15)
// Ubicado en la esquina inferior derecha de la columna
#define CARD_TVA_LOGO_X                 342.0f
#define CARD_TVA_LOGO_Y                 197.0f
#define CARD_TVA_LOGO_W                 46.0f
#define CARD_TVA_LOGO_H                 15.0f

// Textos de Telemetria internos del monitor (esquina inferior izquierda de Rect 2)
#define GRID_DECOR_X                    29.0f
#define GRID_DECOR_MAG_Y                198.0f
#define GRID_DECOR_SEG_Y                206.0f

// Textos inferiores apoyados sobre la base del Rectangulo 1 (y = 223, base en y = 236)
#define TERMINAL_DATE_X                 8.0f
#define TERMINAL_LINE_Y                 223.0f
#define TERMINAL_ENERGY_X               334.0f

#endif // TVA_STYLE_H
