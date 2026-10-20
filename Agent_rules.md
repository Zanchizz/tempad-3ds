# Reglas de Proyecto: TemPad 3DS

## Plataforma y Entorno
- Target: Nintendo 3DS Homebrew (`.3dsx`).
- Toolchain: devkitARM (arm-none-eabi-gcc).
- Librerías: `libctru`, `citro2d`, `citro3d`.
- Prohibido sugerir librerías estándar de PC (SDL2, Raylib, OpenGL estándar, etc.).

## Pantallas
- Top Screen: 400x240 (Sagrada Línea Temporal).
- Bottom Screen: 320x240 táctil (Panel TemPad).
- Mantener la ejecución sincronizada a 60 FPS dentro de `C3D_FrameBegin` y `C3D_FrameEnd`.