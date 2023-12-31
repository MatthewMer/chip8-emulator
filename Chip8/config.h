#ifndef CONFIG_H
#define CONFIG_H

#undef NDEBUG

#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32
#define CHIP8_HEIGHT_HALF CHIP8_HEIGHT / 2
#define CHIP8_WIDTH_HALF CHIP8_WIDTH / 2
#define CHIP8_WIN_MULT 25
#define CHIP8_SCREEN_BORDER_PADDING 5		// in percent
#define EMU_WIN_TITLE "Chip8 Emulator"

#define CHIP8_CLOCK_TIME 2					// in ns (500Hz)
#define CHIP8_DT_TIME 17					// in ms (60Hz)

#define CHIP8_MEMORY_SIZE 0x1000            // 4096 Bytes
#define CHIP8_CHARSET_ADDR 0x000			// 5 bytes per sprite; 0x00 - 0x50 
#define CHIP8_PROG_LOAD_ADDR 0x200

#define CHIP8_REGISTERS_SIZE 0x10
#define CHIP8_STACK_SIZE 0x10

#define CHIP8_KEYS_NUM 16

#endif