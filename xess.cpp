#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
using namespace std;

#define Pp 0
#define Pt 1
#define Pc 2
#define Pb 3
#define Pe 4
#define Pa 5

#define Bp 6
#define Bt 7
#define Bc 8
#define Bb 9
#define Be 10
#define Ba 11

#define __ 12

// cursor
int cx,cy;
// mark
int mx,my;

int board [8][8] = {
	{Pt,Pc,Pb,Pa,Pe,Pb,Pc,Pt},
	{Pp,Pp,Pp,Pp,Pp,Pp,Pp,Pp},
	{__,__,__,__,__,__,__,__},
	{__,__,__,__,__,__,__,__},
	{__,__,__,__,__,__,__,__},
	{__,__,__,__,__,__,__,__},
	{Bp,Bp,Bp,Bp,Bp,Bp,Bp,Bp},
	{Bt,Bc,Bb,Ba,Be,Bb,Bc,Bt},
};

string moveUp = "\33[10A";
string eraseLine = "\r\33[2K";

string markColor = "\33[7m";
string selectionColor = "\33[4m";
string tableColor = "\33[41m";
string B = "\33[30m";
string W = "\33[37m";
string n = "\33[0m";

string piece [13] = {
	B+" p"+n,B+" t"+n,B+" h"+n,B+" b"+n,B+" k"+n,B+" q"+n,
	W+" p"+n,W+" t"+n,W+" h"+n,W+" b"+n,W+" k"+n,W+" q"+n,
	"  "+n,
};

string columns [8] = {
	"a","b","c","d","e","f","g","h"
};

string lines [8] = {
	"8","7","6","5","4","3","2","1"
};

string nendl = "\n\r";

string errors[] = {
	"Trying to move a piece on itself",
	"Trying to move blank space"
};

void draw () {
	int x,y;

	for (y=0;y<8;y++) {
		if (cy == y) cout << selectionColor;
		if (my == y) cout << markColor;
		cout << lines[y] << n << " ";
		
		for (x=0;x<8;x++) {
			cout << tableColor;

			if (cx == x && cy == y)
				cout << selectionColor;		
			if (mx == x && my == y)
				cout << markColor;		

			cout << piece[board[y][x]];
		}

		cout << nendl;
	}

	cout << nendl << "  ";
	
	for (x=0;x<8;x++) {
		if (cx == x) cout << selectionColor;		
		if (mx == x) cout << markColor;		
		cout << " ";
		cout << columns[x] << n;
	}

	cout << nendl;
}

int move (int x, int y,
		  int dx, int dy) {
	if (dx == x && dy == y)
		return 0;


	if (board[y][x] != __) { 
		board[dy][dx] = board[y][x];
		board[y][x] = __;
		return -1;
	} else {
		return 1;
	}
}

void waitEntry (string header) {
	char entry;

	do {
		cout << header << " [press ENTER]";
		entry = getc(stdin);
		cout << eraseLine;
	} while (entry != 13); 
}

struct termios prevAttr;

void nonCanonicalStdin () {
	struct termios serialAttr;
	
	bzero (&serialAttr, sizeof (struct termios));

	serialAttr.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                    INLCR | PARMRK | INPCK | ISTRIP | IXON);
	serialAttr.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	serialAttr.c_cflag &= ~(CSIZE | PARENB);
	serialAttr.c_cflag |= CS8;
	serialAttr.c_cc[VMIN]  = 1;
	serialAttr.c_cc[VTIME] = 0;

	tcgetattr (1, &prevAttr);
	tcsetattr (1,TCSANOW,&serialAttr);
}

void restoreStdin () {
	tcsetattr (1,TCSANOW,&prevAttr);
}

int main (void) {
	cout << "tXess version 0.1" << nendl;
	cout << "Copyright (C) 2013 by Felipe Tavares et al." << nendl;
	cout << nendl << nendl;

	nonCanonicalStdin();

	cx = cy = 0;

	char cmd;
	int err;

	while (true) {
		draw();

		cmd = getc(stdin);

		switch (cmd) {
			case 'q':
				goto endOfGame;

			case 'w':
				if (cy > 0)
					cy--;
			break;
			case 's':
				if (cy < 7)
					cy++;
			break;
			case 'a':
				if (cx > 0)
					cx--;
			break;
			case 'd':
				if (cx < 7)
					cx++;
			break;
			case ' ':
				mx = cx;
				my = cy;
			break;
			case 13:
				err = move (mx,my,cx,cy);
				if (err >= 0) {
					waitEntry(errors[err]);
				}
			break;
		}

		cout << eraseLine;
		cout << moveUp;
	}
	endOfGame:
	cout << eraseLine;

	restoreStdin();

	return 0;
}