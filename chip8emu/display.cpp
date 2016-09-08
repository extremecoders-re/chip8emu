#include "curses.h"
#include "keybindings.h"

#pragma comment(lib, "pdcurses.lib")

/*
Initializes the Display. Must be called first
*/
void setupDisplay()
{
#ifdef _VERBOSE
	return;
#endif
	// Init curses
	initscr();

	// Resize console window
	resize_term(34, 66);

	// Make the cursor invisible
	curs_set(0);

	// Bright text
	attron(A_BOLD);

	// Init colors
	start_color();
	init_pair(1, COLOR_YELLOW, COLOR_BLACK);
	
	attron(COLOR_PAIR(1));
	noecho();	//Turn off displaying chars
}

/*
Closes open curses window. 
*/
void closeDisplay()
{
#ifdef _VERBOSE
	return;
#endif
	endwin();
}

/*
Displays the board on screen
*/
void displayBoard(unsigned char display[32][64])
{
#ifdef _VERBOSE
	return;
#endif
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			if (display[i][j]) mvaddch(1+i, 1+j, '#');
			else mvaddch(1+i, 1+j, ' ');
		}
	}
	refresh();
}

void delay(int millisec)
{
	napms(millisec);
}

bool isKeyPressed(char key)
{
	nodelay(stdscr, TRUE);
	char input = getch();
	if (input == ERR) return false;
	if (input >= 97) input -= 32;
	return key == input;
}

char getKeyPress()
{
	nodelay(stdscr, FALSE);
	char input;

	while (true)
	{
		input = getch();
		if (input >= 97) input -= 32;

		switch (input)
		{
			case KEYPAD_1:
			case KEYPAD_2:
			case KEYPAD_3:
			case KEYPAD_C:
			case KEYPAD_4:
			case KEYPAD_5:
			case KEYPAD_6:
			case KEYPAD_D:
			case KEYPAD_7:
			case KEYPAD_8:
			case KEYPAD_9:
			case KEYPAD_E:
			case KEYPAD_A:
			case KEYPAD_0:
			case KEYPAD_B:
			case KEYPAD_F:
				return input;
		}
	}	
}