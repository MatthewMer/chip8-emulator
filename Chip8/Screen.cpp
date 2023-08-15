#include "Screen.h"
#include "cstring"

#include "iostream"

Screen::Screen(){}

void Screen::set(int x, int y) {
	pixels[x][y] = true;
}
bool Screen::isset(int x, int y) {
	return pixels[x][y];
}

u8 Screen::draw_sprite(int x, int y, const u8* sprite, int num) {
	u8 collision = 0;
	int x_ = x % CHIP8_WIDTH, y_ = y % CHIP8_HEIGHT;
	int x_cur, y_cur;

	for (int y_offset = 0; y_offset < num; y_offset++) {
		u8 byte = sprite[y_offset];

		for (int x_offset = 0; x_offset < 8; x_offset++) {

			x_cur = x_ + x_offset;
			y_cur = y_ + y_offset;

			if (x_cur < CHIP8_WIDTH && y_cur < CHIP8_HEIGHT) {
				if (byte & (0x80 >> x_offset)) {
					if (pixels[x_cur][y_cur]) collision = 1;
					pixels[x_cur][y_cur] ^= true;
				}
			}
		}
	}

	return collision;
}

void Screen::clear() {
	memset(&pixels, false, sizeof(pixels));
}