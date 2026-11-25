# TemPad 3DS

[![Nintendo 3DS](https://img.shields.io/badge/Platform-Nintendo%203DS-red.svg)](https://www.nintendo.com/)
[![devkitPro](https://img.shields.io/badge/Toolchain-devkitARM-blue.svg)](https://devkitpro.org/)
[![Libraries](https://img.shields.io/badge/Libraries-citro2d%20%7C%20libctru%20%7C%20tex3ds-orange.svg)](https://github.com/devkitPro/citro2d)
[![Release](https://img.shields.io/badge/Release-v0.2.0-orange.svg)](https://github.com/Zanchizz/tempad-3ds/releases)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

> *"For all time. Always."* — Time Variance Authority

Simulador para Nintendo 3DS del dispositivo **TemPad** de la TVA (*Time Variance Authority*), inspirado en la serie *Loki* de Marvel Studios. Desarrollado en C nativo utilizando `citro2d`, `citro3d`, `libctru` y el pipeline de texturas `tex3ds` + `romfs`.

La interfaz combina gráficos vectoriales CRT dibujados en tiempo real (grilla calibrada para PICA200, marcos TVA y simulación dinámica de líneas temporales) con sprites reales preprocesados y empaquetados en `romfs` (logo oficial de la TVA, glifos laterales, íconos del Dashboard con resplandor CRT, retratos de variantes en duotono ámbar y tipografía retro personalizada).

---

## Características Principales

* **Interfaz TVA en doble pantalla calibrada para 3DS real**: Paleta naranja cobrizo profundo ajustada para la pantalla física de Nintendo 3DS, grilla CRT dibujada con coordenadas enteras (`C2D_DrawRectSolid`) para evitar descarte sub-píxel en PICA200, y tipografía nativa `IBM Plex Mono` (`romfs/tva_font.bcfnt`).
* **Monitor de la Sagrada Línea Temporal (Pantalla Superior — 400×240)**:
  * Sagrada Línea Temporal en tono plateado/gris con flotación sutil y múltiples ramificaciones variantes doradas simultáneas que viran a rojo intenso al superar las líneas de umbral crítico.
  * Contador dinámico `UNITS`, marquesina deslizante `REDLINE IMMINENT`, estado de emergencia `DANGER: MULTIVERSE`, ficha de variante seleccionada (`VARIANT#`, año y retrato en duotono ámbar CRT) y podado selectivo con animación progresiva de desintegración.
  * Animación de apertura de Timedoor en tiempo real cuando hay un portal activo.
* **Dashboard flotante con glow CRT (Pantalla Inferior — 320×240)**: Íconos flotantes sin cajas con resplandor CRT (`TIMEDOOR`, `MISS MINUTES`, `SETTINGS`, `DIRECTORY`) sobre la grilla de fondo y lista de comandos numerada (`ACTION LIST //////`).
* **Timedoor en 2 pasos (`SAVED TIMES` → `SET DATE`)**:
  * Catálogo modular de destinos en `include/tva_catalog.h` (`2026, EARTH`, `1705, LAMENTIS`, `2012, EARTH`, `1412, TITAN`, `2077, ASGARD`).
  * Selector táctil y por D-Pad de **Día** y **Mes** (con inicio en `00/00`, validación real por mes/año bisiesto y año fijo por destino) con actualización en vivo de `COORDINATES`.
  * Portal persistente (`TIMEDOOR OPEN` / `PORTAL LOCKED`) con animación activa en la pantalla superior hasta el próximo toque en pantalla o botón `B`.
* **Pipeline nativo de assets (`tex3ds` + `romfs`)**: Preprocesamiento automatizado mediante `tools/prep_assets.py` (duotono ámbar CRT, scanlines, auto-recorte y halo glow) compilado directamente en `romfs/gfx/ui_sprites.t3x` y `romfs/gfx/variants.t3x`.
* **Rendimiento nativo**: 60 FPS constantes acelerados por hardware (GPU PICA200) a través de `citro2d` y `citro3d`.

---

## Hoja de ruta (Roadmap)

**General e Interfaz**
- [x] Arquitectura modular en doble pantalla (`timeline`, `dashboard`, `timedoor`, `assets`, `tva_style.h`, `tva_catalog.h`) y bucle principal a 60 FPS.
- [x] Dashboard táctil flotante con efecto glow CRT y ejecución en dos pasos (selección + `1. RUN PROGRAM` o doble toque).
- [x] Submenú `SAVED TIMES` con selector de destinos, selector de fecha (`DAY`/`MONTH`) y portal persistente con animación en pantalla superior.
- [x] Traducción íntegra al inglés de toda la interfaz, menús y alertas con terminología canónica de la TVA.
- [x] Reemplazo de la fuente del sistema por tipografía retro personalizada `.bcfnt` (`IBM Plex Mono` 16pt en `romfs/tva_font.bcfnt`).
- [x] Incorporación de recursos gráficos reales mediante `tex3ds` y `romfs` (logo oficial TVA, glifos laterales, íconos del Dashboard con glow CRT y fotos de variantes en duotono ámbar).
- [x] Calibración de paleta naranja profundo y grilla entera para hardware real de Nintendo 3DS (GPU PICA200).
- [ ] Renderizado alámbrico en 3D estereoscópico de la Timedoor en pantalla superior.
- [ ] Efectos de audio retro y zumbido analógico de monitor CRT mediante `ndsp`.

**Monitor de la Sagrada Línea Temporal (Pantalla Superior)**
- [x] Oscilación suave de la Sagrada Línea Temporal y generación/purga de ramas.
- [x] Fidelidad de color canónica: línea temporal principal en tono blanco/grisáceo; ramificaciones variantes en dorado brillante que viran al rojo intenso al cruzar el umbral crítico.
- [x] Geometría de ramificaciones: crecimiento con curva exponencial independiente de la onda principal, mapeo cronológico, contador `UNITS` y cruce de líneas de umbral rojas sólidas.
- [x] Múltiples ramas simultáneas, marquesina `REDLINE IMMINENT` y estado de emergencia `DANGER: MULTIVERSE`.
- [x] Podado selectivo con retícula `[ ]` en la punta de la rama seleccionada y animación progresiva de desintegración (manteniendo `3. PRUNE TIMELINE` para purga total).
- [x] Comando `2. CHECK STATUS` con mensaje `STATUS: UPDATING TIMELINE...` y visor de ficha de variantes (`VARIANT#` + fotografía CRT).

**Módulos del Dashboard (Pantalla Inferior)**
- [x] **TIMEDOOR (Base y selector de fecha)**: Catálogo modular de destinos en `include/tva_catalog.h`, flujo en 2 pasos (`SAVED TIMES` → `SET DATE` con `DAY`/`MONTH` desde `00/00` y año fijo), coordenadas dinámicas en tiempo real y apertura de portal persistente con animación en pantalla superior.
- [ ] **TIMEDOOR (Expansión 3D / Archive Feed)**: Proyección estereoscópica 3D en pantalla superior y vistas previas reales dentro de `[ ARCHIVE FEED ]`.
- [ ] **MISS MINUTES**: Sección interactiva estilo asistente con diálogos preprogramados (ej. `"Introduction to the TVA"`) y soporte futuro para clips de voz mediante `ndsp`.
- [ ] **DIRECTORY**: Base de datos navegable con archivos de personajes, variantes y lore de la TVA.
- [ ] **SETTINGS**: Menú de configuración general del sistema y opciones de interfaz.

---

## Instalación

1. Descarga el paquete comprimido más reciente (`tempad-3ds-vX.X.X.zip`) desde la sección [Releases](https://github.com/Zanchizz/tempad-3ds/releases) de este repositorio.
2. Descomprime el contenido directamente en la raíz de la tarjeta SD de tu Nintendo 3DS (colocará el archivo ejecutable dentro de `SD:/3ds/tempad-3ds/`).
3. Vuelve a colocar la tarjeta SD en tu consola e inicia la aplicación desde el **Homebrew Launcher**.

### Envío rápido por red local (`3dslink`)

Si tu PC y tu consola están en la misma red Wi-Fi:

1. Abre el **Homebrew Launcher** en tu 3DS y presiona el botón **Y** para activar el receptor NetLoader.
2. En la terminal de tu PC ejecuta:
   ```bash
   3dslink -a <IP_DE_TU_3DS> tempad-3ds.3dsx
   ```

---

## Controles

| Entrada | Acción |
| :--- | :--- |
| **Pantalla táctil (Stylus)** | Seleccionar módulos y acciones en Dashboard (doble toque en `TIMEDOOR` abre directo), elegir destino en `SAVED TIMES`, ajustar `DAY`/`MONTH` con `▲`/`▼`, abrir Timedoor y tocar la pantalla para cerrar un portal abierto |
| **D-Pad Izq. / Der. o L / R** | En Dashboard: seleccionar rama variante anterior/siguiente; en `SET DATE`: alternar foco entre `DAY` y `MONTH` |
| **D-Pad Arriba / Abajo** | En Dashboard: navegar módulos; en `SAVED TIMES`: cambiar destino; en `SET DATE`: incrementar o decrementar `DAY`/`MONTH` |
| **Botón A** | En Dashboard: podar la rama seleccionada (si hay activas) o abrir el módulo elegido; en Timedoor: avanzar al selector de fecha / abrir Timedoor |
| **Botón X** | Podar selectivamente la rama variante activa desde el Dashboard |
| **Botón B** | Cerrar el Timedoor abierto / volver al paso anterior (`SET DATE` → `SAVED TIMES` → Dashboard) |
| **Botón Start** | Cerrar la aplicación y volver al Homebrew Launcher |

---

## Estructura del Proyecto

```text
tempad-3ds/
├── include/
│   ├── tva_style.h        # Paleta CRT, grosores, escalas y velocidades (estilo "CSS")
│   ├── tva_catalog.h      # Catálogo editable de variantes y destinos Timedoor
│   ├── tempad.h           # Estados globales y utilidades compartidas de dibujado
│   ├── assets.h           # Carga de spritesheets tex3ds en romfs
│   ├── timeline.h         # API del monitor de la Sagrada Línea Temporal
│   ├── dashboard.h        # API del Dashboard inferior
│   └── timedoor.h         # API del módulo Timedoor y estado del portal
├── source/                # Implementación modular en C (.c)
├── gfx/                   # Sprites preprocesados y descriptores .t3s para tex3ds
├── romfs/                 # Fuente tva_font.bcfnt y atlas compilados (.t3x)
├── tools/prep_assets.py   # Preprocesador Python (duotono CRT, glow, recorte)
└── Makefile               # Compilación 3DS (.3dsx + empaquetado tex3ds/romfs)
```

---

## Compilación desde el código fuente

### Requisitos previos

* Entorno [devkitPro](https://devkitpro.org/) con el grupo de paquetes `3ds-dev` instalado (`arm-none-eabi-gcc`, `libctru`, `citro2d`, `citro3d`, `tex3ds`).
* *(Opcional)* Python 3 con `Pillow` únicamente si deseas reprocesar los sprites crudos mediante `tools/prep_assets.py`.

### Instrucciones

1. Clona el repositorio:
   ```bash
   git clone https://github.com/Zanchizz/tempad-3ds.git
   ```

2. Entra a la carpeta del proyecto:
   ```bash
   cd tempad-3ds
   ```

3. Compila las texturas `tex3ds` y el binario ejecutable:
   ```bash
   make clean && make
   ```

El archivo ejecutable `tempad-3ds.3dsx` (con la fuente y los spritesheets empaquetados en `romfs`) se generará en la raíz del proyecto.

---

## Aviso legal y créditos

Proyecto experimental sin fines de lucro desarrollado con fines recreativos y educativos. Los conceptos visuales del TemPad y la estética de la TVA pertenecen a Marvel Studios.

Inspiración en diseño de comunidad de Figma (Izzy Muda): https://www.figma.com/community/file/994716661475712123/loki-tva-tempad?after-auth-duplicate-file-id=994716661475712123
