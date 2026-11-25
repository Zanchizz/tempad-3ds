# Reglas de Proyecto: TemPad 3DS

## Plataforma y Entorno
- Target: Nintendo 3DS Homebrew (`.3dsx`).
- Toolchain: devkitARM (arm-none-eabi-gcc).
- Librerías: `libctru`, `citro2d`, `citro3d`.
- Prohibido sugerir librerías estándar de PC (SDL2, Raylib, OpenGL estándar, etc.).

## Pantallas y Renderizado
- Top Screen: 400x240 (Sacred Timeline Monitor).
- Bottom Screen: 320x240 táctil (TemPad Control Panel).
- Mantener la ejecución sincronizada a 60 FPS dentro de `C3D_FrameBegin` y `C3D_FrameEnd`.
- Fidelidad de color canónica: Línea Temporal Principal en blanco/gris claro brillante (`CLR_TIMELINE_MAIN`); ramificaciones variantes en naranja/ámbar TVA (`CLR_AMBER_BRIGHT`) que viran al rojo crítico (`CLR_CRITICAL_RED`) al acercarse al umbral.
- Idioma de la interfaz: 100% en inglés con la terminología oficial de la TVA.

## Arquitectura Modular
- `include/tempad.h`: Definición de `AppState`, constantes de pantalla, paleta de colores canónica de la TVA y primitivas compartidas.
- `include/timeline.h` y `source/timeline.c`: Lógica, estado y renderizado de la pantalla superior (Sacred Timeline y ramificaciones Nexus).
- `include/dashboard.h` y `source/dashboard.c`: Renderizado y control táctil/físico del Dashboard principal (matriz 2x2 y lista de acciones).
- `include/timedoor.h` y `source/timedoor.c`: Submenú de selección de destinos temporales ("Saved Times") basado en `struct Location` y vista de Timedoor activa.
- `source/main.c`: Inicialización de `gfx`/`citro2d`, bucle principal `aptMainLoop()`, lectura de inputs globales (`hidScanInput`) y despacho por estado `AppState`.
