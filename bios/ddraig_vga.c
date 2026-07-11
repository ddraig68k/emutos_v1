#include "config.h"

/* #define ENABLE_KDEBUG */

#if CONF_WITH_DDRAIGVGA_CONSOLE || CONF_WITH_DDRAIGVGA_DESKTOP

#include <stdbool.h>

#include "ddraig_vga.h"
#include "ddraigvdp.h"
#include "emutos.h"
#include "biosdefs.h"
#include "lineavars.h"
#include "tosvars.h"
#include "string.h"

#define VGA_CHAR_WIDTH  80
#define VGA_CHAR_HEIGHT 30
#define DRVGA_BLANK_CELL vdp_make_text_cell(' ', VDP_TEXT_FG_WHITE, 0)

/* 640x480 at 1 bit per pixel */
#define DRVGA_BITMAP_SIZE (640L/8*480)

// Hard codeed for now, should be detected
uint32_t ddraigvga_base = 0xF7F500;
uint32_t ddraigvga_membase = 0xA00000;

void drvga_write_control_reg(uint16_t data)
{
    VDP_REG_WRITE(REG_CONTROL, data);
}

#if CONF_WITH_DDRAIGVGA_CONSOLE

uint16_t ddraigvga_screenbuf[DRVGA_TEXTBUF_SIZE];

void drvga_write_char(uint16_t address, uint16_t text)
{
    if (address >= DRVGA_TEXTBUF_SIZE) {
        return;
    }
    
    vdp_get_textmem_base()[address] = text;
    ddraigvga_screenbuf[address] = text;
}

uint16_t drvga_read_char(uint16_t address)
{
    if (address < DRVGA_TEXTBUF_SIZE) {
        return ddraigvga_screenbuf[address];
    } else {
        return 0;
    }
}

void drvga_copy_buffer(void)
{
    uint16_t i;
    uint16_t *textmem = vdp_get_textmem_base();

    for (i = 0; i < DRVGA_TEXTBUF_SIZE; i++)
    {
        textmem[i] = ddraigvga_screenbuf[i];
    }
}

void drvga_scroll_up(void)
{
    uint16_t *src = &ddraigvga_screenbuf[VGA_CHAR_WIDTH];
    uint16_t *dst = &ddraigvga_screenbuf[0];
    uint16_t i;

    for (i = 0; i < (VGA_CHAR_HEIGHT - 1) * 80; i++) {
        *dst++ = *src++;
    }
    // blank out last line
    dst = &ddraigvga_screenbuf[(VGA_CHAR_HEIGHT - 1) * VGA_CHAR_WIDTH];
    for (i = 0; i < VGA_CHAR_WIDTH; i++) {
        *dst++ = DRVGA_BLANK_CELL;
    }
    drvga_copy_buffer();
}

void drvga_scroll_down(void)
{
    uint16_t *src = &ddraigvga_screenbuf[(VGA_CHAR_HEIGHT - 2) * VGA_CHAR_WIDTH];
    uint16_t *dst = &ddraigvga_screenbuf[(VGA_CHAR_HEIGHT - 1) * VGA_CHAR_WIDTH];
    uint16_t i;

    // Move lines down
    for (i = 0; i < (VGA_CHAR_HEIGHT - 1) * VGA_CHAR_WIDTH; i++) {
        *dst-- = *src--;
    }
    // Blank out first line
    dst = &ddraigvga_screenbuf[0];
    for (i = 0; i < VGA_CHAR_WIDTH; i++) {
        *dst++ = DRVGA_BLANK_CELL;
    }
    drvga_copy_buffer();
}

#endif /* CONF_WITH_DDRAIGVGA_CONSOLE */

#if CONF_WITH_DDRAIGVGA_DESKTOP

/*
 * Native desktop video modes.  The table is indexed by the TOS 'rez'
 * value; modes with planes==0 are unsupported.  640x480x4 interleaved
 * planar is exactly the TT-medium layout, so it is reported as such.
 */
struct ddraig_mode {
    UWORD ctrl;         /* VDP control register value */
    UWORD planes;
    UWORD hz_rez;
    UWORD vt_rez;
};

static const struct ddraig_mode ddraig_mode_table[] = {
    { DISPMODE_BITMAP | DISP_DEPTH_PLANAR4, 4, 320, 240 },      /* 0: "ST low" */
    { 0, 0, 0, 0 },                                             /* 1: ST medium: no planar2 */
    { DISPMODE_BITMAPHIRES | DISP_DEPTH_1BPP, 1, 640, 480 },    /* 2: "ST high" */
    { 0, 0, 0, 0 },                                             /* 3: Falcon */
    { DISPMODE_BITMAPHIRES | DISP_DEPTH_PLANAR4, 4, 640, 480 }, /* 4: TT medium */
};

static WORD ddraig_cur_rez = ST_HIGH;

/* shadow of the ST(E)-format palette, for Setcolor() readback */
static UWORD ddraig_shadow_palette[16];

/* default 16-colour palette, ST format (same as Atari TOS) */
static const UWORD ddraig_default_palette[16] = {
    0x0777, 0x0700, 0x0070, 0x0770, 0x0007, 0x0707, 0x0077, 0x0555,
    0x0333, 0x0733, 0x0373, 0x0773, 0x0337, 0x0737, 0x0377, 0x0000
};

/*
 * Convert an ST(E) palette word to the card's 5:6:5 format.
 *
 * If any gun has its STE extension bit set, the word is decoded as STE
 * (4 bits per gun, bit 3 the lsb); otherwise as plain ST (3 bits per
 * gun, 7 = full intensity).  ST-format writers - which include our VDI,
 * since has_ste_shifter is FALSE - thus get the full brightness range.
 */
static UWORD stcol_to_565(UWORD col)
{
    UWORD r, g, b;

    if (col & 0x0888) {
        r = (col >> 8) & 0x0f; r = ((r & 7) << 1) | (r >> 3);
        g = (col >> 4) & 0x0f; g = ((g & 7) << 1) | (g >> 3);
        b = col & 0x0f;        b = ((b & 7) << 1) | (b >> 3);
        r = (r << 1) | (r >> 3);    /* 4 -> 5 bits */
        g = (g << 2) | (g >> 2);    /* 4 -> 6 bits */
        b = (b << 1) | (b >> 3);
    } else {
        r = (col >> 8) & 7;
        g = (col >> 4) & 7;
        b = col & 7;
        r = (r << 2) | (r >> 1);    /* 3 -> 5 bits */
        g = (g << 3) | g;           /* 3 -> 6 bits */
        b = (b << 2) | (b >> 1);
    }

    return (r << 11) | (g << 5) | b;
}

WORD ddraigvga_setcolor(WORD colorNum, WORD color)
{
    WORD old;

    colorNum &= 0x0f;
    old = ddraig_shadow_palette[colorNum];
    if (color == -1)
        return old;

    ddraig_shadow_palette[colorNum] = color & 0x0fff;
    vdp_write_palette_entry(colorNum, stcol_to_565(color));

    return old;
}

void ddraigvga_setpalette(const UWORD *palettePtr)
{
    WORD i;

    for (i = 0; i < 16; i++)
        ddraigvga_setcolor(i, palettePtr[i]);
}

BOOL ddraigvga_rez_supported(WORD rez)
{
    if (rez < 0 || rez >= (WORD)ARRAY_SIZE(ddraig_mode_table))
        return FALSE;

    return ddraig_mode_table[rez].planes != 0;
}

void ddraigvga_get_current_mode_info(UWORD *planes, UWORD *hz_rez, UWORD *vt_rez)
{
    const struct ddraig_mode *m = &ddraig_mode_table[ddraig_cur_rez];

    *planes = m->planes;
    *hz_rez = m->hz_rez;
    *vt_rez = m->vt_rez;
}

void ddraigvga_setrez(WORD rez)
{
    const struct ddraig_mode *m;

    if (rez < 0 || rez >= (WORD)ARRAY_SIZE(ddraig_mode_table))
        return;
    m = &ddraig_mode_table[rez];
    if (m->planes == 0)     /* unsupported mode: leave unchanged */
        return;

    ddraig_cur_rez = rez;
    sshiftmod = rez;

    if (m->planes == 1) {
        ddraigvga_setcolor(0, 0x0777);      /* paper white */
        ddraigvga_setcolor(1, 0x0000);      /* ink black */
    } else {
        ddraigvga_setpalette(ddraig_default_palette);
    }

    /* clear to colour 0 before switching so no stale VRAM is shown */
    memset((void *)ddraigvga_membase, 0,
           (LONG)m->hz_rez / 8 * m->vt_rez * m->planes);
    drvga_write_control_reg(m->ctrl);
}

#endif /* CONF_WITH_DDRAIGVGA_DESKTOP */

/* Sets system variables so that EmuTOS will use the graphics card */
static void init_system_vars(void)
{
    KDEBUG(("init_system_vars()\n"));
#if !CONF_WITH_DDRAIGVGA_DESKTOP
    /* Screen address: in desktop mode screen_init_address() points
     * v_bas_ad at the VRAM window (CONF_VRAM_ADDRESS) instead */
    v_bas_ad = 0;
#endif
    /* Fake 640x400x2 video mode (ST high) */
    sshiftmod = 2;

    /* Line A vars */
    /* Number of bitplanes */
    v_planes = 1;
    /* Bytes per scan-line */
    BYTES_LIN = 80;
    v_lin_wr = 80;
    /* Vertical resolution */
    V_REZ_VT = 480;
    /* Horizontal resolution */
    V_REZ_HZ = 640;
}

void ddraigvga_screen_init(void)
{
    KDEBUG(("ddraigvga_screen_init()\n"));

    vdp_init(ddraigvga_base, ddraigvga_membase);

#if CONF_WITH_DDRAIGVGA_DESKTOP
    /* Native desktop: the standard framebuffer console and the native
     * VDI render directly into the VRAM window; scanout starts at VRAM
     * offset 0.  Boot in mono 640x480; the desktop can switch modes
     * later via Setscreen()/the resolution dialog. */
    vdp_set_framebuffer_addr(0);
    vdp_set_bitmap_palette(0);
    ddraigvga_setrez(ST_HIGH);
#else
    {
        int i;

        vdp_set_text_base(0);

        drvga_write_control_reg(DISPMODE_TEXT);
        for (i = 0; i < DRVGA_TEXTBUF_SIZE; i++) {
            ddraigvga_screenbuf[i] = DRVGA_BLANK_CELL;
        }
        drvga_copy_buffer();
    }
#endif

    init_system_vars();

    // Enable the VBL interrupt
    VDP_REG_WRITE(REG_INTERRUPT, 0x0001);
}

#endif
