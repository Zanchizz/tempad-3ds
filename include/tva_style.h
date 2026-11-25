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
// 2. PALETA NARANJA PROFUNDO TVA (CALIBRADA PARA PANTALLA FISICA 3DS — RGBA: 0..255)
// -----------------------------------------------------------------------------
// Fondos y Paneles CRT
#define COLOR_BG_DARK                   C2D_Color32(8, 2, 0, 255)        // #080200 Fondo general
#define COLOR_PANEL_BG                  C2D_Color32(26, 8, 1, 220)
#define COLOR_PANEL_INSET               C2D_Color32(14, 4, 0, 210)
#define COLOR_BORDER_DIM                C2D_Color32(110, 32, 2, 255)     // Naranja tenue / apagado
#define COLOR_BORDER_MID                C2D_Color32(190, 60, 4, 255)     // Naranja medio (bordes secundarios)
#define COLOR_GRID_LINE                 C2D_Color32(105, 32, 2, 85)      // Grilla naranja profunda sutil
#define COLOR_SCANLINE                  C2D_Color32(242, 82, 4, 85)      // Scanline CRT en Timedoor

// Logo TVA (fallback y referencia)
#define COLOR_TVA_LOGO                  C2D_Color32(220, 68, 4, 255)
#define COLOR_TVA_LOGO_BG               C2D_Color32(32, 10, 1, 255)
#define COLOR_TVA_LOGO_TEXT             C2D_Color32(220, 68, 4, 255)

// Sagrada Linea Temporal (Gris / Plateado Apagado)
#define COLOR_SACRED_CORE               C2D_Color32(175, 180, 190, 255)
#define COLOR_SACRED_GLOW               C2D_Color32(100, 105, 115, 180)
#define COLOR_SACRED_MAIN               C2D_Color32(150, 155, 165, 255)

// Ramificaciones Variantes (Amarillo / Dorado brillante para contrastar con la UI naranja)
#define COLOR_BRANCH_GOLD_CORE          C2D_Color32(255, 140, 0, 255)    // Nucleo de rama variante
#define COLOR_BRANCH_GOLD_SEL           C2D_Color32(255, 240, 145, 255)  // Nucleo de rama seleccionada
#define COLOR_BRANCH_GOLD_GLOW          C2D_Color32(245, 175, 25, 255)   // Brillo / borde dorado

// UI Naranja Profundo TVA
#define COLOR_TVA_ORANGE                C2D_Color32(242, 82, 4, 255)     // Naranja principal vivo
#define COLOR_BRANCH_AMBER              COLOR_TVA_ORANGE
#define COLOR_BRANCH_SELECTED           C2D_Color32(255, 152, 32, 255)   // Naranja brillante de seleccion UI
#define COLOR_BRANCH_AMBER_GLOW         C2D_Color32(235, 58, 0, 115)
#define COLOR_AMBER_TEXT                COLOR_TVA_ORANGE
#define COLOR_AMBER_DIM                 C2D_Color32(110, 32, 2, 255)     // Naranja tenue / apagado
#define COLOR_TEXT_INVERTED             C2D_Color32(13, 8, 4, 255)       // #0D0804 Negro de contraste

// Alerta Critica Nexus y Lineas de Umbral Rojas Solidas
#define COLOR_BRANCH_RED                C2D_Color32(250, 48, 24, 255)
#define COLOR_REDLINE_SOLID             C2D_Color32(235, 45, 20, 245)
#define COLOR_BRANCH_RED_GLOW           C2D_Color32(220, 35, 15, 130)
#define COLOR_RED_DIM                   C2D_Color32(120, 20, 10, 255)

// Botones del Panel Inferior (marron oscuro calido semitransparente para insinuar grilla)
#define COLOR_BTN_NORMAL                C2D_Color32(42, 13, 2, 195)      // Marron oscuro calido
#define COLOR_BTN_NORMAL_BORDER         C2D_Color32(190, 60, 4, 255)     // Naranja medio
#define COLOR_BTN_PRESSED               C2D_Color32(242, 82, 4, 255)     // Naranja principal vivo
#define COLOR_BTN_PRESSED_BORDER        C2D_Color32(255, 152, 32, 255)

#define COLOR_PURGE_BTN_NORMAL          C2D_Color32(68, 18, 10, 200)
#define COLOR_PURGE_BTN_BORDER          C2D_Color32(200, 48, 22, 255)
#define COLOR_PURGE_BTN_PRESSED         C2D_Color32(245, 45, 25, 255)

// Dashboard Flotante con Glow CRT (Sin cajas de botones)
#define COLOR_DASH_DIM                  C2D_Color32(165, 50, 4, 255)     // Naranja inactivo (reposo)
#define COLOR_DASH_GLOW_HALO            C2D_Color32(235, 58, 0, 85)      // Halo CRT naranja translucido
#define COLOR_DASH_GLOW_CORE            C2D_Color32(255, 152, 32, 255)   // Nucleo brillante naranja profundo
#define COLOR_DASH_RED_DIM              C2D_Color32(170, 56, 32, 255)    // Accion critica inactiva
#define COLOR_DASH_RED_HALO             C2D_Color32(255, 45, 20, 80)     // Halo CRT rojo translucido
#define COLOR_DASH_RED_CORE             C2D_Color32(255, 105, 80, 255)   // Nucleo texto critico brillante

// -----------------------------------------------------------------------------
// 3. VELOCIDADES Y TIEMPOS DEL MONITOR TEMPORAL (60 FPS)
// -----------------------------------------------------------------------------
// Flotacion sutil de la Sagrada Linea Temporal (editable a mano)
// SACRED_WAVE_AMPLITUDE: amplitud vertical maxima en pixeles (~1.5f px)
// SACRED_WAVE_SPEED: velocidad de desplazamiento por frame (0.5x mas calma)
#define SACRED_WAVE_AMPLITUDE           1.50f
#define SACRED_WAVE_SPEED               0.020f
#define TIMELINE_WAVE_SPEED             SACRED_WAVE_SPEED

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
#define GRID_LINE_THICKNESS             1.0f
#define THICK_GRID_LINE                 GRID_LINE_THICKNESS

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
// 5. ESCALAS DE TEXTO CALIBRADAS PARA FUENTE NATIVA 16PX (scaleX >= 0.85 * scaleY)
// -----------------------------------------------------------------------------
// Alerta superior dentro del monitor ("REDLINE IMMINENT" / "DANGER: MULTIVERSE")
#define SCALE_REDLINE_ALERT_X           0.56f
#define SCALE_REDLINE_ALERT_Y           0.60f

// Contador UNITS (Esquina superior izquierda debajo de la linea roja superior)
#define SCALE_UNITS_VALUE               0.86f
#define SCALE_UNITS_LABEL               0.54f

// Numeros de escala vertical a la derecha de la grilla ("02", "03", "04")
#define SCALE_GRID_AXIS_NUM             0.50f

// Texto decorativo interno de la grilla ("MAG: 70.62.0738" / "SEG: 616.432")
#define SCALE_GRID_DECOR                0.48f

// Sub-rectangulos de Variante (CONFIRM IDENTITY en x = 287, w = 99px)
#define SCALE_IDENTITY_HEADER_X         0.45f
#define SCALE_IDENTITY_HEADER_Y         0.52f
#define SCALE_IDENTITY_PHOTO_TAG        0.52f
#define SCALE_IDENTITY_NAME_Y           0.56f
#define SCALE_IDENTITY_BADGE_X          0.50f
#define SCALE_IDENTITY_BADGE_Y          0.56f

// Barra Inferior Estilo Terminal y Logo TVA
#define SCALE_TERMINAL_BOTTOM_X         0.56f
#define SCALE_TERMINAL_BOTTOM_Y         0.60f
#define SCALE_TVA_LOGO_X                0.56f
#define SCALE_TVA_LOGO_Y                0.58f
#define SCALE_TVA_CODE_X                0.54f
#define SCALE_TVA_CODE_Y                0.58f

// -----------------------------------------------------------------------------
// 6. LAYOUT GEOMETRICO DE LA PANTALLA SUPERIOR (400x240)
// -----------------------------------------------------------------------------
// Rectangulo 1 (Marco Exterior Unico: x = 4, y = 4, w = 392, h = 232 -> base y = 236)
#define OUTER_FRAME_X                   4.0f
#define OUTER_FRAME_Y                   4.0f
#define OUTER_FRAME_W                   392.0f
#define OUTER_FRAME_H                   232.0f

// Columna Izquierda de 5 Glifos (12x12 px en x = 9..21)
#define GLYPH_COL_X                     9.0f
#define GLYPH_BOX_SIZE                  12.0f
#define GLYPH_START_Y                   22.0f
#define GLYPH_STEP_Y                    38.0f

// Area del Grafico de la Linea Temporal (x = 26 hasta x = 284, dejando 3px antes de x = 287)
#define GRAPH_BOX_X                     26.0f
#define GRAPH_BOX_Y                     10.0f
#define GRAPH_RIGHT_X                   284.0f
#define GRAPH_BOX_W                     (GRAPH_RIGHT_X - GRAPH_BOX_X)   // 258.0f
#define GRAPH_BOX_H                     206.0f
#define GRAPH_BOTTOM_Y                  216.0f

// Posiciones Y clave dentro del monitor (simetricas respecto a 113.0f)
#define TIMELINE_BASELINE_Y             113.0f
#define TIMELINE_THRESHOLD_TOP          44.0f
#define TIMELINE_THRESHOLD_BOTTOM       182.0f

// Columna de numeros de escala ("02", "03", "04") junto al borde derecho del grafico
#define GRAPH_SCALE_NUM_X               264.0f

// Mapeo cronologico en el eje X (Anios -> Posicion X de nacimiento de rama)
#define TIMELINE_YEAR_MIN               1880
#define TIMELINE_YEAR_MAX               2060
#define TIMELINE_SPAWN_X_MIN            40.0f
#define TIMELINE_SPAWN_X_MAX            188.0f
#define TIMELINE_BRANCH_MAX_X           262.0f

// Cajas de Variante ADENTRO del Segundo Rectangulo (x = 287, w = 99)
#define CARD_COL_X                      287.0f
#define CARD_COL_W                      99.0f
#define CARD_CENTER_X                   (CARD_COL_X + (CARD_COL_W * 0.5f)) // 336.5f

// Caja 1: CONFIRM IDENTITY (x = 287, y = 13, w = 99, h = 15)
#define CARD_BOX1_Y                     13.0f
#define CARD_BOX1_H                     15.0f

// Caja 2: Foto (x = 287, y = 30, w = 99, h = 104)
#define CARD_BOX2_Y                     30.0f
#define CARD_BOX2_H                     104.0f

// Caja 3: Nombre (x = 287, y = 136, w = 99, h = 20)
#define CARD_BOX3_Y                     136.0f
#define CARD_BOX3_H                     20.0f

// Caja 4: VARIANT# (x = 287, y = 158, w = 99, h = 20)
#define CARD_BOX4_Y                     158.0f
#define CARD_BOX4_H                     20.0f

// Sprite del Logo TVA (44x20 px en esquina inferior derecha)
#define CARD_TVA_LOGO_X                 346.0f
#define CARD_TVA_LOGO_Y                 196.0f
#define CARD_TVA_LOGO_W                 44.0f
#define CARD_TVA_LOGO_H                 20.0f

// Textos de Telemetria internos del monitor (esquina inferior izquierda de Rect 2)
#define GRID_DECOR_X                    29.0f
#define GRID_DECOR_MAG_Y                190.0f
#define GRID_DECOR_SEG_Y                201.0f

// Textos inferiores apoyados sobre la base del Rectangulo 1 (y = 219, base en y = 236)
#define TERMINAL_DATE_X                 8.0f
#define TERMINAL_LINE_Y                 219.0f
#define TERMINAL_ENERGY_X               324.0f

#endif // TVA_STYLE_H
