#ifndef _DDRAIG_VGA_H_
#define _DDRAIG_VGA_H_

#define DRVGA_TEXTBUF_SIZE  2400

#include <stdint.h>
#include "portab.h"

extern uint16_t ddraigvga_screenbuf[DRVGA_TEXTBUF_SIZE];

void drvga_write_control_reg(uint16_t data);
void drvga_write_char(uint16_t address, uint16_t text);
uint16_t drvga_read_char(uint16_t address);
void drvga_copy_buffer(void);
void drvga_scroll_up(void);
void drvga_scroll_down(void);
void ddraigvga_screen_init(void);

/* native desktop (CONF_WITH_DDRAIGVGA_DESKTOP) mode/palette interface */
BOOL ddraigvga_rez_supported(WORD rez);
void ddraigvga_setrez(WORD rez);
WORD ddraigvga_setcolor(WORD colorNum, WORD color);
void ddraigvga_setpalette(const UWORD *palettePtr);
void ddraigvga_get_current_mode_info(UWORD *planes, UWORD *hz_rez, UWORD *vt_rez);

#endif
