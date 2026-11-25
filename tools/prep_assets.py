#!/usr/bin/env python3
"""
Preprocesamiento de assets visuales para TemPad 3DS (pipeline tex3ds / citro2d).

Lee los PNG/JPG originales desde:
  - assets_raw/ui/ o assets_raw/ (logo TVA, 5 glifos, 4 iconos del Dashboard)
  - assets_raw/variants/, variants/ o assets_raw/ (10 fotos de variantes)

Genera los PNG finales optimizados para Nintendo 3DS en:
  - gfx/ui/
  - gfx/variants/
"""

import os
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageEnhance, ImageFilter, ImageOps

ROOT_DIR = Path(__file__).resolve().parent.parent
OUT_UI_DIR = ROOT_DIR / "gfx" / "ui"
OUT_VAR_DIR = ROOT_DIR / "gfx" / "variants"

UI_SEARCH_DIRS = [
    ROOT_DIR / "assets_raw" / "ui",
    ROOT_DIR / "assets_raw",
]

VARIANT_SEARCH_DIRS = [
    ROOT_DIR / "assets_raw" / "variants",
    ROOT_DIR / "variants",
    ROOT_DIR / "assets_raw",
]

# Mismo orden exacto que g_variantCatalog[10] en include/tva_catalog.h
VARIANTS = [
    ("var_loki",    ["var_loki"]),
    ("var_sylvie",  ["var_sylvie"]),
    ("var_gator",   ["var_gator"]),
    ("var_classic", ["var_classic"]),
    ("var_timely",  ["var_timely", "var_timeley"]),
    ("var_wanda",   ["var_wanda"]),
    ("var_wade",    ["var_wade"]),
    ("var_peter",   ["var_peter"]),
    ("var_strange", ["var_strange"]),
    ("var_magneto", ["var_magneto"]),
]

DASHBOARD_ICONS = [
    ("icon_timedoor",    ["icon_timedoor"]),
    ("icon_missminutes", ["icon_missminutes", "icon_miss_minutes"]),
    ("icon_settings",    ["icon_settings"]),
    ("icon_directory",   ["icon_directory"]),
]

GLYPHS = [
    ("glyph_1", ["glyph_1"]),
    ("glyph_2", ["glyph_2"]),
    ("glyph_3", ["glyph_3"]),
    ("glyph_4", ["glyph_4"]),
    ("glyph_5", ["glyph_5"]),
]

# Paleta duotono ambar CRT de la TVA
COLOR_BLACK = (10, 4, 0)      # #0A0400
COLOR_MID   = (217, 107, 0)   # #D96B00
COLOR_HIGH  = (255, 184, 51)  # #FFB833

# Colores pre-pintados para sprites de UI (sin depender de tinte por hardware)
COLOR_TVA_LOGO_RGB      = (225, 110, 20)
COLOR_ICON_DIM_RGB      = (175, 82, 16)    # Estado inactivo (naranja medio/apagado)
COLOR_ICON_GLOW_FAR_RGB = (255, 110, 10)   # Capa inferior glow amplio (radius=4)
COLOR_ICON_GLOW_MID_RGB = (255, 145, 25)   # Capa media glow cercano (radius=2)
COLOR_ICON_GLOW_TOP_RGB = (255, 215, 115)  # Nucleo brillante ambar luminoso

VARIANT_SIZE        = (94, 98)
ICON_INNER_SIZE     = (46, 46)
ICON_MM_INNER_SIZE  = (58, 52)  # Tamano visual compensado para Miss Minutes (~1.3x)
ICON_CANVAS_SIZE    = (70, 70)  # Lienzo comun con margen amplio para el halo de glow
GLYPH_SIZE          = (12, 12)
TVA_LOGO_SIZE       = (44, 20)


def find_source_image(search_dirs, stem_candidates):
    """Busca una imagen (.png/.jpg/.jpeg/.webp) de forma insensible a mayusculas."""
    valid_exts = {".png", ".jpg", ".jpeg", ".webp"}
    wanted = {s.lower() for s in stem_candidates}

    for folder in search_dirs:
        if not folder.is_dir():
            continue
        for entry in folder.iterdir():
            if entry.is_file() and entry.suffix.lower() in valid_exts:
                if entry.stem.lower() in wanted:
                    return entry
    return None


def build_tva_duotone_lut():
    """Precalcula tabla de 256 valores RGB para duotono ambar CRT."""
    lut = []
    for i in range(256):
        if i <= 128:
            t = i / 128.0
            r = int(round(COLOR_BLACK[0] + (COLOR_MID[0] - COLOR_BLACK[0]) * t))
            g = int(round(COLOR_BLACK[1] + (COLOR_MID[1] - COLOR_BLACK[1]) * t))
            b = int(round(COLOR_BLACK[2] + (COLOR_MID[2] - COLOR_BLACK[2]) * t))
        else:
            t = (i - 128) / 127.0
            r = int(round(COLOR_MID[0] + (COLOR_HIGH[0] - COLOR_MID[0]) * t))
            g = int(round(COLOR_MID[1] + (COLOR_HIGH[1] - COLOR_MID[1]) * t))
            b = int(round(COLOR_MID[2] + (COLOR_HIGH[2] - COLOR_MID[2]) * t))
        lut.append((r, g, b))
    return lut


TVA_LUT = build_tva_duotone_lut()


def make_variant_placeholder(label):
    """Genera un retrato placeholder estilo CRT ambar si falta el archivo fuente."""
    w, h = VARIANT_SIZE
    img = Image.new("RGBA", (w, h), (*COLOR_BLACK, 255))
    draw = ImageDraw.Draw(img)
    draw.rectangle([4, 4, w - 5, h - 5], outline=(*COLOR_MID, 255), width=1)
    # Silueta simple centrada
    draw.ellipse([w // 2 - 14, 18, w // 2 + 14, 48], fill=(95, 48, 6, 255), outline=(*COLOR_HIGH, 255))
    draw.rounded_rectangle([w // 2 - 26, 52, w // 2 + 26, h - 10], radius=10, fill=(95, 48, 6, 255), outline=(*COLOR_HIGH, 255))
    return img


def process_variant_photo(src_path, out_path, name):
    """Recorta a 94x98 (aspect fill), convierte a duotono ambar CRT y agrega scanlines sutiles."""
    if src_path is None:
        print(f"  [WARN] Falta fuente para {name}; generando placeholder.")
        out_img = make_variant_placeholder(name)
        out_img.save(out_path, "PNG")
        return

    with Image.open(src_path) as raw:
        rgb = raw.convert("RGB")
        # Centrado ligeramente hacia arriba (0.25) para encuadrar bien rostros
        fitted = ImageOps.fit(rgb, VARIANT_SIZE, method=Image.Resampling.LANCZOS, centering=(0.5, 0.25))
        gray = ImageOps.grayscale(fitted)
        gray = ImageOps.autocontrast(gray, cutoff=2)
        gray = ImageEnhance.Contrast(gray).enhance(1.22)

        w, h = gray.size
        out = Image.new("RGBA", (w, h))
        src_px = gray.load()
        dst_px = out.load()

        for y in range(h):
            # Scanlines CRT sutiles en filas impares
            scan_mul = 0.87 if (y % 2 == 1) else 1.0
            for x in range(w):
                r, g, b = TVA_LUT[src_px[x, y]]
                dst_px[x, y] = (
                    int(r * scan_mul),
                    int(g * scan_mul),
                    int(b * scan_mul),
                    255,
                )

        out.save(out_path, "PNG")
        print(f"  [OK] {name}: {src_path.name} -> {out_path.relative_to(ROOT_DIR)} ({w}x{h})")


def resize_contain_rgba(img_rgba, target_size):
    """Recorta margenes transparentes (alpha > 15), redimensiona manteniendo proporcion y centra."""
    tw, th = target_size
    # Auto-recorte estricto de bordes transparentes usando pixeles visibles (alpha > 15)
    vis_mask = img_rgba.getchannel("A").point(lambda a: 255 if a > 15 else 0)
    bbox = vis_mask.getbbox()
    if bbox:
        img_rgba = img_rgba.crop(bbox)

    src_w, src_h = img_rgba.size
    if src_w == 0 or src_h == 0:
        return Image.new("RGBA", target_size, (0, 0, 0, 0))

    scale = min(tw / src_w, th / src_h)
    new_w = max(1, int(round(src_w * scale)))
    new_h = max(1, int(round(src_h * scale)))

    resized = img_rgba.resize((new_w, new_h), Image.Resampling.LANCZOS)
    canvas = Image.new("RGBA", target_size, (0, 0, 0, 0))
    offset_x = (tw - new_w) // 2
    offset_y = (th - new_h) // 2
    canvas.paste(resized, (offset_x, offset_y), resized)
    return canvas


def extract_alpha_mask(img_rgba):
    """Extrae una mascara alfa limpia si la imagen viene con fondo opaco."""
    w, h = img_rgba.size
    alpha = img_rgba.getchannel("A")
    min_a, _ = alpha.getextrema()
    if min_a < 245:
        return img_rgba

    # Sin canal alfa transparente: inferir fondo por las esquinas
    gray = ImageOps.grayscale(img_rgba.convert("RGB"))
    corners = [
        gray.getpixel((0, 0)),
        gray.getpixel((w - 1, 0)),
        gray.getpixel((0, h - 1)),
        gray.getpixel((w - 1, h - 1)),
    ]
    avg_corner = sum(corners) / 4.0

    new_alpha = Image.new("L", (w, h))
    g_px = gray.load()
    a_px = new_alpha.load()

    if avg_corner > 180:
        # Fondo claro -> trazos oscuros visibles
        for y in range(h):
            for x in range(w):
                val = 255 - g_px[x, y]
                a_px[x, y] = 0 if val < 24 else min(255, int(val * 1.15))
    else:
        # Fondo oscuro -> trazos claros visibles
        for y in range(h):
            for x in range(w):
                val = g_px[x, y]
                a_px[x, y] = 0 if val < 24 else min(255, int(val * 1.15))

    out = img_rgba.copy()
    out.putalpha(new_alpha)
    return out


def make_glow_icon_layers(padded_rgba, wide_radius=4, mid_radius=2, alpha_boost=1.85):
    """Compone version _dim y version _glow (3 capas con GaussianBlur) en lienzo 70x70."""
    # Mascara alfa reforzada del trazo nitido
    base_alpha = padded_rgba.getchannel("A").point(lambda a: min(255, int(a * alpha_boost)) if a > 12 else 0)

    # 1. Version inactiva (_dim): trazo limpio naranja apagado
    dim_img = Image.new("RGBA", ICON_CANVAS_SIZE, (*COLOR_ICON_DIM_RGB, 255))
    dim_img.putalpha(base_alpha)

    # 2. Version seleccionada (_glow): 3 capas (halo amplio + halo cercano + nucleo brillante)
    # Capa inferior: halo amplio
    wide_layer = Image.new("RGBA", ICON_CANVAS_SIZE, (*COLOR_ICON_GLOW_FAR_RGB, 255))
    wide_layer.putalpha(base_alpha)
    wide_blur = wide_layer.filter(ImageFilter.GaussianBlur(radius=wide_radius))
    wide_a = wide_blur.getchannel("A").point(lambda a: min(155, int(a * 2.2)))
    wide_blur.putalpha(wide_a)

    # Capa media: halo cercano
    mid_layer = Image.new("RGBA", ICON_CANVAS_SIZE, (*COLOR_ICON_GLOW_MID_RGB, 255))
    mid_layer.putalpha(base_alpha)
    mid_blur = mid_layer.filter(ImageFilter.GaussianBlur(radius=mid_radius))
    mid_a = mid_blur.getchannel("A").point(lambda a: min(210, int(a * 1.65)))
    mid_blur.putalpha(mid_a)

    # Capa superior: nucleo brillante nitido en ambar luminoso
    core_layer = Image.new("RGBA", ICON_CANVAS_SIZE, (*COLOR_ICON_GLOW_TOP_RGB, 255))
    core_layer.putalpha(base_alpha)

    glow_img = Image.alpha_composite(wide_blur, mid_blur)
    glow_img = Image.alpha_composite(glow_img, core_layer)
    return dim_img, glow_img


def process_dashboard_icon_variants(src_path, out_dim_path, out_glow_path, name):
    """Genera las dos variantes 70x70 (_dim y _glow), compensando Miss Minutes a 58x52 con halo mas nitido."""
    is_miss_minutes = (name == "icon_missminutes")
    inner_size = ICON_MM_INNER_SIZE if is_miss_minutes else ICON_INNER_SIZE
    wide_radius = 3 if is_miss_minutes else 4
    mid_radius = 1.5 if is_miss_minutes else 2
    alpha_boost = 2.05 if is_miss_minutes else 1.85

    if src_path is None:
        print(f"  [WARN] Falta fuente para {name}; generando placeholders _dim y _glow.")
        placeholder = Image.new("RGBA", ICON_CANVAS_SIZE, (0, 0, 0, 0))
        d = ImageDraw.Draw(placeholder)
        d.rectangle([12, 12, ICON_CANVAS_SIZE[0] - 13, ICON_CANVAS_SIZE[1] - 13], outline=(255, 255, 255, 255), width=2)
        dim_img, glow_img = make_glow_icon_layers(placeholder, wide_radius=wide_radius, mid_radius=mid_radius)
        dim_img.save(out_dim_path, "PNG")
        glow_img.save(out_glow_path, "PNG")
        return

    with Image.open(src_path) as raw:
        rgba = extract_alpha_mask(raw.convert("RGBA"))
        inner = resize_contain_rgba(rgba, inner_size)

        # Centrar el icono redimensionado dentro del lienzo comun 70x70 para no cortar el glow
        padded = Image.new("RGBA", ICON_CANVAS_SIZE, (0, 0, 0, 0))
        pad_x = (ICON_CANVAS_SIZE[0] - inner_size[0]) // 2
        pad_y = (ICON_CANVAS_SIZE[1] - inner_size[1]) // 2
        padded.paste(inner, (pad_x, pad_y), inner)

        dim_img, glow_img = make_glow_icon_layers(
            padded, wide_radius=wide_radius, mid_radius=mid_radius, alpha_boost=alpha_boost
        )
        dim_img.save(out_dim_path, "PNG")
        glow_img.save(out_glow_path, "PNG")
        print(
            f"  [OK] {name}: {src_path.name} -> "
            f"{out_dim_path.name} & {out_glow_path.name} "
            f"(inner {inner_size[0]}x{inner_size[1]} en lienzo {ICON_CANVAS_SIZE[0]}x{ICON_CANVAS_SIZE[1]})"
        )


def process_ui_sprite(src_path, out_path, name, target_size):
    """Prepara glifo lateral preservando proporcion y transparencia."""
    if src_path is None:
        print(f"  [WARN] Falta fuente para {name}; generando placeholder.")
        img = Image.new("RGBA", target_size, (0, 0, 0, 0))
        d = ImageDraw.Draw(img)
        d.rectangle([1, 1, target_size[0] - 2, target_size[1] - 2], outline=(*COLOR_TVA_LOGO_RGB, 255), width=1)
        img.save(out_path, "PNG")
        return

    with Image.open(src_path) as raw:
        rgba = extract_alpha_mask(raw.convert("RGBA"))
        fitted = resize_contain_rgba(rgba, target_size)
        fitted.save(out_path, "PNG")
        print(f"  [OK] {name}: {src_path.name} -> {out_path.relative_to(ROOT_DIR)} ({target_size[0]}x{target_size[1]})")


def process_tva_logo(src_path, out_path, target_size):
    """Pinta la silueta del logo TVA directamente en naranja TVA (225, 110, 20) con alfa limpio 0..255."""
    if src_path is None:
        print("  [WARN] Falta fuente para tva_logo; generando placeholder.")
        img = Image.new("RGBA", target_size, (0, 0, 0, 0))
        d = ImageDraw.Draw(img)
        d.rectangle([2, 2, target_size[0] - 3, target_size[1] - 3], outline=(*COLOR_TVA_LOGO_RGB, 255), width=2)
        img.save(out_path, "PNG")
        return

    with Image.open(src_path) as raw:
        rgba = extract_alpha_mask(raw.convert("RGBA"))
        alpha = rgba.getchannel("A")
        _, max_a = alpha.getextrema()
        floor_a = 35 if max_a > 60 else 0
        span = max(1, max_a - floor_a)
        norm_alpha = alpha.point(lambda a: 0 if a <= floor_a else min(255, int(round((a - floor_a) * 255.0 / span))))
        cleaned = Image.new("RGBA", rgba.size, (*COLOR_TVA_LOGO_RGB, 255))
        cleaned.putalpha(norm_alpha)

        fitted = resize_contain_rgba(cleaned, target_size)
        final_a = fitted.getchannel("A").point(lambda a: min(255, int(a * 1.25)) if a > 10 else 0)
        out = Image.new("RGBA", target_size, (*COLOR_TVA_LOGO_RGB, 255))
        out.putalpha(final_a)
        out.save(out_path, "PNG")
        print(f"  [OK] tva_logo: {src_path.name} -> {out_path.relative_to(ROOT_DIR)} ({target_size[0]}x{target_size[1]}, RGB{COLOR_TVA_LOGO_RGB})")


def main():
    OUT_UI_DIR.mkdir(parents=True, exist_ok=True)
    OUT_VAR_DIR.mkdir(parents=True, exist_ok=True)

    print("=== 1. Procesando Logo TVA (naranja pre-coloreado) y 5 Glifos ===")
    tva_src = find_source_image(UI_SEARCH_DIRS, ["tva_logo"])
    process_tva_logo(tva_src, OUT_UI_DIR / "tva_logo.png", TVA_LOGO_SIZE)

    for out_stem, candidates in GLYPHS:
        src = find_source_image(UI_SEARCH_DIRS, candidates)
        process_ui_sprite(src, OUT_UI_DIR / f"{out_stem}.png", out_stem, GLYPH_SIZE)

    print("\n=== 2. Procesando 4 Iconos del Dashboard (58x58 _dim y _glow) ===")
    for out_stem, candidates in DASHBOARD_ICONS:
        src = find_source_image(UI_SEARCH_DIRS, candidates)
        process_dashboard_icon_variants(
            src,
            OUT_UI_DIR / f"{out_stem}_dim.png",
            OUT_UI_DIR / f"{out_stem}_glow.png",
            out_stem,
        )

    print("\n=== 3. Procesando 10 Fotos de Variantes (duotono ambar CRT 94x98) ===")
    for out_stem, candidates in VARIANTS:
        src = find_source_image(VARIANT_SEARCH_DIRS, candidates)
        process_variant_photo(src, OUT_VAR_DIR / f"{out_stem}.png", out_stem)

    print("\nPreprocesamiento completado.")


if __name__ == "__main__":
    main()
