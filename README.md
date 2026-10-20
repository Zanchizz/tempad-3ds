# ⏳ TemPad 3DS (Time Variance Authority)

[![Nintendo 3DS](https://img.shields.io/badge/Platform-Nintendo%203DS-red.svg)](https://www.nintendo.com/)
[![devkitPro](https://img.shields.io/badge/Toolchain-devkitARM-blue.svg)](https://devkitpro.org/)
[![Libraries](https://img.shields.io/badge/Libraries-citro2d%20%7C%20libctru-orange.svg)](https://github.com/devkitPro/citro2d)
[![Release](https://img.shields.io/badge/Release-v0.1.0-amber.svg)](https://github.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

> *"For all time. Always."* — Time Variance Authority

**TemPad 3DS** es un homebrew para la familia de consolas **Nintendo 3DS / 2DS** que recrea fielmente el dispositivo **TemPad** de la TVA (*Time Variance Authority*), inspirado en la serie *Loki* de Marvel Studios.

Aprovecha la arquitectura de doble pantalla y la pantalla táctil resistiva de la 3DS junto a la aceleración por GPU (PICA200) mediante **citro2d** y **citro3d**, sincronizado a 60 FPS estables con una estética retro de fósforo ámbar y CRT de los años 70.

---


## ✨ Características (v0.1.0)

### 🖥️ Pantalla Superior (Top Screen - 400x240)
* **Sagrada Línea Temporal Viva (60 FPS)**: Renderizado de osciloscopio CRT con armónicos sinusoidales continuos, compuesto por tres capas gráficas: resplandor exterior (*glow*), núcleo ámbar intenso y filamento de fósforo incandescente.
* **Umbrales Críticos de Evento Nexus**: Líneas rojas punteadas que delimitan el punto de no retorno temporal (*redline*).
* **Ramificaciones Dinámicas (Nexus Events)**: Desviación progresiva de variantes temporales con interpolación suave (*lerp*), alerta visual al aproximarse al umbral y sistema de purga (*Reset Charge*).
* **Proyección de Brecha Timedoor**: Cuando se abre una puerta temporal, la pantalla superior proyecta una fisura vertical de energía con anillos de expansión cuántica sobre la coordenada exacta.
* **Telemetría y Estado**: Indicador de modo en tiempo real (`[MODE: DASHBOARD]`, `[MODE: TIMEDOOR SELECT]`, `[MODE: TIMEDOOR ACTIVE]`) y porcentaje de divergencia del continuo.

### 📱 Pantalla Inferior (Bottom Screen - 320x240 Táctil)
Arquitectura basada en una máquina de estados de interfaz (`AppState`):

1. **Estado 1: Dashboard TVA (`STATE_DASHBOARD`)**
   * **Matriz 2x2**: Acceso directo a `TIMEDOOR`, `MISS MINUTES`, `SETTINGS` y `DIRECTORY`, con glifos vectoriales retro y respuesta táctil.
   * **Action List**: Botones laterales para `RUN PROGRAM`, `INFO ESTADO` (generación de variantes de prueba) y `PURGAR RAMA`.
   * **Doble Tap**: Acceso instantáneo a la selección de destinos al presionar dos veces `TIMEDOOR`.

2. **Estado 2: Submenú "Saved Times / Select Time" (`STATE_TIMEDOOR_SELECT`)**
   * **Columna Izquierda (Preview de Destino)**:
     * Visor de cámara / thumbnail (130x65 px) con marco de fósforo, retícula de enfoque y *scanline* animada.
     * Nombre de la época y ubicación en gran formato.
     * Código de coordenadas TVA y clasificación de era.
     * Botón táctil `[ ABRIR TIMEDOOR ]`.
   * **Columna Derecha (Lista de Destinos)**:
     * Encabezado decorativo `SAVED TIMES //////`.
     * Destinos disponibles:
       * `1705, LAMENTIS` (Sector C-2 // Extinción)
       * `2012, EARTH` (Nueva York // Battle of NY)
       * `1412, TITAN` (Galaxia M-31 // Pre-colapso)
       * `2077, ASGARD` (Reino Dorado // Cataclismo)
     * Resaltado en ámbar sólido con texto invertido para la celda activa y actualización en tiempo real del visor.
     * Barra de desplazamiento lateral interactiva (*scrollbar*) con flechas y cursor (*thumb*).

3. **Estado 3: Timedoor Desplegada (`STATE_TIMEDOOR_ACTIVE`)**
   * Representación geométrica del umbral dorado con energía pulsante.
   * Telemetría de sincronización y paso seguro.
   * Retorno rápido con botón `B` o toque en pantalla.

---

## 🎮 Controles

| Botón / Entrada | Función |
| :--- | :--- |
| **Pantalla Táctil** | Selección directa de aplicaciones, botones y destinos; respuesta táctil visual continua (*stylus* o dedo). |
| **Doble Toque (Touch)** | Abrir inmediatamente `TIMEDOOR` o desplegar el destino seleccionado. |
| **Cruz de Control (D-Pad)** | Navegar entre celdas de la matriz 2x2 o subir/bajar en la lista de destinos. |
| **Botón A** | Confirmar selección / Ejecutar programa / Abrir Timedoor. |
| **Botón B** | Volver al menú anterior / Cerrar Timedoor / Retorno a Dashboard. |
| **START** | Salir de la aplicación y regresar al Homebrew Launcher. |

---

## 🛠️ Requisitos de Compilación

Para compilar el proyecto desde el código fuente necesitas el entorno oficial de **devkitPro** configurado para Nintendo 3DS:

* **devkitARM** (compilador `arm-none-eabi-gcc`)
* **libctru** (librería base del sistema 3DS)
* **citro2d** & **citro3d** (motor de renderizado 2D/3D por GPU)

### Instrucciones de Compilación

1. Clona el repositorio:
   ```bash
   git clone https://github.com/TU_USUARIO/tempad-3ds.git
   cd tempad-3ds
   ```

2. Compila el ejecutable `.3dsx`:
   ```bash
   make
   ```

   El archivo resultante se generará en la raíz: `tempad-3ds.3dsx`.

3. Para limpiar los archivos objeto de compilación previa:
   ```bash
   make clean
   ```

---

## 📦 Instalación en Nintendo 3DS

### Método 1: Envío Inalámbrico con `3dslink`
Si tu consola y tu PC están en la misma red Wi-Fi:
1. Abre el **Homebrew Launcher** en tu 3DS y presiona el botón **`Y`** para activar el receptor `3dslink`.
2. En tu computadora, ejecuta:
   ```bash
   3dslink tempad-3ds.3dsx
   ```

### Método 2: Emulador (Azahar / Citra)
* Simplemente arrastra y suelta el archivo `tempad-3ds.3dsx` dentro de la ventana de **Azahar** o **Citra**.

---

## 🏛️ Créditos y Agradecimientos

* **Time Variance Authority & Marvel Studios**: Inspiración de diseño e iconografía (*Loki*).
* **devkitPro team**: Por las herramientas y librerías `libctru`, `citro2d` y `citro3d`.
* Comunidad de desarrollo Homebrew de Nintendo 3DS.

