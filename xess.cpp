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

enum Player {White,Black,None};
enum Piece {Pawn,Tower,Knight,Bishop,King,Queen,NoPiece};

Player player = White;
int numMoves = 0;

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

string moveUp = "\33[11A";
string eraseLine = "\r\33[2K";

string markColor = "\33[7m";
string selectionColor = "\33[4m";
string tableColor = "\33[41m";
string toolbarColor = "\33[1m";
string B = "\33[30m";
string W = "\33[37m";
string n = "\33[0m";

string tableColor2[] = {
	"\33[41m",
	"\33[44m"
};

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
	"Trying to move a piece on itself!",
	"Trying to move blank space!",
	"Trying to move opponent piece!",
	"Trying to capture your own piece!",
	"Invalid move!"
};

string pieceLongNames[] = {
	"Pawn","Tower","Knigth","Bishop","King","Queen","None",
};

Player rankColor (int x,int y) {
	if (board[y][x] == __)
		return None;

	if (board[y][x] >= 0 && board[y][x] <= 5)
		return Black;
	else
		return White;
}

Piece rankPiece (int x, int y) {
	int p = board[y][x];

	if (p == Pt || p == Bt)
		return Tower;
	if (p == Pc || p == Bc)
		return Knight;
	if (p == Pb || p == Bb)
		return Bishop;
	if (p == Pa || p == Ba)
		return Queen;
	if (p == Pe || p == Be)
		return King;
	if (p == Pp || p == Bp)
		return Pawn;

	return NoPiece;
}

void draw () {
	int x,y;

	cout << "  ";
	cout << toolbarColor;

	if (player == White)
		cout << W+"White";
	else
		cout << B+"Black";

	cout << " #" << numMoves;

	cout << " " << pieceLongNames[(int)rankPiece(mx,my)];

	cout << " " << columns[cx] << lines[cy];

	cout << "     ";

	cout << n << nendl;

	for (y=0;y<8;y++) {
		if (cy == y) cout << selectionColor;
		if (my == y) cout << markColor;
		cout << lines[y] << n << " ";
		
		for (x=0;x<8;x++) {
			cout << tableColor2[(y*7+x)%2];

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

bool validLine (int x, int y, int ix, int iy,
			   int dx, int dy,
			   int len) {
	int i;
	for (i=0;i<len;i++) {
		x += ix;
		y += iy;

		if (x == dx && y == dy)
			return true;
		if (x < 0 || x > 7)
			return false;
		if (y < 0 || y > 7)
			return false;

		if (board[y][x] != __)
			return false;
	} 

	return false;
}

bool validLinePawn (int x, int y, int ix, int iy,
			   int dx, int dy,
			   int len) {
	int i;
	for (i=0;i<len;i++) {
		x += ix;
		y += iy;

		if (x == dx && y == dy && board[y][x] != __)
			return true;
		if (x < 0 || x > 7)
			return false;
		if (y < 0 || y > 7)
			return false;
	} 

	return false;
}

int validPawn (int x, int y,
			 int dx, int dy, Player pl) {
	if (pl == Black) {
		if (validLine(x,y,0,1,dx,dy,(y==1)?2:1))
			return -1;

		if (validLinePawn(x,y,1,1,dx,dy,1))
			return -1;
		if (validLinePawn(x,y,-1,1,dx,dy,1))
			return -1;

		return 4;
	} else {
		if (validLine(x,y,0,-1,dx,dy,(y==6)?2:1))
			return -1;

		if (validLinePawn(x,y,1,-1,dx,dy,1))
			return -1;
		if (validLinePawn(x,y,-1,-1,dx,dy,1))
			return -1;

		return 4;
	}
}

int validTower (int x,int y,
				int dx, int dy, Player pl) {
	if (validLine(x,y,-1,0,dx,dy,8))
		return -1;
	if (validLine(x,y,1,0,dx,dy,8))
		return -1;
	if (validLine(x,y,0,-1,dx,dy,8))
		return -1;
	if (validLine(x,y,0,1,dx,dy,8))
		return -1;

	return 4;
}

int validBishop (int x,int y,
				int dx, int dy, Player pl) {
	if (validLine(x,y,-1,1,dx,dy,8))
		return -1;
	if (validLine(x,y,1,-1,dx,dy,8))
		return -1;
	if (validLine(x,y,-1,-1,dx,dy,8))
		return -1;
	if (validLine(x,y,1,1,dx,dy,8))
		return -1;

	return 4;
}

int validKing (int x,int y,
				int dx, int dy, Player pl) {

	if (validLine(x,y,-1,0,dx,dy,1))
		return -1;
	if (validLine(x,y,1,0,dx,dy,1))
		return -1;
	if (validLine(x,y,0,-1,dx,dy,1))
		return -1;
	if (validLine(x,y,0,1,dx,dy,1))
		return -1;

	if (validLine(x,y,1,1,dx,dy,1))
		return -1;
	if (validLine(x,y,-1,1,dx,dy,1))
		return -1;
	if (validLine(x,y,1,-1,dx,dy,1))
		return -1;
	if (validLine(x,y,-1,-1,dx,dy,1))
		return -1;

	return 4;
}

int validQueen (int x,int y,
				int dx, int dy, Player pl) {

	if (validLine(x,y,-1,0,dx,dy,8))
		return -1;
	if (validLine(x,y,1,0,dx,dy,8))
		return -1;
	if (validLine(x,y,0,-1,dx,dy,8))
		return -1;
	if (validLine(x,y,0,1,dx,dy,8))
		return -1;

	if (validLine(x,y,1,1,dx,dy,8))
		return -1;
	if (validLine(x,y,-1,1,dx,dy,8))
		return -1;
	if (validLine(x,y,1,-1,dx,dy,8))
		return -1;
	if (validLine(x,y,-1,-1,dx,dy,8))
		return -1;

	return 4;
}

int validKnight (int x,int y,
				 int dx, int dy, Player pl) {

	if (validLine(x,y,-1,-2,dx,dy,8))
		return -1;
	if (validLine(x,y,1,-2,dx,dy,8))
		return -1;
	if (validLine(x,y,-2,-1,dx,dy,8))
		return -1;
	if (validLine(x,y,-2,1,dx,dy,8))
		return -1;

	if (validLine(x,y,2,1,dx,dy,8))
		return -1;
	if (validLine(x,y,2,-1,dx,dy,8))
		return -1;
	if (validLine(x,y,1,2,dx,dy,8))
		return -1;
	if (validLine(x,y,-1,2,dx,dy,8))
		return -1;

	return 4;
}

int isValid (int x, int y,
			 int dx, int dy,
			 Piece pi, Player pl) {
	switch (pi) {
		case Pawn:
			return validPawn(x,y,dx,dy,pl);
		break;
		case Tower:
			return validTower(x,y,dx,dy,pl);
		break;
		case King:
			return validKing(x,y,dx,dy,pl);
		break;
		case Queen:
			return validQueen(x,y,dx,dy,pl);
		break;
		case Bishop:
			return validBishop(x,y,dx,dy,pl);
		break;
		case Knight:
			return validKnight(x,y,dx,dy,pl);
		break;
		default:
		break;
	}

	return -1;
}

int move (int x, int y,
		  int dx, int dy) {
	if (dx == x && dy == y)
		return 0;

	if (rankColor(x,y) != player)
		return 2;

	if (rankColor(x,y) == rankColor(dx,dy))
		return 3;

	if (board[y][x] != __) { 
		int valid = isValid (x,y,dx,dy,rankPiece(x,y),rankColor(x,y));

		if (valid < 0) {
			board[dy][dx] = board[y][x];
			board[y][x] = __;

			player = (Player)!(bool)player;
			numMoves++;
			return -1;
		} else
			return valid;
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