#include <os.h>
#include <stdint.h>

#include "chip8.h"
#include "menu.h"

#define EXE "nCXIP8"

int main(int argc, char *argv[]){
	if(argc == 1) { //if no file opened
		cfg_register_fileext("c8", EXE);
		cfg_register_fileext("ch8", EXE);
		cfg_register_fileext("chip8", EXE);
		
		cpu_debug("Extensions registered\n");
		
		show_msgbox("Info","File extensions registered.\nOpen a 'c8', 'ch8' or 'chip8'\nfile to use.");
		return 0;
	}
	
	cpu_t cpu;
	screen_t screen;
	key8_t keypad;
	
	set_active_CPU(&cpu);
	init_active_CPU();
	set_active_screen(&screen);
	init_active_screen(C8SCREENWIDTH, C8SCREENHEIGHT, 0xFFFFFF, 0x000000, 0x444444);
	set_active_keypad(&keypad);
	
	if(open_chip8_ROM(argv[1]))
		return 1;
	
	init_opcode_list();
	srand(time(NULL));
	
	init_display();
	
	int opcode_to_skip = 16; //default : 1
	
	while(!isKeyPressed(KEY_NSPIRE_ESC)) {
		for(int i = 0; i <= opcode_to_skip; i++) {
#ifdef DEBUG
			if(isKeyPressed(KEY_NSPIRE_ENTER)) {//DEBUG only
				char buf[12];
				sprintf(buf, "PC = 0x%4.4X", cpu.pc);
				cpu_debug(buf);
#endif
				if(!i)
					emulate_cycle();
#ifdef DEBUG
				while(isKeyPressed(KEY_NSPIRE_ENTER));
			}

			if(isKeyPressed(KEY_NSPIRE_S)) { //For debug purposes only
				for(int i = 0; i < 8; i++)
					printf("V%1.1X = 0x%2.2X  V%1.1X = 0x%2.2X\n", i, cpu.V[i], i+8, cpu.V[i+8]);
				printf("PC = 0x%4.4X  SP = 0x%1.1X\n", cpu.pc, cpu.sp);
				printf("I = 0x%4.4X\n", cpu.I);
				while(isKeyPressed(KEY_NSPIRE_D));
			}
#endif
			refresh_active_screen();
			msleep(1);
		}
		
		//refresh_active_screen();
		
		//msleep(1);
			
		//TODO : keys and timers
		
		set_keypad_state(); //too slow ?
		//Timers : use Timer 1 or 2 (0x900C0000 or 0x900D0000)
		//first support of timers -> to continue
		if(cpu.delayTimer > 0)
			cpu.delayTimer--;
		if(cpu.soundTimer > 0) {
			if(--cpu.soundTimer == 0)
				printf("BIIIIP!!\n");
		}
	}
	
	end_display();
	
	cpu_debug("Exit");
	
	return 0;
}