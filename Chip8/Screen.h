#ifndef CHIP8_SCREEN_H
#define CHIP8_SCREEN_H

#include "config.h"
#include "defs.h"

class Screen {
private:

public:
	Screen();

	bool pixels[CHIP8_WIDTH][CHIP8_HEIGHT] = {};

	void set(int x, int y);
	bool isset(int x, int y);

	u8 draw_sprite(int x, int y, const u8* sprite, int num);

	void clear();
};

#endif
