#ifndef CHIP8_KEYBOARD_H
#define CHIP8_KEYBOARD_H

#include "config.h"
#include "defs.h"
#include "SDL.h"


class Keyboard {
private:
	bool pressed_keys[CHIP8_KEYS_NUM] = {};
	char map[CHIP8_KEYS_NUM] = {
		SDLK_0,SDLK_1, SDLK_2, SDLK_3, 
		SDLK_4, SDLK_5, SDLK_6, SDLK_7, 
		SDLK_8, SDLK_9, SDLK_a, SDLK_b, 
		SDLK_c, SDLK_d, SDLK_e, SDLK_f
	};

public:
	Keyboard();

	int get_mapped_key(char key);

	u8 get_key();

	bool key_isdown(int key);
	void key_down(int key);
	void key_up(int key);
};

#endif