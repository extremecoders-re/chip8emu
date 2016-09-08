#include <stdio.h>
#include <conio.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include "keybindings.h"

// The RAM
unsigned char memory[4096];

// Registers from V0 to V15
unsigned char reg[16];

// The carry flag is the V15 register
#define VF 0xF

// Register I (Address register)
unsigned short I;

// The program counter
unsigned short PC;

// The stack
unsigned short stack[16];

// Stack Pointer
char SP;

// Delay timer
unsigned char DT;

// Sound timer
unsigned char ST;

// The keyboard indicating which keys are pressed
bool keyboard[16];

// The display screen
unsigned char display[32][64];

// Sprite data for characters from 0-F
unsigned const char sprites[16][8] = {
	{ 0xF0, 0x90, 0x90, 0x90, 0xF0 },	// 0
	{ 0x20, 0x60, 0x20, 0x20, 0x70 },	// 1
	{ 0xF0, 0x10, 0xF0, 0x80, 0xF0 },	// 2
	{ 0xF0, 0x10, 0xF0, 0x10, 0xF0 },	// 3	
	{ 0x90, 0x90, 0xF0, 0x10, 0x10 },	// 4		
	{ 0xF0, 0x80, 0xF0, 0x10, 0xF0 },	// 5		
	{ 0xF0, 0x80, 0xF0, 0x90, 0xF0 },	// 6		
	{ 0xF0, 0x10, 0x20, 0x40, 0x40 },	// 7		
	{ 0xF0, 0x90, 0xF0, 0x90, 0xF0 },	// 8
	{ 0xF0, 0x90, 0xF0, 0x10, 0xF0 },	// 9				
	{ 0xF0, 0x90, 0xF0, 0x90, 0x90 },	// A
	{ 0xE0, 0x90, 0xE0, 0x90, 0xE0 },	// B		
	{ 0xF0, 0x80, 0x80, 0x80, 0xF0 },	// C		
	{ 0xE0, 0x90, 0x90, 0x90, 0xE0 },	// D		
	{ 0xF0, 0x80, 0xF0, 0x80, 0xF0 },	// E	
	{ 0xF0, 0x80, 0xF0, 0x80, 0x80 }	// F
};


extern void setupDisplay();
extern void closeDisplay();
extern void displayBoard(unsigned char[32][64]);
extern void delay(int millisec);
extern bool isKeyPressed(char key);
extern char getKeyPress();

void readKeyboard()
{
	keyboard[0x1] = isKeyPressed(KEYPAD_1);
	keyboard[0x2] = isKeyPressed(KEYPAD_2);
	keyboard[0x3] = isKeyPressed(KEYPAD_3);
	keyboard[0xC] = isKeyPressed(KEYPAD_C);

	keyboard[0x4] = isKeyPressed(KEYPAD_4);
	keyboard[0x5] = isKeyPressed(KEYPAD_5);
	keyboard[0x6] = isKeyPressed(KEYPAD_6);
	keyboard[0xD] = isKeyPressed(KEYPAD_D);

	keyboard[0x7] = isKeyPressed(KEYPAD_7);
	keyboard[0x8] = isKeyPressed(KEYPAD_8);
	keyboard[0x9] = isKeyPressed(KEYPAD_9);
	keyboard[0xE] = isKeyPressed(KEYPAD_E);

	keyboard[0xA] = isKeyPressed(KEYPAD_A);
	keyboard[0x0] = isKeyPressed(KEYPAD_0);
	keyboard[0xB] = isKeyPressed(KEYPAD_B);
	keyboard[0xF] = isKeyPressed(KEYPAD_F);
}

//==========================================================
//
// Main execution loop
//
//==========================================================
void execute()
{
	unsigned short opcode;	//2 bytes
	bool updatePending = false;

	// Fetch, decode, execute loop
	while (true)
	{
		// Fetch next opcode
		opcode = (memory[PC] << 8) | memory[PC + 1];

		// Some #def's for simplicity
#define VX (reg[(opcode >> 8) & 0xF])
#define VY (reg[(opcode >> 4) & 0xF])

#ifdef _VERBOSE
#define LOG_OPCODE(msg) printf("0x%03X %s %04X\n", PC, msg, opcode)
#define LOG_UNKN(opcode) printf("0x%03X Undefined opcode %04X\n", PC, opcode)
#else
#define LOG_OPCODE(msg) 
#define LOG_UNKN(opcode)
#endif

		// Decode opcode
		switch ((opcode >> 12) & 0xF)	// First 4 bits
		{
		case 0:
			switch (opcode & 0xFF)
			{
			case 0xE0:
				//00E0 Clear Display
				memset(display, 0, sizeof(display));
				LOG_OPCODE("00E0 CLS");
				PC += 2;
				break;
			case 0xEE:
				// 00EE Return from subroutine
				PC = stack[SP--];
				LOG_OPCODE("00EE RET");
				break;
			default:
				LOG_UNKN(opcode);
				break;
			}
			break;

		case 1:
			// 1NNN Jump to address NNN
			PC = (opcode & 0x0FFF);
			LOG_OPCODE("1NNN JMP");
			break;

		case 2:
			// 2NNN Call sunroutine NNN
			stack[++SP] = PC + 2;
			PC = (opcode & 0x0FFF);
			LOG_OPCODE("2NNN CALL");
			break;

		case 3:
			// 3XNN Skips the next instruction if VX equals NN
			if (VX == (opcode & 0xFF)) PC += 2;
			PC += 2;
			LOG_OPCODE("3XNN SE");
			break;

		case 4:
			// 4XNN Skips the next instruction if VX doesn't equal NN
			if (VX != (opcode & 0xFF)) PC += 2;
			PC += 2;
			LOG_OPCODE("4XNN SNE");
			break;

		case 5:
			// 5XY0 Skips the next instruction if VX equals VY
			if (VX == VY) PC += 2;
			PC += 2;
			LOG_OPCODE("5XY0 SE");
			break;

		case 6:
			// 6XNN Sets VX to NN
			VX = opcode & 0xFF;
			PC += 2;
			LOG_OPCODE("6XNN LD");
			break;

		case 7:
			// 7XNN Adds NN to VX
			VX += opcode & 0xFF;
			PC += 2;
			LOG_OPCODE("7XNN ADD");
			break;

		case 8:
			switch (opcode & 0xF) // Last 4 bits
			{
			case 0:
				// 8XY0 Sets VX to the value of VY
				VX = VY;
				PC += 2;
				LOG_OPCODE("8XYO LD");
				break;

			case 1:
				// 8XY1 Sets VX to VX | VY
				VX |= VY;
				PC += 2;
				LOG_OPCODE("8XY1 OR");
				break;

			case 2:
				// 8XY2 Sets VX to VX & VY
				VX &= VY;
				PC += 2;
				LOG_OPCODE("8XY2 AND");
				break;

			case 3:
				// 8XY3 Sets VX to VX ^ VY
				VX ^= VY;
				PC += 2;
				LOG_OPCODE("8XY3 XOR");
				break;

			case 4:
				// 8XY4 Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
				reg[VF] = (VX + VY) > 0xFF;
				VX = (VX + VY) & 0xFF;
				PC += 2;
				LOG_OPCODE("8XY4 ADD");
				break;

			case 5:
				// 8XY5 VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
				reg[VF] = VX > VY;
				VX -= VY;
				PC += 2;
				LOG_OPCODE("8XY5 SUB");
				break;

			case 6:
				// 8XY6 Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
				reg[VF] = VX & 1;
				VX >>= 1;
				PC += 2;
				LOG_OPCODE("8XY6 SHR");
				break;

			case 7:
				// 8XY7 Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
				reg[VF] = VY > VX;
				VX -= VY;
				PC += 2;
				LOG_OPCODE("8XY7 SUBN");
				break;

			case 0xE:
				// 8XYE Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
				reg[VF] = (VX >> 7) & 1;
				VX <<= 1;
				PC += 2;
				LOG_OPCODE("8XYE SHL");
				break;

			default:
				LOG_UNKN(opcode);
				break;
			}
			break;

		case 9:
			// 9XY0 Skips the next instruction if VX doesn't equal VY
			if (VX != VY) PC += 2;
			PC += 2;
			LOG_OPCODE("9XY0 SNE");
			break;

		case 0xA:
			// ANNN Sets I to the address NNN
			I = opcode & 0xFFF;
			PC += 2;
			LOG_OPCODE("ANNN LD");
			break;

		case 0xB:
			// BNNN Jumps to the address NNN plus V0
			PC = (opcode & 0xFFF) + reg[0];
			LOG_OPCODE("BNNN JMP");
			break;

		case 0xC:
			// CXNN Sets VX to a random number, masked by NN
			VX = (rand() % 0xFF) & (opcode & 0xFF);
			PC += 2;
			LOG_OPCODE("CXNN RND");
			break;

		case 0xD:
		{
			// DXYN Sprites stored in memory at location in index register (I), maximum 8bits wide. 
			// Wraps around the screen. If when drawn, clears a pixel, register VF is set to 1 
			// otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen pixels)
			reg[VF] = 0;
			unsigned char n = opcode & 0xF;
			for (unsigned char row = 0; row < n; row++)
			{
				unsigned char line = memory[I + row]; // 8 bits

				// These 8 bits will be drawn at VX, VY
				for (unsigned char col = 0; col <= 7; col++)
				{
					unsigned char px = (line >> (7 - col)) & 1; // actually 1 bit
					/*if (VY + col > 63)
					{
						unsigned char oldPx = display[VY + col][(VX + row) - 64];
						display[VY + col][(VX + row) - 64] ^= px;
						unsigned char newPx = display[VY + col][(VX + row) - 64];
						if (oldPx == 1 && newPx == 0)	reg[VF] = 1;
					}
					else*/
					{
						unsigned char oldPx = display[VY + row][VX + col];
						display[VY + row][VX + col] ^= px;
						unsigned char newPx = display[VY + row][VX + col];
						if (oldPx == 1 && newPx == 0)	reg[VF] = 1;
					}
				}
			}
			PC += 2;
			updatePending = true;
			LOG_OPCODE("DXYN DRW");
			break;
		}

		case 0xE:
			switch (opcode & 0xFF)
			{
			case 0x9E:
				// EX9E Skips the next instruction if the key stored in VX is pressed
				readKeyboard();
				if (keyboard[VX])
					PC += 2;
				PC += 2;
				LOG_OPCODE("EX9E SKP");
				break;
			case 0xA1:
				// EXA1 Skips the next instruction if the key stored in VX isn't pressed
				readKeyboard();
				if (!keyboard[VX])
					PC += 2;
				PC += 2;
				LOG_OPCODE("EXA1 SKNP");
				break;
			default:
				LOG_UNKN(opcode);
				break;
			}
			break;

		case 0xF:
			switch (opcode & 0xFF)
			{
			case 0x07:
				// FX07 Sets VX to the value of the delay timer
				VX = DT;
				PC += 2;
				LOG_OPCODE("FX07 LD");
				break;
			case 0x0A:
				// FX0A A key press is awaited, and then stored in VX
				VX = getKeyPress();
				PC += 2;
				LOG_OPCODE("FX0A LD");
				break;

			case 0x15:
				// FX15 Sets the delay timer to VX.
				DT = VX;
				PC += 2;
				LOG_OPCODE("FX15 LD");
				break;

			case 0x18:
				// FX18 Sets the sound timer to VX
				ST = VX;
				PC += 2;
				LOG_OPCODE("FX18 LD");
				break;

			case 0x1E:
				// FX1E Adds VX to I
				I += VX;
				PC += 2;
				LOG_OPCODE("FX1E ADD");
				break;

			case 0x29:
				// FX29 Sets I to the location of the sprite for the character in VX. 
				// Characters 0F (in hexadecimal) are represented by a 4x5 font
				I = VX * 40;
				PC += 2;
				LOG_OPCODE("FX29 LD");
				break;

			case 0x33:
				// FX33 Stores the Binarycoded decimal representation of VX, with the most significant of three
				// digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2.
				// (In other words, take the decimal representation of VX, place the hundreds digit in memory at
				// location in I, the tens digit at location I+1, and the ones digit at location I+2.)
				memory[I] = VX / 100;
				memory[I + 1] = (VX % 100) / 10;
				memory[I + 2] = VX % 10;
				PC += 2;
				LOG_OPCODE("FX33 LD");
				break;

			case 0x55:
				// FX55 Stores V0 to VX in memory starting at address I
				for (unsigned char i = 0; i <= ((opcode >> 8) & 0xF); i++)
					memory[I + i] = reg[i];
				PC += 2;
				LOG_OPCODE("FX55 LD");
				break;

			case 0x65:
				// FX65 Fills V0 to VX with values from memory starting at address I
				for (unsigned char i = 0; i <= ((opcode >> 8) & 0xF); i++)
					reg[i] = memory[I + i];
				PC += 2;
				LOG_OPCODE("FX65 LD");
				break;

			default:
				LOG_UNKN(opcode);
				break;
			}
			break;
		default:
			LOG_UNKN(opcode);
			break;
		}

		if (DT > 0)	--DT;
		if (ST > 0) --ST;

		if (updatePending)
		{
			displayBoard(display);
			updatePending = false;
			delay(1000 / 24);
		}
#ifdef _VERBOSE
		getch();
#endif
	}
}


//==========================================================
//
// Initializes registers, memory and other data
//
//==========================================================
void setup()
{
	memset(memory, 0, sizeof(memory));
	memset(reg, 0, sizeof(reg));
	I = 0;
	PC = 0x200;
	memset(stack, 0, sizeof(stack));
	SP = -1;
	DT = 0;
	ST = 0;
	memset(keyboard, 0, sizeof(keyboard));
	memset(display, 0, sizeof(display));
	srand(time(NULL));
	memcpy(memory, sprites, sizeof(sprites));
}

//==========================================================
//
// Main
//
//==========================================================
int main(int argc, char **argv)
{
	FILE *fPtr;
	if (argc > 1)
	{
		// Open the file
		fPtr = fopen(argv[1], "rb");
		if (!fPtr) return -1;
		fseek(fPtr, 0, SEEK_END);
		long fSize = ftell(fPtr);

		if (fSize < 0xFFF - 0x200)
		{
			setup();
			setupDisplay();
			fseek(fPtr, 0, SEEK_SET);
			// Read file into memory
			fread(&(memory[0x200]), 1, fSize + 1, fPtr);
			execute();
		}
		fclose(fPtr);
	}
	return 0;
}