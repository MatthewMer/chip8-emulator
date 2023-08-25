#ifndef CHIP8_KEYBOARD_H
#define CHIP8_KEYBOARD_H

#include "config.h"
#include "defs.h"
#include "SDL.h"


class Keyboard {
private:
	bool pressed_keys[CHIP8_KEYS_NUM] = {};
	char map[CHIP8_KEYS_NUM] = {
		SDLK_x, SDLK_1, SDLK_2, SDLK_3, 
		SDLK_q, SDLK_w, SDLK_e, SDLK_a, 
		SDLK_s, SDLK_d, SDLK_z, SDLK_c, 
		SDLK_4, SDLK_r, SDLK_f, SDLK_v
	};

public:
	Keyboard();

	int get_mapped_key(char key);

	u8 get_key();

	bool key_isdown(int key);
	void key_down(int key);

	u8 any_key_down();
};

#endif