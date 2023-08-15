#include "Chip8.h"
#include <windows.h>
#include <cmath>


// ----- CPU INIT -----

Chip8::Chip8(){
	keys = Keyboard();
	screen = Screen();

	memset(this, 0, sizeof(this));
	memcpy(&this->memory, default_charset, sizeof(default_charset));

	time_prev = high_resolution_clock::now();
}


// ----- CPU FETCH/EXECUTE -----

bool Chip8::exec() {

	if (!awaits_key) {
		// execute
		// 
		// delay timer
		if (reg.dt > 0) {
			do {
				time_cur = high_resolution_clock::now();
				time_passed = duration_cast<milliseconds>(time_cur - time_prev).count();
			} while (time_passed < ms_per_decr);
			time_prev = time_cur;
			reg.dt--;
		}

		// sound timer
		if (reg.st > 0) {
			Beep(500, reg.st * ms_per_decr);
			reg.st = 0;
		}

		// instruction execution

		instr = next_instr();

		u8 opcode = (instr & 0xf000) >> 12;
		u16 data = instr & 0x0fff;

		switch (opcode) {
		case 0x0:
		{
			exec_0(data);
		}break;
		case 0x1:
		{
			printf("JP $%.3x", data);
			reg.PC = data;
		}break;
		case 0x2:
		{
			printf("CALL $%.3x", data);
			stack_push(reg.PC);
			reg.PC = data;
		}break;
		case 0x3:
		{
			u8 x = (data & 0xf00) >> 8;
			u8 kk = data & 0xff;
			printf("SE V%.1x($%.2x), $%.2x", x, reg.V[x], kk);
			if (reg.V[x] == kk) {
				reg.PC += 2;
			}
		}break;
		case 0x4:
		{
			u8 x = (data & 0xf00) >> 8;
			u8 kk = data & 0xff;
			printf("SNE V%.1x($%.2x), $%.2x", x, reg.V[x], kk);
			if (reg.V[x] != kk) {
				reg.PC += 2;
			}
		}break;
		case 0x5:
		{
			switch (data & 0xf) {
			case 0x0:
			{
				u8 x = (data & 0xf00) >> 8;
				u8 y = (data & 0xf0) >> 4;
				printf("SE V%.1x($%.2x), V%.1x($%.2x)", x, reg.V[x], y, reg.V[y]);
				if (reg.V[x] == reg.V[y]) {
					reg.PC += 2;
				}
			}break;
			default:
			{
				error_flags |= 0x01;
				printf("Unknown instruction");
			}break;
			}
		}break;
		case 0x6:
		{
			u8 x = (data & 0xf00) >> 8;
			u8 kk = data & 0xff;
			printf("LD V%.1x, $%.2x", x, kk);
			reg.V[x] = kk;
		}break;
		case 0x7:
		{
			u8 x = (data & 0xf00) >> 8;
			u8 kk = data & 0xff;
			printf("ADD V%.1x($%.2x), $%.2x", x, reg.V[x], kk);
			reg.V[x] += kk;
		}break;
		case 0x8:
		{
			exec_8(data);
		}break;
		case 0x9:
		{
			switch (data & 0xf) {
			case 0x0:
			{
				u8 x = (data & 0xf00) >> 8;
				u8 y = (data & 0xf0) >> 4;
				printf("SNE V%.1x($%.2x), V%.1x($%.2x)", x, reg.V[x], y, reg.V[y]);
				if (reg.V[x] != reg.V[y]) {
					reg.PC += 2;
				}
			}break;
			default:
			{
				error_flags |= 0x01;
				printf("Unknown instruction");
			}break;
			}
		}break;
		case 0xa:
		{
			printf("LD I, $%.3x", data);
			reg.I = data;
		}break;
		case 0xb:
		{
			printf("JP V0($%.2x), $%.3x", *reg.V0, data);
			reg.PC = data + *reg.V0;
		}break;
		case 0xc:
		{
			u8 x = (data & 0xf00) >> 8;
			u8 kk = data & 0xff;
			printf("RND V%.1x($%.2x), $%.2x", x, reg.V[x], kk);
			srand(clock());
			reg.V[x] = (rand() % 256) & kk;
		}break;
		case 0xd:
		{
			u8 x = (data & 0xf00) >> 8;
			u8 y = (data & 0xf0) >> 4;
			u8 n = data & 0xf;
			printf("DRW V%.1x($%.2x), V%.1x($%.2x), $%.1x", x, reg.V[x], y, reg.V[y], n);
			*reg.VF = screen.draw_sprite(reg.V[x], reg.V[y], &memory[reg.I], n);
		}break;
		case 0xe:
		{
			exec_e(data);
		}break;
		case 0xf:
		{
			exec_f(data);
		}break;
		default:
		{
			error_flags |= 0x01;
			printf("Unknown instruction");
		}break;
		}

		printf("\n");
	}
	else {
		for (int i = 0; i < CHIP8_KEYS_NUM; i++) {
			if (keys.key_isdown(i)) {
				*key_dest = i;
				awaits_key = false;
			}
		}
	}

	return error_flags;
}

void Chip8::exec_0(u16 data) {
	switch (data) {
	case 0x0e0:
	{
		printf("CLS");
		screen.clear();
	}break;
	case 0x0ee:
	{
		printf("RET");
		reg.PC = stack_pop();
	}break;
	default:
	{
		error_flags |= 0x01;
		printf("Unknown instruction");
	}break;
	}
}

void Chip8::exec_8(u16 data) {
	u8 opcode = data & 0xf;
	u8 x = (data & 0xf00) >> 8;
	u8 y = (data & 0xf0) >> 4;

	switch (opcode) {
	case 0x0:
		printf("LD V%.1x, V%.1x($%.2x)", x, y, reg.V[y]);
		reg.V[x] = reg.V[y];
		break;
	case 0x1:
		printf("OR V%.1x($%.2x), V%.1x($%.2x)", x, reg.V[x], y, reg.V[y]);
		reg.V[x] |= reg.V[y];
		*reg.VF = 0x00;
		break;
	case 0x2:
		printf("AND V%.1x($%.2x), V%.1x($%.2x)", x, reg.V[x], y, reg.V[y]);
		reg.V[x] &= reg.V[y];
		*reg.VF = 0x00;
		break;
	case 0x3:
		printf("XOR V%.1x($%.2x), V%.1x($%.2x)", x, reg.V[x], y, reg.V[y]);
		reg.V[x] ^= reg.V[y];
		*reg.VF = 0x00;
		break;
	case 0x4:
	{
		printf("ADD V%.1x($%.2x), V%.1x($%.2x) (VF)", x, reg.V[x], y, reg.V[y]);
		u8 carry = ((u16)reg.V[x] + reg.V[y]) > 0xff ? 1 : 0;
		reg.V[x] += reg.V[y];
		*reg.VF = carry;
	}break;
	case 0x5:
	{
		printf("SUB V%.1x($%.2x), V%.1x($%.2x) (VF)", x, reg.V[x], y, reg.V[y]);
		u8 borrow = reg.V[y] > reg.V[x] ? 0 : 1;
		reg.V[x] -= reg.V[y];
		*reg.VF = borrow;
	}break;
	case 0x6:
	{
		printf("SHR V%.1x($%.2x) (VF)", x, reg.V[y]);
		reg.V[x] = reg.V[y];
		u8 carry = reg.V[x] & 0x1;
		reg.V[x] >>= 1;
		*reg.VF = carry;
	}break;
	case 0x7:
	{
		printf("SUBN V%.1x($%.2x), V%.1x($%.2x) (VF)", x, reg.V[x], y, reg.V[y]);
		u8 borrow = reg.V[x] > reg.V[y] ? 0 : 1;
		reg.V[x] = reg.V[y] - reg.V[x];
		*reg.VF = borrow;
	}break;
	case 0xe:
	{
		printf("SHL V%.1x($%.2x) (VF)", x, reg.V[y]);
		reg.V[x] = reg.V[y];
		u8 carry = (reg.V[x] & 0x80) >> 7;
		reg.V[x] <<= 1;
		*reg.VF = carry;
	}break;
	default:
		error_flags |= 0x01;
		printf("Unknown instruction");
		break;
	}
}

// key presses
void Chip8::exec_e(u16 data) {
	u8 opcode = data & 0xff;
	u8 x = (data & 0xf00) >> 8;

	switch (opcode) {
	case 0x9e:
		printf("SKP V%.1x($%.2x)", x, reg.V[x]);
		if (keys.key_isdown(reg.V[x])) {
			reg.PC += 2;
		}
		break;
	case 0xa1:
		printf("SKNP V%.1x($%.2x)", x, reg.V[x]);
		if (!keys.key_isdown(reg.V[x])) {
			reg.PC += 2;
		}
		break;
	default:
		error_flags |= 0x01;
		printf("Unknown instruction");
		break;
	}
}

void Chip8::exec_f(u16 data) {
	u8 opcode = data & 0xff;
	u8 x = (data & 0xf00) >> 8;

	switch (opcode) {
	case 0x07:
		printf("LD V%.1x, DT($%.2x)", x, reg.dt);
		reg.V[x] = reg.dt;
		break;
	case 0x0a:
		printf("LD V%.1x, K", x);
		key_dest = &reg.V[x];
		awaits_key = true;
	break;
	case 0x15:
		printf("LD DT, V%.1x($%.2x)", x, reg.V[x]);
		reg.dt = reg.V[x];
		break;
	case 0x18:
		printf("LD ST, V%.1x($%.2x)", x, reg.V[x]);
		reg.st = reg.V[x];
		break;
	case 0x1e:
		printf("ADD I($%.2x), V%.1x($%.2x)", reg.I, x, reg.V[x]);
		reg.I += reg.V[x];
		break;
	case 0x29:
		printf("LD F, V%.1x($%.2x)", x, reg.V[x]);
		reg.I = (u16)reg.V[x] * 5;
		break;
	case 0x33:
	{
		printf("LD B, V%.1x($%.2x)", x, reg.V[x]);
		u8 val = reg.V[x], div;
		for (int i = 2; i > -1; i--) {
			div = (u8)pow(10, i);
			write_8(reg.I + (2 - i), val / div);
			val -= (u8)(val / div) * div;
		}
	}break;
	case 0x55:
		for (u8 i = 0; i <= x; i++) {
			printf("\n\tLD [I]([$%.3x]), V%.1x($%.2x)", reg.I, i, reg.V[i]);
			write_8(reg.I++, reg.V[i]);
		}
		break;
	case 0x65:
		for (u8 i = 0; i <= x; i++) {
			printf("\n\tLD V%.1x, [I]([$%.3x])($%.2x)", i, reg.I, read_8(reg.I));
			reg.V[i] = read_8(reg.I++);
		}
		break;
	default:
		error_flags |= 0x01;
		printf("Unknown instruction");
		break;
	}
}

void Chip8::write_8(u16 addr, u8 val) {
	memory[addr] = val;
}

u8 Chip8::read_8(u16 addr) {
	return memory[addr];
}


// ----- MEMORY ACCESS -----

void Chip8::load(std::vector<char> &buf) {
	int i;
	for (i = 0; i < buf.size(); i++) {
		memory[CHIP8_PROG_LOAD_ADDR + i] = *(u8*)&buf[i];
	}
	reg.PC = CHIP8_PROG_LOAD_ADDR;

	mem_size = i + CHIP8_PROG_LOAD_ADDR;
}

u16 Chip8::next_instr() {
	u16 data = ((u16)memory[reg.PC] << 8) | memory[reg.PC + 1];

	printf("PC(%.4x): %.4x -> ", reg.PC, data);

	reg.PC += 2;
	return data;
}


// ----- STACK -----
void Chip8::stack_push(u16 data) {
	if (reg.SP == CHIP8_STACK_SIZE) {
		printf("\tSP($%.2x)\nERROR: Stackoverflow\n", reg.SP);
		error_flags |= 0x02;
	}
	else {
		stack[reg.SP] = data;
		reg.SP++;
	}
}
u16 Chip8::stack_pop() {
	if (reg.SP == 0) {
		printf("\tSP($%.2x)\nERROR: Nothing on stack\n", reg.SP);
		error_flags |= 0x04;
	}
	else {
		reg.SP--;
		return stack[reg.SP];
	}
}

// access
u8* Chip8::get_memory() {
	return memory;
}
int Chip8::get_mem_size() {
	return mem_size;
}

u16* Chip8::get_stack() {
	return stack;
}

registers* Chip8::get_registers() {
	return &reg;
}