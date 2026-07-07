#include "config.h"

/* #define ENABLE_KDEBUG */

#if CONF_WITH_DDRAIGVGA_CONSOLE || CONF_WITH_DDRAIGVGA_DESKTOP

#include <stdbool.h>

#include "ddraig_vga.h"
#include "ddraigvdp.h"
#include "emutos.h"
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
    /* 640x480 1bpp bitmap mode: the standard framebuffer console and
     * the native VDI render directly into the VRAM window; scanout
     * starts at VRAM offset 0. */
    vdp_set_framebuffer_addr(0);
    vdp_set_bitmap_palette(0);
    /* ST mono convention: bit clear = paper, bit set = ink.  0xFFFF and
     * 0x0000 are white and black in any RGB packing. */
    vdp_write_palette_entry(0, 0xFFFF);
    vdp_write_palette_entry(1, 0x0000);
    /* clear to paper so we don't display VRAM garbage */
    memset((void *)ddraigvga_membase, 0, DRVGA_BITMAP_SIZE);
    drvga_write_control_reg(DISPMODE_BITMAPHIRES | DISP_DEPTH_1BPP);
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
