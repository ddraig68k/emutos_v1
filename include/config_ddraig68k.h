#ifdef MACHINE_DDRAIG68K
# ifndef CONF_ATARI_HARDWARE
#  define CONF_ATARI_HARDWARE 0
# endif
# ifndef CONF_WITH_ADVANCED_CPU
#  define CONF_WITH_ADVANCED_CPU 1
# endif
# ifndef CONF_WITH_APOLLO_68080
#  define CONF_WITH_APOLLO_68080 0
# endif
# ifndef CONF_WITH_BUS_ERROR
#  define CONF_WITH_BUS_ERROR 1
# endif
# ifndef CONF_WITH_CACHE_CONTROL
#  define CONF_WITH_CACHE_CONTROL 0
# endif
# ifndef ALWAYS_SHOW_INITINFO
#  define ALWAYS_SHOW_INITINFO 1
# endif

/*
 * The Ddraig target normally ships without AES, but when AES/desktop support
 * is enabled it is safer to use the larger stack size already used by other
 * non-Atari targets.  AES_STACK_SIZE is specified in LONGs, so 2048 = 8 KiB.
 */
# ifndef AES_STACK_SIZE
#  define AES_STACK_SIZE 2048
# endif

# ifndef CONF_STRAM_SIZE
#  define CONF_STRAM_SIZE (6<<20)
# endif
# ifndef CONF_WITH_ALT_RAM
#  define CONF_WITH_ALT_RAM 0
# endif
# ifndef CONF_WITH_MFP
#  define CONF_WITH_MFP 0
# endif
# ifndef CONF_WITH_DUART
#  define CONF_WITH_DUART 1
# endif
# ifndef DUART_BASE
#  define DUART_BASE 0xFFF7F000UL
# endif
# ifndef CONF_WITH_DUART_CHANNEL_B
#  define CONF_WITH_DUART_CHANNEL_B 1
# endif
# ifndef CONF_DUART_TIMER_C
#  define CONF_DUART_TIMER_C 1
# endif
# ifndef CONF_DUART_AUTOVECTOR
#  define CONF_DUART_AUTOVECTOR 4
# endif
# ifndef DUART_DEBUG_PRINT
#  define DUART_DEBUG_PRINT 1
# endif
# ifndef RS232_DEBUG_PRINT
#  define RS232_DEBUG_PRINT 0
# endif

#ifndef CONF_WITH_FDC
# define CONF_WITH_FDC 0
#endif

#ifndef CONF_WITH_ACSI
# define CONF_WITH_ACSI 0
#endif

#ifndef CONF_WITH_SCSI
# define CONF_WITH_SCSI 0
#endif

#define CONF_WITH_VBL_RTE 1

# ifndef CONF_WITH_IDE
#  define CONF_WITH_IDE 1
# endif
# ifndef CONF_ATARI_IDE
#  define CONF_ATARI_IDE 1
# endif
# ifndef CONF_IDE_NO_RESET
#  define CONF_IDE_NO_RESET 1
# endif

# ifndef CONF_WITH_SDMMC
#  define CONF_WITH_SDMMC 0
# endif

# ifndef CONF_WITH_RESET
#  define CONF_WITH_RESET 0
# endif

# ifndef CONF_SERIAL_CONSOLE
#  define CONF_SERIAL_CONSOLE 1
# endif
# ifndef CONF_SERIAL_CONSOLE_ANSI
#  if CONF_SERIAL_CONSOLE
#   define CONF_SERIAL_CONSOLE_ANSI 1
#  else
#   define CONF_SERIAL_CONSOLE_ANSI 0
#  endif
# endif
# ifndef CONF_SERIAL_CONSOLE_POLLING_MODE
#  define CONF_SERIAL_CONSOLE_POLLING_MODE 0
# endif
/*
 * DdraigVGA display mode.  With CONF_WITH_DDRAIGVGA_DESKTOP the standard
 * EmuTOS framebuffer console and native VDI render directly into the
 * card's 640x480 1bpp bitmap mode, so the GEM desktop runs without fVDI.
 * Set it to 0 to get the original VDP text-cell console instead (GEM
 * then requires fVDI).
 */
#ifndef CONF_WITH_DDRAIGVGA_DESKTOP
# define CONF_WITH_DDRAIGVGA_DESKTOP 1
#endif
#if CONF_WITH_DDRAIGVGA_DESKTOP
# ifndef CONF_VRAM_ADDRESS
#  define CONF_VRAM_ADDRESS 0xA00000UL
# endif
#else
# ifndef CONF_WITH_DDRAIGVGA_CONSOLE
#  define CONF_WITH_DDRAIGVGA_CONSOLE 1
# endif
#endif
#ifndef CONF_WITH_VT82C42
# define CONF_WITH_VT82C42 1
#endif
# ifndef CONF_VT82C42_AUTOVECTOR
#  define CONF_VT82C42_AUTOVECTOR 5
# endif

# ifndef USE_STOP_INSN_TO_FREE_HOST_CPU
#  define USE_STOP_INSN_TO_FREE_HOST_CPU 0
# endif
# ifndef DETECT_NATIVE_FEATURES
#  define DETECT_NATIVE_FEATURES 0
# endif

/*
 * Set CONF_WITH_EXTENDED_MOUSE to 1 to enable extended mouse support.
 * This includes new Eiffel scancodes for mouse buttons 3, 4, 5, and
 * the wheel.
 */
#ifndef CONF_WITH_EXTENDED_MOUSE
# define CONF_WITH_EXTENDED_MOUSE 0
#endif

#define DEFAULT_BAUDRATE B19200

#endif
