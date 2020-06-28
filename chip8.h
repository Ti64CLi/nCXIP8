#ifndef CPU_H
#define CPU_H

#define NUMBEROFREGISTER 16
#define NUMBEROFKEY 16
#define MEMORYLENGTH 4096
#define STACKLENGTH 16

typedef uint16_t reg16_t;
typedef uint8_t reg8_t;
	
typedef struct {
	reg16_t opcode;
	reg16_t pc;
	reg8_t sp;
	reg16_t I;
	reg8_t V[NUMBEROFREGISTER];
	
	reg8_t delayTimer;
	reg8_t soundTimer;
	
	uint8_t memory[MEMORYLENGTH];
	uint16_t stack[STACKLENGTH];
} cpu_t;

typedef struct {
	reg8_t width;
	reg8_t height;
	reg8_t scale;
	
	uint32_t pixelOnColor;
	uint32_t pixelOffColor;
	uint32_t borderColor;
	
	uint8_t *datas;
	bool drawFlag;
} screen_t;

typedef struct {
	t_key nspireKeys[NUMBEROFKEY];
	uint8_t keysState[NUMBEROFKEY];
} key8_t;

int init_display(void);
int end_display(void);

int init_CPU(cpu_t *); //init the given CPU
int init_active_CPU(void); //init the CPU currently used
void set_active_CPU(cpu_t *); //change the working CPU
int init_screen(screen_t *, uint8_t, uint8_t, uint32_t, uint32_t, uint32_t); //init the given screen
int init_active_screen(uint8_t, uint8_t, uint32_t, uint32_t, uint32_t); //init the screen currently used
void set_active_screen(screen_t *); //change the working screen
int refresh_screen(screen_t *);
int init_keypad(key8_t *); //init the given keypad
int init_active_keypad(void); //init the keypad currently used
void set_active_keypad(key8_t *); //change the working keypad

//suffixe _num when the opcode exists with both reg and immediate value as operand
//any other suffixes for further informations

//CHIP8 Instructions
void opcode_NULL(void);
void opcode_CLS(void);
void opcode_RET(void);
void opcode_JP(void);
void opcode_CALL(void);
void opcode_SE_num(void);
void opcode_SNE_num(void);
void opcode_SE(void);
void opcode_LD_num(void);
void opcode_ADD_num(void);
void opcode_LD(void);
void opcode_OR(void);
void opcode_AND(void);
void opcode_XOR(void);
void opcode_ADD(void);
void opcode_SUB(void);
void opcode_SHR(void);
void opcode_SUBN(void);
void opcode_SHL(void);
void opcode_SNE(void);
void opcode_LDI(void);
void opcode_JPA(void);
void opcode_RND(void);
void opcode_DRW(void);
void opcode_SKP(void);
void opcode_SKNP(void);
void opcode_LD_delaytimer(void);
void opcode_LD_key(void);
void opcode_STO_delaytimer(void);
void opcode_STO_soundtimer(void);
void opcode_ADDI(void);
void opcode_HEXI(void);
void opcode_STO_BCD(void);
void opcode_PUSH(void);
void opcode_POP(void);

void opcode_SYSCALLS_list(void);
void opcode_ARITHMETICS_list(void);
void opcode_KEYS_list(void);
void opcode_REGISTERS_list(void);

//Super CHIP-(4)8 Instructions
//TODO

//general functions
int open_chip8_ROM(const char *);
void fetch_opcode(void);
void init_opcode_list(void); //TODO : add SCHIP-(4)8 opcode
void emulate_cycle(void);

#endif