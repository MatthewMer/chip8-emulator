#include "Keyboard.h"
#include <cstring>

Keyboard::Keyboard() {
	memset(pressed_keys, false, CHIP8_KEYS_NUM);
}

int Keyboard::get_mapped_key(char key) {
	for (int i = 0; i < CHIP8_KEYS_NUM; i++) {
		if (map[i] == key) {
			return i;
		}
	}

	return -1;
}

u8 Keyboard::get_key() {
	for (u8 i = 0; i < CHIP8_KEYS_NUM; i++) {
		if (pressed_keys[i]) {
			pressed_keys[i] = false;
			return i;
		}
	}

	return -1;
}

bool Keyboard::key_isdown(int key) {
	return pressed_keys[key];
}

void Keyboard::key_down(int key) {
	pressed_keys[key] = true;
}
void Keyboard::key_up(int key) {
	pressed_keys[key] = false;
}