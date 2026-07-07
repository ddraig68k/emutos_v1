#include "config.h"
#include <stdint.h>

// Keyboard initialisation improved to stop spurios charater interrupts occuringh
// Based on Tom Storey's code https://github.com/tomstorey/emutos/blob/comet68k/bios/comet_vga.c


/* #define ENABLE_KDEBUG */

#if CONF_WITH_VT82C42

#include "emutos.h"
#include "ikbd.h"
#include "kprint.h"
#include "vectors.h"
#include "asm.h"
#include "tosvars.h"

#include <stdio.h>
#include <ctype.h>

#include "vt82c42.h"

#define PS2_BASE                0x00F7F200
#define PS2_READ(r) (*(volatile UBYTE *)(PS2_BASE + (r)))
#define PS2_WRITE(v, r) (*(volatile UBYTE *)(PS2_BASE + (r)) = (v))

// Register Offsets
#define PS2_DATA		  		0x00
#define PS2_CMD 				0x02
#define PS2_STAT				0x02

// For mouse
#define MOUSE_REL_POS_REPORT    0xf8    /* values for mouse_packet[0] */
#define RIGHT_BUTTON_DOWN       0x01    /* these values are OR'ed in */
#define LEFT_BUTTON_DOWN        0x02
#define MOUSE_STATUS_XOVERFLOW  0x40
#define MOUSE_STATUS_YOVERFLOW  0x80

#define WAIT_TIMEOUT 10000
#define VT82C42_DEBUG_COUNTERS 1
#define VT82C42_MAX_IRQ_BYTES 16
#ifndef VT82C42_MOUSE_SAMPLE_RATE
#define VT82C42_MOUSE_SAMPLE_RATE 10
#endif

enum vt_port {
    PORT_KB = 0,
    PORT_MS = 1
};

// Function prototypes
static void vt_delay(ULONG count);
static void vt_write_wait(const UBYTE data, const UBYTE reg);
static UBYTE vt_cmd_data_polled(const UBYTE cmd);
static UBYTE vt_data_data_polled(const UBYTE data);
static UBYTE vt_data_polled(void);
static UBYTE vt_send_device_cmd(const enum vt_port port, UBYTE cmd);
static UBYTE vt_get_cmd_byte(void);
static void vt_set_cmd_byte(const UBYTE cmd);
static UBYTE device_keyboard_reset(void);
static void device_keyboard_led_animate(void);
static void vt_set_leds(UBYTE leds);
static void vt_flush(void);

void __attribute__((interrupt)) vt_interrupt_handler(void);
void vt_process_scancode(UBYTE sc);
static void vt_process_mouse(int8_t *packet);
void vt_handle_mouse(UBYTE data);
static void vt_flush_mouse_packet(void);

extern volatile ULONG ikbd_reset_drops;   /* in aciavecs.S: in_packet guard */

static UBYTE g_key_mode = 0;
static UBYTE mouse_pending;
static UBYTE mouse_pending_buttons;
static WORD mouse_pending_dx;
static WORD mouse_pending_dy;

#if VT82C42_DEBUG_COUNTERS
static volatile ULONG dbg_irq_no_obf;
static volatile ULONG dbg_irq_max_bytes;
static volatile ULONG dbg_mouse_bad_sync;
static volatile ULONG dbg_mouse_overflow_drop;
static volatile ULONG dbg_mouse_packet_in;
static volatile ULONG dbg_mouse_packet_out;
static volatile ULONG dbg_mouse_packet_coalesced;
static volatile ULONG dbg_mouse_delta_saturated;
static volatile ULONG dbg_scancode_proto_drop;
static volatile ULONG dbg_scancode_oob_drop;
static volatile ULONG dbg_scancode_unmapped_drop;

static void vt82c42_debug_reset_counters(void)
{
    dbg_irq_no_obf = 0;
    dbg_irq_max_bytes = 0;
    dbg_mouse_bad_sync = 0;
    dbg_mouse_overflow_drop = 0;
    dbg_mouse_packet_in = 0;
    dbg_mouse_packet_out = 0;
    dbg_mouse_packet_coalesced = 0;
    dbg_mouse_delta_saturated = 0;
    dbg_scancode_proto_drop = 0;
    dbg_scancode_oob_drop = 0;
    dbg_scancode_unmapped_drop = 0;
}

void vt82c42_debug_dump_counters(void)
{
    kcprintf("vt82c42 dbg: irq_no_obf=%lu irq_max_bytes=%lu mouse_bad_sync=%lu mouse_overflow=%lu\n",
        dbg_irq_no_obf, dbg_irq_max_bytes, dbg_mouse_bad_sync, dbg_mouse_overflow_drop);
    kcprintf("vt82c42 dbg: mouse_in=%lu mouse_out=%lu mouse_coalesced=%lu mouse_sat=%lu\n",
        dbg_mouse_packet_in, dbg_mouse_packet_out, dbg_mouse_packet_coalesced, dbg_mouse_delta_saturated);
    kcprintf("vt82c42 dbg: sc_proto_drop=%lu sc_oob_drop=%lu sc_unmapped_drop=%lu\n",
        dbg_scancode_proto_drop, dbg_scancode_oob_drop, dbg_scancode_unmapped_drop);
    kcprintf("ikbd dbg: reset_drops=%lu\n", ikbd_reset_drops);
}
#else
static void vt82c42_debug_reset_counters(void)
{
}

void vt82c42_debug_dump_counters(void)
{
}
#endif

static const UBYTE st_make_code_map[] = {
    0 , 67 /*F9*/, 0 , 63 /*F5*/, 61 /*F3*/, 59 /*F1*/, 60 /*F2*/, 97 /*F12*/,
	0 , 68 /*F10*/, 66 /*F8*/, 64 /*F6*/, 62 /*F4*/, 15 /*Tab*/, 41 /*Backtick/Tilde (`~)*/, 0 , 
    0 , 56 /*Left Alt*/, 42 /*Left Shift*/, 0 , 29 /*Left Ctrl*/, 16 /*Q*/, 2 /*1*/, 0 ,
    0 , 0 , 44 /*Z*/, 31 /*S*/, 30 /*A*/, 17 /*W*/, 3 /*2*/, 0 ,
	0 , 46 /*C*/, 45 /*X*/, 32 /*D*/, 18 /*E*/, 5 /*4*/, 4 /*3*/, 0 ,
    0 , 57 /*Space*/, 47 /*V*/, 33 /*F*/, 20 /*T*/, 19 /*R*/, 6 /*5*/, 0 ,
    0 , 49 /*N*/, 48 /*B*/, 35 /*H*/, 34 /*G*/, 21 /*Y*/, 7 /*6*/, 0 ,
    0 , 0 , 50 /*M*/, 36 /*J*/, 22 /*U*/, 8 /*7*/, 9 /*8*/, 0 ,
    0 , 51 /*Comma (,<)*/, 37 /*K*/, 23 /*I*/, 24 /*O*/, 11 /*0*/, 10 /*9*/, 0 ,
    0 , 52 /*Period (.>)*/, 53 /*Slash (/?)*/, 38 /*L*/, 39 /*Semicolon (;:)*/, 25 /*P*/, 12 /*Minus (-_)*/, 0 ,
    0 , 0 , 40 /*Apostrophe ('")*/, 0 , 26 /*Left Bracket ([{)*/, 13 /*Equals (=+)*/, 0 , 0 , 58 /*CapsLock*/,
    54 /*Right Shift*/, 28 /*Enter*/, 27 /*Right Bracket (]})*/, 0 , 43 /*Backslash (\|)*/, 0 , 
    0 , 0 , 96 /*UK \| between left shift and Z*/, 0 , 0 , 0 , 0 , 14 /*Backspace*/, 0 , 0 , 109 /*Keypad 1/End*/,
    0 , 106 /*Keypad 4/Left*/, 103 /*Keypad 7/Home*/, 0 , 0 , 0 , 112 /*Keypad 0/Ins*/, 113 /*Keypad ./Del*/,
    110 /*Keypad 2/Down*/, 107 /*Keypad 5*/, 108 /*Keypad 6/Right*/, 104 /*Keypad 8/Up*/, 1 /*Escape*/,
    -1 /*NumLock*/, 98 /*F11*/, 78 /*Keypad +*/, 111 /*Keypad 3/PgDn*/, 74 /*Keypad -*/, 102 /*Keypad **/,
    105 /*Keypad 9/PgUp*/, -1 /*ScrollLock*/, 0 , 0 , 0 , 0 , 65 /*F7*/
};

static const UBYTE st_extended_make_code_map[] = {
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
    56 /*Right Alt*/, 0 , 0 , 29 /*Right Ctrl*/, 0 , 0 , 0 , 0 , 0 , 0 ,
    0 , 0 , 0 , 0 , -1 /*Left GUI (Windows)*/, 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
    -1 /*Right GUI (Windows)*/, 0 , 0 , 0 , 0 , 0 , 0 , 0 , -1 /*Menu*/,
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 101 /*Keypad /*/, 0 , 0 , 0 ,
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 114 /*Keypad Enter*/,
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 79 /*End*/,
    0 , 75 /*Left Arrow*/, 71 /*Home*/, 0 , 0 , 0 , 82 /*Insert*/, 83 /*Delete*/,
    80 /*Down Arrow*/, 0 , 77 /*Right Arrow*/, 72 /*Up Arrow*/, 0 , 0 , 0 , 0 ,
    81 /*Page Down*/, 0 , 0 , 73 /*Page Up*/, 0 , 0
};

static void vt_delay(ULONG count)
{
    volatile ULONG delay = count;

    while (delay--)
    {}
}

static void vt_write_wait(const UBYTE data, const UBYTE reg)
{
    UBYTE val;
    volatile ULONG timeout = WAIT_TIMEOUT;
    
    /* Wait until input buffer empty */
    do {
        val = PS2_READ(PS2_CMD);
        if (--timeout == 0)
            break;
    } while (val & STATUS_IBF);

    /* Send the command */
    PS2_WRITE(data, reg);
}

static UBYTE vt_cmd_data_polled(const UBYTE cmd)
{
    UBYTE val;
    volatile ULONG timeout = WAIT_TIMEOUT;

    /* Send the command */
    vt_write_wait(cmd, PS2_CMD);

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
        if (--timeout == 0)
            break;        
    } while (!(val & STATUS_OBF));

    /* Return the value from the data register */
    val = PS2_READ(PS2_DATA);

    return val;
}

static UBYTE vt_data_data_polled(const UBYTE data)
{
    UBYTE val;
    volatile ULONG timeout = WAIT_TIMEOUT;

    /* Write the data */
    vt_write_wait(data, PS2_DATA);

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
        if (--timeout == 0)
            break;        
    } while (!(val & STATUS_OBF));

    /* Return the value from the data register */
    val = PS2_READ(PS2_DATA);

    return val;
}

static UBYTE vt_data_polled(void)
{
    UBYTE val;
    volatile ULONG timeout = WAIT_TIMEOUT;

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
        if (--timeout == 0)
            break;      
    } while (!(val & STATUS_OBF));

    /* Return the value from the data register */
    val = PS2_READ(PS2_DATA);

    return val;
}

static UBYTE vt_send_device_cmd(const enum vt_port port, UBYTE cmd)
{
    UBYTE retries = 10;
    UBYTE val;

    while (retries--) {
        if (port == PORT_MS) {
            /* Will write to the mouse output port */
            vt_write_wait(0xD4, PS2_CMD);
        }

        val = vt_data_data_polled(cmd);

        if (retries == 1 || val != 0xFE) {
            break;
        }

        /* Delay until before retrying */
        vt_delay(500);
    }

    return val;
}

static UBYTE vt_get_cmd_byte(void)
{
    return vt_cmd_data_polled(0x20);
}

static void vt_set_cmd_byte(const UBYTE cmd)
{
    vt_write_wait(0x60, PS2_CMD);
    vt_write_wait(cmd, PS2_DATA);
}

static UBYTE device_keyboard_reset(void)
{
    UBYTE val;

    /* Send the keyboard reset command */
    val = vt_send_device_cmd(PORT_KB, 0xFF);

    if (val != 0xFA) {
        /* Keyboard did not acknowledge reset command */
        return 0;
    }

    /* Keyboard acknowledged the reset command, it should also indicate whether the reset was successful */
    val = vt_data_polled();

    if (val != 0xAA) {
        return 0;
    }

    /* Reset succeeded */
    return 1;
}

static void device_keyboard_led_animate(void)
{
    /* Animation pattern: none -> num -> caps -> scroll -> none -> num - 0xFF terminates */
    const UBYTE anim[] = {0, STATUS_NUM_LOCK, STATUS_CAPS_LOCK, STATUS_SCROLL_LOCK, 0, STATUS_NUM_LOCK, 0xFF};

    UBYTE i;

    for (i = 0;; i++) {
        if (anim[i] == 0xFF) {
            /* Animation done */
            break;
        }

        /* Set LED */
        (void)vt_data_data_polled(0xED);
        (void)vt_data_data_polled(anim[i]);

        /* Update LED status for tracking purposes */
        g_key_mode = anim[i];

        /* Delay between transitions */
        vt_delay(2000);
    }
}

static void vt_set_leds(UBYTE leds)
{
    vt_data_data_polled(KBD_CMD_LED);
	vt_data_data_polled(leds);
}

static void vt_flush(void)
{
    UBYTE status;
    /* Flush the output buffer */
    for (;;)
    {
        status = PS2_READ(PS2_CMD);

        if (status & STATUS_OBF) {
            (void)PS2_READ(PS2_DATA);
            KDEBUG(("vt8242_init(): flush\n"));
        } else {
            break;
        }
    }
}

static WORD vt_mouse_saturating_add(WORD acc, WORD delta)
{
    LONG value = (LONG)acc + delta;

    if (value > 127) {
#if VT82C42_DEBUG_COUNTERS
        dbg_mouse_delta_saturated++;
#endif
        return 127;
    }

    if (value < -127) {
#if VT82C42_DEBUG_COUNTERS
        dbg_mouse_delta_saturated++;
#endif
        return -127;
    }

    return (WORD)value;
}

static void vt_emit_mouse_packet(UBYTE buttons, WORD dx, WORD dy)
{
    UBYTE packet0 = MOUSE_REL_POS_REPORT | buttons;

    KDEBUG(("Mouse: X=%d Y=%d B=%d\n", dx, dy, buttons));

    call_ikbdraw(packet0);
    call_ikbdraw((UBYTE)dx);
    call_ikbdraw((UBYTE)dy);
}

static void vt_queue_mouse_packet(int8_t *process)
{
    UBYTE buttons = 0;
    UBYTE status = (UBYTE)process[0];
    WORD dx = process[1];
    WORD dy = -(WORD)process[2];

#if VT82C42_DEBUG_COUNTERS
    dbg_mouse_packet_in++;
    if (mouse_pending)
        dbg_mouse_packet_coalesced++;
#endif

    if (status & 0x01)
        buttons |= LEFT_BUTTON_DOWN;
    if (status & 0x02)
        buttons |= RIGHT_BUTTON_DOWN;

    mouse_pending_dx = vt_mouse_saturating_add(mouse_pending_dx, dx);
    mouse_pending_dy = vt_mouse_saturating_add(mouse_pending_dy, dy);
    mouse_pending_buttons = buttons;
    mouse_pending = TRUE;
}

static void vt_flush_mouse_packet(void)
{
    UBYTE buttons;
    WORD dx;
    WORD dy;
    WORD old_sr;

    old_sr = set_sr(0x2700);
    if (!mouse_pending)
    {
        set_sr(old_sr);
        return;
    }

    buttons = mouse_pending_buttons;
    dx = mouse_pending_dx;
    dy = mouse_pending_dy;

    mouse_pending = FALSE;
    mouse_pending_buttons = 0;
    mouse_pending_dx = 0;
    mouse_pending_dy = 0;

#if VT82C42_DEBUG_COUNTERS
    dbg_mouse_packet_out++;
#endif

    /*
     * Emit while interrupts are still masked.  ikbdraw is a non-reentrant
     * state machine, and mousevec (fVDI's cursor engine) assumes the
     * atomicity it gets on Atari hardware, where this whole chain runs at
     * IPL 6 inside the ACIA interrupt.  Restoring SR first would let the
     * VBL/DUART/PS2 interrupts preempt fVDI mid-draw.
     */
    vt_emit_mouse_packet(buttons, dx, dy);
    set_sr(old_sr);
}

void vt82c42_poll_mouse(void)
{
    vt_flush_mouse_packet();
}

//	keyboard interrupt handler
void __attribute__((interrupt)) vt_interrupt_handler(void)
{
    UBYTE status;
    UBYTE data;
    UBYTE n;

    status = PS2_READ(PS2_STAT);
    if (!(status & STATUS_OBF)) {
#if VT82C42_DEBUG_COUNTERS
        dbg_irq_no_obf++;
#endif
        return;
    }

    /* Drain pending controller bytes with a bound to avoid a long IRQ stall. */
    for (n = 0; n < VT82C42_MAX_IRQ_BYTES; n++) {
        status = PS2_READ(PS2_STAT);
        if (!(status & STATUS_OBF))
            break;

        data = PS2_READ(PS2_DATA);

        if (status & STATUS_AUXDATA)
            vt_handle_mouse(data);
        else
            vt_process_scancode(data);
    }

#if VT82C42_DEBUG_COUNTERS
    if (n == VT82C42_MAX_IRQ_BYTES)
        dbg_irq_max_bytes++;
#endif

}

UBYTE vt8242_init(void)
{
    volatile PFVOID *vector_addr;
    UBYTE data;
    UBYTE cfg;

    KDEBUG(("vt8242_init()\n"));
    WORD old_sr;
    /* disable interrupts */
    old_sr = set_sr(0x2700);
    vt82c42_debug_reset_counters();
    mouse_pending = FALSE;
    mouse_pending_buttons = 0;
    mouse_pending_dx = 0;
    mouse_pending_dy = 0;

    /* Disable keyboard/mouse ports and interrupts while probing the controller. */
    cfg = vt_get_cmd_byte();
    cfg &= ~(CMD_BYTE_KBD_INT | CMD_BYTE_AUX_INT | CMD_BYTE_TRANS);
    cfg |= (CMD_BYTE_AUX_OFF | CMD_BYTE_KBD_OFF);
    vt_set_cmd_byte(cfg);
    vt_flush();

    KDEBUG(("vt8242_init: install keyboard interrupt handler\n"));
    vector_addr = &VEC_LEVEL1 + (CONF_VT82C42_AUTOVECTOR - 1);
    *vector_addr = (PFVOID)vt_interrupt_handler;

    KDEBUG(("vt8242_init: controller self test\n"));
    data = vt_cmd_data_polled(CMD_DIAG);
	if (data != KBD_STATUS_DIAG_OK)
		KDEBUG(("vt8242_init: PS/2 keyboard controller FAILED.\n"));
    else
        KDEBUG(("vt8242_init: PS/2 keyboard controller passed.\n"));

    /* Controller firmware/hardware versions */
    data = vt_cmd_data_polled(CMD_VERSION_CTRL);
    KDEBUG(("vt8242_init: Version (A1)=%02X\n", data));
    data = vt_cmd_data_polled(CMD_VERSION);
    KDEBUG(("vt8242_init: Version (AF)=%02X\n", data));

    /* Operating mode */
    data = vt_cmd_data_polled(CMD_GET_MODE);
    if (data == 0x01) {
        KDEBUG(("vt8242_init: PS/2 mode\n"));
    } else {
        KDEBUG(("vt8242_init: AT mode (unsupported)\n"));
    }

	KDEBUG(("vt8242_init: starting keyboard test.\n"));
    data = vt_cmd_data_polled(CMD_KBD_TEST);
	if (data != 0x00)
    {
		KDEBUG(("vt8242_init: PS/2 keyboard test failed.\n"));
	}

    /* Enable the keyboard interface */
    vt_write_wait(CMD_KBD_ON, PS2_CMD);

    /* Reset keyboard */
    // if (device_keyboard_reset() != 1) {
    //     KDEBUG(("vt8242_init(): Keyboard reset failed\n"));
    // }

    /* Do a little LED animation :) */
    device_keyboard_led_animate();

    /* Initialise the mouse controller */
    vt_write_wait(CMD_AUX_ON, PS2_CMD); // enable 2nd port

    data = vt_send_device_cmd(PORT_MS, MOUSE_CMD_RATE);
    if (data != 0xFA) {
        KDEBUG(("vt8242_init(): Mouse CMD_RATE failed, got %02X\n", data));
    }
    data = vt_send_device_cmd(PORT_MS, VT82C42_MOUSE_SAMPLE_RATE); // Set sample rate to reports/sec
    if (data != 0xFA) {
        KDEBUG(("vt8242_init(): Mouse CMD_RATE data failed, got %02X\n", data));
    }

    data = vt_send_device_cmd(PORT_MS, MOUSE_CMD_RESOLUTION);
    if (data != 0xFA) {
        KDEBUG(("vt8242_init(): Mouse CMD_RESOLUTION failed, got %02X\n", data));
    }
    data = vt_send_device_cmd(PORT_MS, 1);
    if (data != 0xFA) {
        KDEBUG(("vt8242_init(): Mouse CMD_RESOLUTION data failed, got %02X\n", data));
    }

    /* Enable streaming mode */
    data = vt_send_device_cmd(PORT_MS, MOUSE_CMD_DATAEN); // enable data reporting
    if (data != 0xFA) {
        KDEBUG(("Mouse enable stream failed: got %02X\n", data));
    }

    KDEBUG(("Mouse init complete\n"));
    KDEBUG(("VT82C42 Init complete\n"));

    vt_flush();

    // Enable interrupts (keep translation off, and explicitly enable both ports)
    cfg = vt_get_cmd_byte();
    cfg &= ~(CMD_BYTE_AUX_OFF | CMD_BYTE_KBD_OFF | CMD_BYTE_TRANS);
    cfg |= (CMD_BYTE_KBD_INT | CMD_BYTE_AUX_INT);
    vt_set_cmd_byte(cfg);

    /* restore interrupts */
    set_sr(old_sr);    

    return 1;
}

static void vt_process_mouse(int8_t *process)
{
    vt_queue_mouse_packet(process);
}

void vt_handle_mouse(UBYTE data)
{
    static UBYTE mouse_cycle = 0;
    static UBYTE mouse_bytes[3] = { 0 };

    switch (mouse_cycle)
    {
        case 0:
            // FIrst byte should have bit 3 set (sync)
            if (!(data & 0x08)) {
#if VT82C42_DEBUG_COUNTERS
                dbg_mouse_bad_sync++;
#endif
                return;
            }

            /* Drop packets with overflow bits set: deltas are invalid anyway. */
            if (data & (MOUSE_STATUS_XOVERFLOW | MOUSE_STATUS_YOVERFLOW)) {
#if VT82C42_DEBUG_COUNTERS
                dbg_mouse_overflow_drop++;
#endif
                return;
            }

            mouse_bytes[0] = data;
            mouse_cycle = 1;
            break;
        
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle = 2;
            break;

        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 0;
            vt_process_mouse((int8_t *)mouse_bytes);
            break;
    }
}

void vt_process_scancode(UBYTE sc)
{
    static UBYTE key_break = 0;
    static UBYTE key_extended = 0;
    static UBYTE key_remaining = 0;

	UBYTE register chr;

    /* Ignore PS/2 protocol/status replies that are not keyboard scancodes. */
    if ((sc == KBD_STATUS_ACK) || (sc == KBD_STATUS_RESEND)
        || (sc == KBD_STATUS_ECHO) || (sc == KBD_STATUS_RST_OK)
        || (sc == KBD_STATUS_OVER)) {
    #if VT82C42_DEBUG_COUNTERS
        dbg_scancode_proto_drop++;
    #endif
        return;
        }

    if (key_remaining > 0)
    {
        key_remaining--;
        return;
    }
    else if (sc == SCAN_CODE_BREAK)
        key_break = 1;
    else if (sc == SCAN_CODE_MODIFIER)
        key_extended  = 1;
    else if (sc == SCAN_CODE_PSBRK)
    {
        // Pause/Break keys extended sequence, ignore for now
        key_remaining = 7;
    }
    else
    {
        if (sc == SCAN_CODE_CAPLOCK)
        {
            g_key_mode ^= STATUS_CAPS_LOCK;
            vt_set_leds(g_key_mode);
        }
        else if (sc == SCAN_CODE_NUMLOCK)
        {
            g_key_mode ^= STATUS_NUM_LOCK;
            vt_set_leds(g_key_mode);
        }
        else if (sc == SCAN_CODE_SCRLOCK)
        {
            g_key_mode ^= STATUS_SCROLL_LOCK;
            vt_set_leds(g_key_mode);
        }

        sc &= 0x7f;

        if (key_extended) {
            if (sc >= ARRAY_SIZE(st_extended_make_code_map)) {
#if VT82C42_DEBUG_COUNTERS
                dbg_scancode_oob_drop++;
#endif
                key_extended = 0;
                key_break = 0;
                return;
            }
            chr = st_extended_make_code_map[sc];
        } else {
            if (sc >= ARRAY_SIZE(st_make_code_map)) {
#if VT82C42_DEBUG_COUNTERS
                dbg_scancode_oob_drop++;
#endif
                key_extended = 0;
                key_break = 0;
                return;
            }
            chr = st_make_code_map[sc];
        }

        /* Ignore unmapped/sentinel entries: sending 0xff to ikbdraw is unsafe. */
        if ((chr == 0) || (chr == 0xff)) {
#if VT82C42_DEBUG_COUNTERS
            dbg_scancode_unmapped_drop++;
#endif
            key_extended = 0;
            key_break = 0;
            return;
        }

        if (key_break)
            chr |= 0x80; // set break code

        
        KDEBUG(("call_ikbdraw 0x%02x\n", chr));
        call_ikbdraw(chr);
        key_extended = 0;
        key_break = 0;        
	}
}

#endif
