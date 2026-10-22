#ifndef TVA_CATALOG_H
#define TVA_CATALOG_H

// =============================================================================
// TVA_CATALOG.H — CATALOGO DE VARIANTES Y DESTINOS TEMPORALES (ESTILO "JSON")
// =============================================================================
// Edita este archivo para modificar o agregar variantes del multiverso, fechas,
// coordenadas y rutas de texturas (.t3x en romfs) para futuras versiones.
// =============================================================================

// -----------------------------------------------------------------------------
// 1. ESTRUCTURA Y CATALOGO DE VARIANTES (PANTALLA SUPERIOR)
// -----------------------------------------------------------------------------
typedef struct {
    const char *variant_id;  // Codigo de variante (ej. "L1130")
    const char *name;        // Nombre completo (ej. "LOKI LAUFEYSON")
    int year;                // Anio del evento Nexus (ordena el eje X cronologicamente)
    const char *date_str;    // Fecha formateada para barra inferior (ej. "05.04.2012")
    const char *coords_str;  // Coordenadas temporales para barra inferior
    const char *image_src;   // Ruta de textura en romfs (ej. "romfs:/gfx/var_loki.t3x")
} VariantProfile;

#define VARIANT_CATALOG_COUNT 10

static const VariantProfile g_variantCatalog[VARIANT_CATALOG_COUNT] = {
    {
        "L1130",
        "LOKI LAUFEYSON",
        2012,
        "05.04.2012",
        "0732.443 // 0538.133",
        "romfs:/gfx/var_loki.t3x"
    },
    {
        "L0852",
        "SYLVIE LAUFEYDOTTIR",
        2050,
        "14.08.2050",
        "0891.112 // 0412.889",
        "romfs:/gfx/var_sylvie.t3x"
    },
    {
        "L6792",
        "ALLIGATOR LOKI",
        1985,
        "21.10.1985",
        "0311.750 // 0198.404",
        "romfs:/gfx/var_gator.t3x"
    },
    {
        "L1190",
        "CLASSIC LOKI",
        2018,
        "09.11.2018",
        "0654.221 // 0901.772",
        "romfs:/gfx/var_classic.t3x"
    },
    {
        "K0001",
        "VICTOR TIMELY",
        1893,
        "01.05.1893",
        "0104.883 // 0042.616",
        "romfs:/gfx/var_timely.t3x"
    },
    {
        "W1999",
        "WANDA MAXIMOFF",
        2023,
        "18.11.2023",
        "0812.616 // 0555.901",
        "romfs:/gfx/var_wanda.t3x"
    },
    {
        "DP300",
        "WADE WILSON",
        2024,
        "26.07.2024",
        "1005.209 // 0838.411",
        "romfs:/gfx/var_wade.t3x"
    },
    {
        "S1610",
        "PETER PARKER",
        2016,
        "12.06.2016",
        "0616.101 // 0404.212",
        "romfs:/gfx/var_peter.t3x"
    },
    {
        "T8380",
        "STEPHEN STRANGE",
        1978,
        "03.09.1978",
        "0245.990 // 0310.118",
        "romfs:/gfx/var_strange.t3x"
    },
    {
        "X1997",
        "ERIK LEHNSHERR",
        1997,
        "19.02.1997",
        "0492.301 // 0711.097",
        "romfs:/gfx/var_magneto.t3x"
    }
};

// -----------------------------------------------------------------------------
// 2. ESTRUCTURA Y CATALOGO DE DESTINOS TEMPORALES ("SAVED TIMES" / TIMEDOOR)
// -----------------------------------------------------------------------------
typedef struct Location {
    const char *name;        // Nombre del destino (ej. "2012, EARTH")
    const char *coords;      // Coordenadas de salto temporal
    const char *era;         // Descripcion del sector / evento
    const char *image_src;   // Ruta de textura de previsualizacion en romfs
} Location;

static const Location g_timedoorLocations[] = {
    {
        "1705, LAMENTIS",
        "0742.11.LMT1.9021",
        "SECTOR C-2 // EXTINCTION",
        "romfs:/gfx/loc_lamentis.t3x"
    },
    {
        "2012, EARTH",
        "1255.92.FGV3.8534",
        "NEW YORK // BATTLE OF NY",
        "romfs:/gfx/loc_earth2012.t3x"
    },
    {
        "1412, TITAN",
        "0412.88.TTN9.1102",
        "GALAXY M-31 // PRE-COLLAPSE",
        "romfs:/gfx/loc_titan.t3x"
    },
    {
        "2077, ASGARD",
        "2077.01.ASG4.6549",
        "GOLDEN REALM // CATACLYSM",
        "romfs:/gfx/loc_asgard.t3x"
    }
};

#define TIMEDOOR_LOCATION_COUNT ((int)(sizeof(g_timedoorLocations) / sizeof(g_timedoorLocations[0])))

#endif // TVA_CATALOG_H

