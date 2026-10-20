# TemPad 3DS

[![Nintendo 3DS](https://img.shields.io/badge/Platform-Nintendo%203DS-red.svg)](https://www.nintendo.com/)
[![devkitPro](https://img.shields.io/badge/Toolchain-devkitARM-blue.svg)](https://devkitpro.org/)
[![Libraries](https://img.shields.io/badge/Libraries-citro2d%20%7C%20libctru-orange.svg)](https://github.com/devkitPro/citro2d)
[![Release](https://img.shields.io/badge/Release-v0.1.0-amber.svg)](https://github.com/Zanchizz/tempad-3ds/releases)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

> *"For all time. Always."* — Time Variance Authority

Simulador para Nintendo 3DS del dispositivo **TemPad** de la TVA (*Agencia de Variación Temporal*), inspirado en la serie *Loki* de Marvel Studios. Desarrollado en C utilizando `citro2d` y `libctru`.

---

## Características

* **Interfaz retro en doble pantalla**: Estética monocromática ámbar fósforo adaptada a las resoluciones nativas de la consola (Pantalla superior: 400x240, Pantalla táctil inferior: 320x240).
* **Monitor de la Sagrada Línea Temporal**: Animación en tiempo real en la pantalla superior con detección y despliegue de ramificaciones ante eventos Nexus.
* **Panel de control táctil (Dashboard)**: Selector de programas interactivo en la pantalla inferior con respuesta visual al tacto mediante `hidTouchRead`.
* **Rendimiento nativo**: Tasa de refresco constante a 60 FPS acelerada por hardware (GPU PICA200) a través de `citro2d`.

---

## Hoja de ruta (Roadmap)

- [x] Monitor de la Sagrada Línea Temporal con oscilaciones y alertas Nexus.
- [x] Panel de control interactivo en pantalla táctil con respuesta háptica/visual.
- [x] Submenú "Saved Times" con selector interactivo de coordenadas y épocas.
- [ ] Renderizado alámbrico en 3D estereoscópico de la Timedoor en pantalla superior.
- [ ] Incorporación de recursos gráficos mediante romfs (Miss Minutes, logo TVA).
- [ ] Efectos de audio retro y zumbido analógico de monitor CRT mediante ndsp.

---

## Instalación

1. Descarga el paquete comprimido más reciente (`tempad-3ds-vX.X.X.zip`) desde la sección [Releases](https://github.com/Zanchizz/tempad-3ds/releases) de este repositorio.
2. Descomprime el contenido directamente en la raíz de la tarjeta SD de tu Nintendo 3DS (colocará el archivo ejecutable dentro de `SD:/3ds/tempad-3ds/`).
3. Vuelve a colocar la tarjeta SD en tu consola e inicia la aplicación desde el **Homebrew Launcher**.

### Envío rápido por red local (3dslink)

Si tu PC y tu consola están en la misma red Wi-Fi:

1. Abre el **Homebrew Launcher** en tu 3DS y presiona el botón **Y** para activar el receptor NetLoader.
2. En la terminal de tu PC ejecuta:
   ```bash
   3dslink -address <IP_DE_TU_3DS> tempad-3ds.3dsx
   ```

---

## Controles

| Entrada | Acción |
| :--- | :--- |
| **Pantalla táctil (Stylus)** | Navegar, seleccionar programas y activar comandos de acción |
| **Botón A** | Confirmar selección / Abrir programa |
| **Botón B** | Volver al menú anterior / Dashboard principal |
| **Botón Start** | Cerrar la aplicación y volver al Homebrew Launcher |

---

## Compilación desde el código fuente

### Requisitos previos

Entorno [devkitPro](https://devkitpro.org/) con la suite de paquetes `3ds-dev` instalada (`arm-none-eabi-gcc`, `libctru`, `citro2d`, `citro3d`).

### Instrucciones

1. Clona el repositorio:
   ```bash
   git clone https://github.com/Zanchizz/tempad-3ds.git
   ```

2. Entra a la carpeta del proyecto:
   ```bash
   cd tempad-3ds
   ```

3. Compila el binario:
   ```bash
   make
   ```

El archivo ejecutable `tempad-3ds.3dsx` se generará en la raíz del proyecto.

---

## Aviso legal y créditos

Proyecto experimental sin fines de lucro desarrollado con fines recreativos y educativos. Los conceptos visuales del TemPad y la estética de la TVA pertenecen a Marvel Studios.
