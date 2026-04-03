#ifndef _DDRAIGVDP_H_
#define _DDRAIGVDP_H_

#include <stdint.h>
#include "portab.h"

extern uint32_t g_vdp_reg_base;
extern uint16_t *g_vdp_memory_base;

#define VDP_TEXTBUF_SIZE  4800  /* Max of 80x60*/

#define VDP_REG_WRITE(x, y)     (*((volatile uint16_t *) (g_vdp_reg_base + (x))) = (y))
#define VDP_REG_READ(x)         (*((volatile uint16_t *) (g_vdp_reg_base + (x))))

#define	REG_STATUS              0x00    // Status register
#define REG_CONTROL             0x02    // Display control register
#define REG_INTERRUPT           0x04    // Interrupt control/ status
#define REG_RAMPAGE             0x06    // SRAM Page control

#define REG_FRAMEBUFFER_L		0x10    // Bitmap data address location (L) 
#define REG_FRAMEBUFFER_H		0x12    // Bitmap data address location (H)
#define REG_TEXT_BASE_L		    0x14    /* Text base address (L) */
#define REG_TEXT_BASE_H		    0x16    /* Text base address (H) */

#define REG_LINESCR_DATA_L		0x18    // Tile scroll table data address location (L) 
#define REG_LINESCR_DATA_H		0x1A    // Tile scroll table tile data address location (H)

#define REG_TEXT_ADDR		    0x20    // Text buffer index
#define REG_TEXT_DATA		    0x22    // Text buffer data
#define REG_CURSOR_POS		    0x24    // Cursor position Y,X
#define REG_CURSOR_SIZE		    0x26    // Cursor size bottom,top

#define REG_PALETTE_IDX         0x28    // Palette index register
#define REG_PALETTE_DATA        0x2A    // Palette data register
#define REG_PALETTE_CTRL0       0x2C    // Palette control register (Text and Bitmap)
#define REG_PALETTE_CTRL1       0x2E    // Palette control register (Tiles and Sprites)

#define	REG_COMMAND             0x30    // Command control register
#define REG_DRAW_BASE_L		    0x32    // Draw base address (L)
#define REG_DRAW_BASE_H		    0x34    // Draw base address (H)
#define	REG_DRAW_COLOR0         0x36    // Draw color 0
#define	REG_DRAW_COLOR1         0x38    // Draw color 1
#define	REG_DRAW_MODE           0x3A    // Draw mode, solid, xor, ...
#define	REG_DRAW_PATTERN        0x3C    // Draw pattern index/data

#define	REG_PARAM_DATA0         0x40    // Command parameter 0
#define	REG_PARAM_DATA1         0x42    // Command parameter 1
#define	REG_PARAM_DATA2         0x44    // Command parameter 2
#define	REG_PARAM_DATA3         0x46    // Command parameter 3
#define	REG_PARAM_DATA4         0x48    // Command parameter 4
#define	REG_PARAM_DATA5         0x4A    // Command parameter 5
#define	REG_PARAM_DATA6         0x4C    // Command parameter 6

#define REG_TILE1_CTRL		    0x50    // Tilemap 0 control register
#define REG_TILE1_DATA_L        0x52    // Tilemap 0 16x16 tile data address register (L)
#define REG_TILE1_DATA_H        0x54    // Tilemap 0 16x16 tile data address register (H)
#define REG_TILE1_MAP_L		    0x56    // Tilemap 0 map data address (L)
#define REG_TILE1_MAP_H		    0x58    // Tilemap 0 map data address (H)
#define REG_TILE1_SCROLL_X      0x5A    // Tilemap 0 horizontal scroll position
#define REG_TILE1_SCROLL_Y	    0x5C    // Tilemap 0 vertical scroll position

#define REG_TILE2_CTRL		    0x60    // Tilemap 1 control register
#define REG_TILE2_DATA_L        0x62    // Tilemap 1 16x16 tile data address register (L)
#define REG_TILE2_DATA_H        0x64    // Tilemap 1 16x16 tile data address register (H)
#define REG_TILE2_MAP_L		    0x66    // Tilemap 1 map data address (L)
#define REG_TILE2_MAP_H		    0x68    // Tilemap 1 map data address (H)
#define REG_TILE2_SCROLL_X      0x6A    // Tilemap 1 horizontal scroll position
#define REG_TILE2_SCROLL_Y	    0x6C    // Tilemap 1 vertical scroll position

#define REG_TILE3_CTRL		    0x70    // Tilemap 2 control register
#define REG_TILE3_DATA_L        0x72    // Tilemap 2 8x8 tile data address register (L)
#define REG_TILE3_DATA_H        0x74    // Tilemap 2 8x8 tile data address register (H)
#define REG_TILE3_MAP_L		    0x76    // Tilemap 2 map data address (L)
#define REG_TILE3_MAP_H		    0x78    // Tilemap 2 map data address (H)
#define REG_TILE3_SCROLL_X      0x7A    // Tilemap 2 horizontal scroll position
#define REG_TILE3_SCROLL_Y	    0x7C    // Tilemap 2 vertical scroll position

#define REG_TILE4_CTRL		    0x80    // Tilemap 3 control register
#define REG_TILE4_DATA_L        0x82    // Tilemap 3 8x8 tile data address register (L)
#define REG_TILE4_DATA_H        0x84    // Tilemap 3 8x8 tile data address register (H)
#define REG_TILE4_MAP_L		    0x86    // Tilemap 3 map data address (L)
#define REG_TILE4_MAP_H		    0x88    // Tilemap 3 map data address (H)
#define REG_TILE4_SCROLL_X      0x8A    // Tilemap 3 horizontal scroll position
#define REG_TILE4_SCROLL_Y	    0x8C    // Tilemap 3 vertical scroll position

#define REG_SPRITE_DATA_L		0x90    // Sprite data address location (L) 
#define REG_SPRITE_DATA_H		0x92    // Sprite data address location (H)
#define REG_SPRITE_IDX          0x94
#define REG_SPRITE_POS_X        0x96
#define REG_SPRITE_POS_Y        0x98
#define REG_SPRITE_TILE         0x9A
#define REG_SPRITE_ATTRIB       0x9C

#define REG_FONT_ADDR		    0xA0    // Pattern address index 
#define REG_FONT_DATA		    0xA2    // Pattern data

#define REG_PATTERN_ADDR		0xA4    // Pattern address index 
#define REG_PATTERN_DATA		0xA6    // Pattern data

#define REG_INT_LINE_Y          0xA8    // Line interrupt control value

#define CMD_NONE                0x00   
#define CMD_FILL_RECT           0x01
#define CMD_DRAW_LINE           0x02
#define CMD_DRAW_VLINE          0x03
#define CMD_DRAW_HLINE          0x04
#define CMD_FILL_TRIANGLE       0x05
#define CMD_MEM_COPY_LINEAR     0x06
#define CMD_MEM_COPY_2D         0x07
#define CMD_MEM_FILL_LINEAR     0x08

#define DRAW_MODE_SOLID        0x0
#define DRAW_MODE_XOR          0x1
#define DRAW_MODE_TRANS        0x2
#define DRAW_MODE_REVTRANS     0x3

// Status register masks
#define STATUS_READY            0x0001
#define STATUS_ERROR            0x0002
#define STATUS_HSYNC            0x0004
#define STATUS_VSYNC            0x0008

// Control register settings
// Display modes
#define DISPMODE_TEXT			0x0000      // Text mode, default
#define DISPMODE_BITMAP			0x0001      // Bitmap 320x240
#define DISPMODE_BITMAPHIRES	0x0002      // Bitmap 640x480
#define DISPMODE_TILEMAP	    0x0003      // Tile layer (320x240)
// Bitmap mode bits per pixel
#define DISP_DEPTH_RGB 		    0x0000
#define DISP_DEPTH_8BPP			0x0008
#define DISP_DEPTH_4BPP			0x0009
#define DISP_DEPTH_2BPP			0x000A
#define DISP_DEPTH_1BPP			0x000B
// Text control 
#define ENABLE_CURSOR           0x0100
#define ENABLE_BLINK            0x0200
#define TEXT_MODE_60            0x0400
#define SPRITE_ENABLE           0x0800
#define SCANLINE_NONE           0x0000
#define SCANLINE_BLACK          0x1000
#define SCANLINE_GREY           0x2000
#define SCANLINE_COLOR          0x3000
#define TEXT_OVERLAY            0x4000

#define TILEMAP1                0x00
#define TILEMAP2                0x01
#define TILEMAP3                0x02
#define TILEMAP4                0x03

#define TILEMAP1_WIDTH          32
#define TILEMAP1_HEIGHT         32
#define TILEMAP2_WIDTH          32
#define TILEMAP2_HEIGHT         32
#define TILEMAP3_WIDTH          64
#define TILEMAP3_HEIGHT         64
#define TILEMAP4_WIDTH          64
#define TILEMAP4_HEIGHT         64

#define MAX_SPRITES             256

typedef struct tile_layer 
{
    uint8_t id;
    uint8_t enabled;
    uint16_t map_width;
    uint16_t map_height;
    uint32_t map_addr;
    uint32_t data_addr;
} tile_layer_t;

void vdp_init(uint32_t regaddr, uint32_t memaddr);

// Control registers
uint16_t vdp_get_status(void);
void vdp_set_control(uint16_t mode);
uint16_t vdp_get_control(void);
void vdp_set_int_control(uint16_t intr);
uint16_t vdp_get_int_control(void);
void vdp_set_mempage(uint16_t page);
uint16_t vdp_get_mempage(void);

void vdp_wait_busy(void);
void vdp_wait_vblank(void);
void vdp_wait_vblank_clear(void);

static inline uint32_t vdp_get_regbase_addr(void)
{
    return g_vdp_reg_base;
}

static inline uint16_t *vdp_get_memory_base(void)
{
    return g_vdp_memory_base;
}

// Framebuffer pointer
void vdp_set_framebuffer_addr(uint32_t addr);
uint32_t vdp_get_framebuffer_addr(void);

// Line scroll pointer
void vdp_set_linescroll_addr(uint32_t addr);
uint32_t vdp_get_linescroll_addr(void);

// Text attributes
#define VDP_TEXT_FG_BLACK           0x0000
#define VDP_TEXT_FG_BLUE            0x0100
#define VDP_TEXT_FG_GREEN           0x0200
#define VDP_TEXT_FG_CYAN            0x0300
#define VDP_TEXT_FG_RED             0x0400
#define VDP_TEXT_FG_MAGENTA         0x0500
#define VDP_TEXT_FG_BROWN           0x0600
#define VDP_TEXT_FG_LIGHTGRAY       0x0700
#define VDP_TEXT_FG_GRAY            0x0800
#define VDP_TEXT_FG_LIGHTBLUE       0x0900
#define VDP_TEXT_FG_LIGHTGREEN      0x0A00
#define VDP_TEXT_FG_LIGHTCYAN       0x0B00
#define VDP_TEXT_FG_LIGHTRED        0x0C00
#define VDP_TEXT_FG_LIGHTMAGENTA    0x0D00
#define VDP_TEXT_FG_YELLOW          0x0E00
#define VDP_TEXT_FG_WHITE           0x0F00

static inline uint16_t vdp_make_text_cell(uint8_t ch, uint16_t fg, uint16_t bg)
{
    return (uint16_t)(ch | fg | bg);
}

// Text commands
void vdp_clear_text(void);
void vdp_write_text(uint16_t posx, uint16_t posy, const char *text);
void vdp_write_char(uint16_t posx, uint16_t posy, uint16_t text);
void vdp_cursor_pos(int x, int y);
void vdp_cursor_size(int top, int bottom);

// Draw command base address
void vdp_set_drawbase_addr(uint32_t addr);
uint32_t vdp_get_drawbase_addr(void);

// Drawing commands
void vdp_set_drawmode(uint16_t mode);
void vdp_set_draw_color(uint16_t col);
void vdp_set_back_color(uint16_t col);
void vdp_set_pattern(uint16_t pat);
void vdp_draw_fill_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void vdp_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void vdp_draw_hline(uint16_t x, uint16_t y);
void vdp_draw_vline(uint16_t x, uint16_t y);
void vdp_draw_fill_tri(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,  uint16_t x2, uint16_t y2);

void vdp_set_bitmap_palette(uint16_t bank);
void vdp_set_text_palette(uint16_t bank);
void vdp_set_tile_palette(uint16_t layer, uint16_t bank);
void vdp_set_sprite_palette(uint16_t bank);

// Tilemap functions
void vdp_tilemap1_map_addr(uint32_t addr);
void vdp_tilemap1_data_addr(uint32_t addr);
void vdp_tilemap1_enable(void);
void vdp_tilemap1_disable(void);
void vdp_tilemap1_scroll_x(uint16_t x);
void vdp_tilemap1_scroll_y(uint16_t y);
void vdp_tilemap1_linescr_mode(uint16_t mode);

void vdp_tilemap2_map_addr(uint32_t addr);
void vdp_tilemap2_data_addr(uint32_t addr);
void vdp_tilemap2_enable(void);
void vdp_tilemap2_disable(void);
void vdp_tilemap2_scroll_x(uint16_t x);
void vdp_tilemap2_scroll_y(uint16_t y);
void vdp_tilemap2_linescr_mode(uint16_t mode);

void vdp_tilemap3_map_addr(uint32_t addr);
void vdp_tilemap3_data_addr(uint32_t addr);
void vdp_tilemap3_enable(void);
void vdp_tilemap3_disable(void);
void vdp_tilemap3_scroll_x(uint16_t x);
void vdp_tilemap3_scroll_y(uint16_t y);
void vdp_tilemap3_linescr_mode(uint16_t mode);

void vdp_tilemap4_map_addr(uint32_t addr);
void vdp_tilemap4_data_addr(uint32_t addr);
void vdp_tilemap4_enable(void);
void vdp_tilemap4_disable(void);
void vdp_tilemap4_scroll_x(uint16_t x);
void vdp_tilemap4_scroll_y(uint16_t y);
void vdp_tilemap4_linescr_mode(uint16_t mode);

void vdp_sprite_data_addr(uint32_t addr);

void vdp_vram_write(uint32_t addr, const uint16_t *src, size_t size);
void vdp_init_tile_layer(tile_layer_t *layer);

void vdp_disable_all_sprites(void);

static inline void vdp_set_sprite_index(uint8_t index)
{
    VDP_REG_WRITE(REG_SPRITE_IDX, index);    
}

static inline void vdp_write_sprite_x(int16_t x)
{
    VDP_REG_WRITE(REG_SPRITE_POS_X, x);
}

static inline void vdp_write_sprite_y(int16_t y)
{
    VDP_REG_WRITE(REG_SPRITE_POS_Y, y);
}

static inline void vdp_write_sprite_tile(uint16_t tile)
{
    VDP_REG_WRITE(REG_SPRITE_TILE, tile);
}

static inline void vdp_write_sprite_attr(uint8_t palette,
                           uint8_t priority,
                           uint8_t flip_x,
                           uint8_t flip_y)
{
    uint16_t attrib = 0x8000; // enable
    attrib |= (palette & 0x0F) | (flip_x << 4) | (flip_y << 5);
    attrib |= (uint16_t)(priority & 0xF) << 6; 
    VDP_REG_WRITE(REG_SPRITE_ATTRIB, attrib);
}

void vdp_fill_memory(uint32_t addr, uint16_t length, uint16_t value);
void vdp_copy_memory(uint32_t src, uint32_t dest, uint16_t length);
void vdp_copy_2d(uint32_t src_base_word, uint32_t dst_base_word,
    uint16_t src_x, uint16_t src_y, uint16_t dst_x, uint16_t dst_y,
    uint16_t width_pixels, uint16_t height_lines, uint16_t screen_width_pixels);

#endif
