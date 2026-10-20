# TemPad 3DS

[![Nintendo 3DS](https://img.shields.io/badge/Platform-Nintendo%203DS-red.svg)](https://www.nintendo.com/)
[![devkitPro](https://img.shields.io/badge/Toolchain-devkitARM-blue.svg)](https://devkitpro.org/)
[![Libraries](https://img.shields.io/badge/Libraries-citro2d%20%7C%20libctru-orange.svg)](https://github.com/devkitPro/citro2d)
[![Release](https://img.shields.io/badge/Release-v0.1.0-amber.svg)](https://github.com/Zanchizz/tempad-3ds/releases)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

> *"For all time. Always."* — Time Variance Authority

Simulador para Nintendo 3DS del dispositivo **TemPad** de la TVA (*Agencia de Variación Temporal*), inspirado en la serie *Loki* de Marvel Studios. Desarrollado en C utilizando `citro2d` y `libctru`.

Nota sobre el apartado visual: Todos los gráficos actuales están generados de forma procedural por código como placeholders funcionales y están sujetos a cambios artísticos a medida que se integren las texturas y fuentes definitivas.
---

## Características Principales

* **Interfaz retro en doble pantalla**: Estética monocromática ámbar fósforo adaptada a las resoluciones nativas de la consola (Pantalla superior: 400x240, Pantalla táctil inferior: 320x240).
* **Monitor de la Sagrada Línea Temporal**: Visualización en tiempo real en la pantalla superior con generación de ramificaciones Nexus y alertas de umbral crítico.
* **Panel de control táctil (Dashboard)**: Navegación de estilo retro/analógico donde se selecciona el módulo y se ejecuta mediante comandos en pantalla.
* **Rendimiento nativo**: Tasa de refresco constante a 60 FPS acelerada por hardware (GPU PICA200) a través de `citro2d`.

---

## Hoja de ruta (Roadmap)
General e Interfaz
- [x] Arquitectura base en doble pantalla y bucle principal a 60 FPS.
- [x] Dashboard táctil inicial con selección de programas y ejecución en dos pasos (Select + Run Program).
- [x] Submenú "Saved Times" con selector interactivo de coordenadas y épocas.
- [ ] Renderizado alámbrico en 3D estereoscópico de la Timedoor en pantalla superior.
- [ ] Traducción íntegra al inglés: Adaptación de todos los textos y alertas a la terminología exacta de la serie
- [ ] Incorporación de recursos gráficos mediante romfs (Miss Minutes, logo TVA): Carga de plantillas de fondo originales, glifos laterales, logo oficial de la TVA e iconos desde diseño en Figma.
- [ ] Efectos de audio retro y zumbido analógico de monitor CRT mediante ndsp.
- [ ] Reemplazo de la fuente del sistema por una fuente retro .bcfnt para corregir el espaciado entre caracteres (kerning)

**Monitor de la Sagrada Línea Temporal (Pantalla Superior)**
- [x] Oscilación de la línea principal y generación/purga básica de ramas.

- [ ] Fidelidad de color canónica: Línea temporal principal en tono blanco/grisáceo; ramificaciones variantes en naranja que viran al rojo al cruzar el umbral.

- [ ] Geometría de ramificaciones: Crecimiento de ramas con curva lineal/exponencial firme (sin heredar la ondulación flotante de la línea principal) y capacidad de sobrepasar las líneas punteadas de límite.

- [ ] Múltiples ramas y Alerta Multiversal: Soporte para numerosas ramificaciones simultáneas y estado de emergencia "DANGER: MULTIVERSE" al acumularse múltiples ramas rojas.

- [ ] Podado selectivo y animación de borrado: Opción para inspeccionar/seleccionar una rama específica y animación progresiva de desintegración desde la punta exterior hasta la línea principal (manteniendo el purgado total rápido para depuración).

- [ ] Renombrado de comando de estado: Cambio de Info Estado a "CHECK STATUS" con mensaje en pantalla "UPDATING TIMELINE...".

**Módulos del Dashboard (Pantalla Inferior)**
- [x] TIMEDOOR (Base): Submenú funcional con lista de destinos precargados.

- [ ] TIMEDOOR (Expansión): Estructura de código modular para agregar Locations fácilmente, selector manual de fecha/lugar entre destinos precargados y proyección 3D en pantalla superior.

- [ ] MISS MINUTES: Sección interactiva estilo asistente/chatbot con diálogos preprogramados (ej. "Introduction to the TVA") y soporte futuro para clips de voz mediante ndsp.

- [ ] DIRECTORY: Base de datos navegable con archivos de personajes, variantes y lore de la TVA.

- [ ] SETTINGS: Menú de configuración general del sistema y opciones de interfaz.
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
Inspiracion en Figma de la comunidad (Izzy Muda): https://www.figma.com/community/file/994716661475712123/loki-tva-tempad?after-auth-duplicate-file-id=994716661475712123
