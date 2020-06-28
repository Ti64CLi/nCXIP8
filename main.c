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
		
		printf("Extensions registered\n");
		
		show_msgbox("Info","File extensions registered.\nOpen a 'c8', 'ch8' or 'chip8'\nfile to use.");
		return 0;
	}
	
	
	
	return 0;
}