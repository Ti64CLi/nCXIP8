#include <ngc.h>
#include <stdio.h>
#include "chip8.h"

#define SYSCALLBASEADDRESS 0xE0
#define ARITHMETICBASEADDRESS 0x00
#define KEYBASEADDRESS 0x9E
#define REGISTERBASEADDRESS 0x07

void (*opcodeList[16])(void) = { opcode_NULL };
void (*opcodeSyscalls[15])(void) = { opcode_NULL };
void (*opcodeArithmetics[15])(void) = { opcode_NULL };
void (*opcodeKeys[4])(void) = { opcode_NULL };
void (*opcodeRegisters[95])(void) = { opcode_NULL };

cpu_t *activeCPU;
screen_t *activeScreen;
key8_t *activeKeypad;

Gc gc;

t_key defaultNspireKeys[NUMBEROFKEY] = {
		KEY_NSPIRE_A, KEY_NSPIRE_B, KEY_NSPIRE_C, KEY_NSPIRE_D, //1 2 3 C
		KEY_NSPIRE_H, KEY_NSPIRE_I, KEY_NSPIRE_J, KEY_NSPIRE_K, //4 5 6 D
		KEY_NSPIRE_O, KEY_NSPIRE_P, KEY_NSPIRE_Q, KEY_NSPIRE_R, //7 8 9 E
		KEY_NSPIRE_V, KEY_NSPIRE_W, KEY_NSPIRE_X, KEY_NSPIRE_Y  //A 0 B F
};

uint8_t chip8Fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

int init_display(void) {
	gc = gui_gc_global_GC();
	
	gui_gc_begin(gc);
	gui_gc_setRegion(gc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	return 0;
}

int end_display(void) {
	free(activeScreen);
	
	gui_gc_finish(gc);
	
	return 0;
}

int init_CPU(cpu_t *cpu) {
	cpu->pc = 0x200;
	cpu->sp = 0;
	
	cpu->delayTimer = 0;
	cpu->soundTimer = 0;
	
	for(int i = 0; i < 80; i++)
		cpu->memory[i+0x50] = chip8Fontset[i]; //fontset needs to be between 0x0000 and 0x0200
	for(int i = 0; i < NUMBEROFREGISTER; i++)
		cpu->V[i] = 0;
	for(int i = 0; i < STACKLENGTH; i++)
		cpu->stack[i] = 0;
	
	init_opcode_list();
	
	return 0;
}

int init_active_CPU(void) {
	return init_CPU(activeCPU);
}

void set_active_CPU(cpu_t *cpu) {
	activeCPU = cpu;
}

int init_screen(screen_t *screen, uint8_t w, uint8_t h, uint32_t pixelOnColor, uint32_t pixelOffColor, uint32_t borderColor) {
	screen->width = w; //support for extended screen
	screen->height = h;
	screen->scale = 4;
	
	screen->pixelOnColor = pixelOnColor; //default = 0xFFFFFF
	screen->pixelOffColor = pixelOffColor; //default = 0x000000
	screen->borderColor = borderColor; //default = 0x666666
	
	screen->datas = calloc(w*h, sizeof(uint8_t));
	screen->drawFlag = true;
	
	return 0;
}

int init_active_screen(uint8_t w, uint8_t h, uint32_t pixelOnColor, uint32_t pixelOffColor, uint32_t borderColor) {
	return init_screen(activeScreen, w, h, pixelOnColor, pixelOffColor, borderColor);
}

void set_active_screen(screen_t *screen) {
	activeScreen = screen;
}

int refresh_screen(screen_t *screen) {
	//TODO
	return 0;
}

int init_keypad(key8_t *keypad) {
	for(int i = 0; i < NUMBEROFKEY; i++) {
		keypad->nspireKeys[i] = defaultNspireKeys[i]; //TODO : user can modify corresponding keys
		keypad->keysState[i] = 0;
	}
	return 0;
}

int init_active_keypad(void) {
	return init_keypad(activeKeypad);
}

void set_active_keypad(key8_t *keypad) {
	activeKeypad = keypad;
}

//general purposes functions
int open_chip8_rom(const char *filename) {
	FILE *rom = NULL;
	rom = fopen(filename, "rb");
	
	if(rom != NULL) {
		fread(&(activeCPU->memory[0x200]), sizeof(uint8_t) * (MEMORYLENGTH - 0x200), 1, rom);
		fclose(rom);
		
		return 0;
	}
	
	printf("Cannot open rom file\n");
	//TODO : display error screen
	
	return 1;
}

void fetch_opcode(void) {
	activeCPU->opcode = (activeCPU->memory[activeCPU->pc] << 8) | activeCPU->memory[activeCPU->pc + 1];
}

void init_opcode_list(void) {
	//opcodes 0x0XXX to 0xFXXX
	opcodeList[0x0] = opcode_SYSCALLS_list;
	opcodeList[0x1] = opcode_JP;
	opcodeList[0x2] = opcode_CALL;
	opcodeList[0x3] = opcode_SE_num;
	opcodeList[0x4] = opcode_SNE_num;
	opcodeList[0x5] = opcode_SE;
	opcodeList[0x6] = opcode_LD_num;
	opcodeList[0x7] = opcode_ADD_num;
	opcodeList[0x8] = opcode_ARITHMETICS_list;
	opcodeList[0x9] = opcode_SNE;
	opcodeList[0xA] = opcode_LDI;
	opcodeList[0xB] = opcode_JPA;
	opcodeList[0xC] = opcode_RND;
	opcodeList[0xD] = opcode_DRW;
	opcodeList[0xE] = opcode_KEYS_list;
	opcodeList[0xF] = opcode_REGISTERS_list;
	
	//opcodes 0x0XXX
	opcodeSyscalls[0xE0 - SYSCALLBASEADDRESS] = opcode_CLS;
	opcodeSyscalls[0xEE - SYSCALLBASEADDRESS] = opcode_RET;
	
	//ocpodes 0x8XXX
	opcodeArithmetics[0x00 - ARITHMETICBASEADDRESS] = opcode_LD;
	opcodeArithmetics[0x01 - ARITHMETICBASEADDRESS] = opcode_OR;
	opcodeArithmetics[0x02 - ARITHMETICBASEADDRESS] = opcode_AND;
	opcodeArithmetics[0x03 - ARITHMETICBASEADDRESS] = opcode_XOR;
	opcodeArithmetics[0x04 - ARITHMETICBASEADDRESS] = opcode_ADD;
	opcodeArithmetics[0x05 - ARITHMETICBASEADDRESS] = opcode_SUB;
	opcodeArithmetics[0x06 - ARITHMETICBASEADDRESS] = opcode_SHR;
	opcodeArithmetics[0x07 - ARITHMETICBASEADDRESS] = opcode_SUBN;
	opcodeArithmetics[0x0E - ARITHMETICBASEADDRESS] = opcode_SHL;
	
	//opcodes 0xEXXX
	opcodeKeys[0x9E - KEYBASEADDRESS] = opcode_SKP;
	opcodeKeys[0xA1 - KEYBASEADDRESS] = opcode_SKNP;
	
	//opcodes 0xFXXX
	opcodeRegisters[0x07 - REGISTERBASEADDRESS] = opcode_LD_delaytimer;
	opcodeRegisters[0x0A - REGISTERBASEADDRESS] = opcode_LD_key;
	opcodeRegisters[0x15 - REGISTERBASEADDRESS] = opcode_STO_delaytimer;
	opcodeRegisters[0x18 - REGISTERBASEADDRESS] = opcode_STO_soundtimer;
	opcodeRegisters[0x1E - REGISTERBASEADDRESS] = opcode_ADDI;
	opcodeRegisters[0x29 - REGISTERBASEADDRESS] = opcode_HEXI;
	opcodeRegisters[0x33 - REGISTERBASEADDRESS] = opcode_STO_BCD;
	opcodeRegisters[0x55 - REGISTERBASEADDRESS] = opcode_PUSH;
	opcodeRegisters[0x65 - REGISTERBASEADDRESS] = opcode_POP;
}

void emulate_cycle(void) {
	
}

void opcode_NULL(void) {}

void opcode_CLS(void) {
	memset(activeScreen->datas, 0, activeScreen->width * activeScreen->height);
}

void opcode_RET(void) {
	
}
void opcode_JP(void) {}
void opcode_CALL(void) {}
void opcode_SE_num(void) {}
void opcode_SNE_num(void) {}
void opcode_SE(void) {}
void opcode_LD_num(void) {}
void opcode_ADD_num(void) {}
void opcode_LD(void) {}
void opcode_OR(void) {}
void opcode_AND(void) {}
void opcode_XOR(void) {}
void opcode_ADD(void) {}
void opcode_SUB(void) {}
void opcode_SHR(void) {}
void opcode_SUBN(void) {}
void opcode_SHL(void) {}
void opcode_SNE(void) {}
void opcode_LDI(void) {}
void opcode_JPA(void) {}
void opcode_RND(void) {}
void opcode_DRW(void) {}
void opcode_SKP(void) {}
void opcode_SKNP(void) {}
void opcode_LD_delaytimer(void) {}
void opcode_LD_key(void) {}
void opcode_STO_delaytimer(void) {}
void opcode_STO_soundtimer(void) {}
void opcode_ADDI(void) {}
void opcode_HEXI(void) {}
void opcode_STO_BCD(void) {}
void opcode_PUSH(void) {}
void opcode_POP(void) {}

void opcode_SYSCALLS_list(void) {}
void opcode_ARITHMETICS_list(void) {}
void opcode_KEYS_list(void) {}
void opcode_REGISTERS_list(void) {}