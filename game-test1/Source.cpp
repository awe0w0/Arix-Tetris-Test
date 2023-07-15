#include <iostream>
using namespace std;

#include <cstdlib>
#include <thread>
#include <Windows.h>

wstring tetromino[7];
int nFieldWidth = 12;
int nFileHight = 18;
unsigned char* pFiled = nullptr;

int nScreenWidth = 80;
int nScreenHight = 30;

int Rotate(int px, int py, int r) {
	switch (r%4)
	{
	case 0:return py * 4 + px;
	case 1:return 12 + py - (px * 4);
	case 2:return 15 - (py * 4) - px;
	case 3:return 3 - py + (px * 4);
	}
	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
	for(int px = 0; px < 4;px++)
		for (int py = 0; py < 4; py++) {
			//Get index into piece
			int pi = Rotate(px, py, nRotation);

			//Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);
			if(nPosX + px >= 0&&nPosX +px<nFieldWidth)
				if (nPosY + py >= 0 && nPosY + py < nFileHight) {
					if (tetromino[nTetromino][pi] == L'X' && pFiled[fi] != 0)
						return false;//fail on first hit
				}
		}

	return true;
}

int main() {
	srand(time(NULL));
	//creat assets
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");

	tetromino[0].append(L"....");
	tetromino[0].append(L".XX.");
	tetromino[0].append(L".X..");
	tetromino[0].append(L".X..");

	pFiled = new unsigned char[nFieldWidth * nFileHight];
	for (int x = 0; x < nFieldWidth; x++)
		for(int y = 0;y <nFileHight;y++)
			pFiled[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFileHight - 1) ? 9 : 0;
	
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHight];
	for (int i = 0; i < nScreenHight * nScreenWidth; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;	



	bool bGameOver = false;

	int nCurrentPiece = 1;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bKey[4];
	bool bRotateHold = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;

	while (!bGameOver) {
		
		//GAME_TIMING==============================
		this_thread::sleep_for(50ms);
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		//INPUT ===================================
		for (int k = 0; k < 4; k++)                              //R   L   D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		//GAME_LOGIC===============================

		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		if (bKey[3]) {
			nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true;
		}
		else {
			bRotateHold = false;
		}

		if (bForceDown) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++;//it can , so do it
			}
			else {
				//Lock the current piece in the field
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
							pFiled[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;


				//Check have we got ant lines

				//Choice next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				//if piece does not fit
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nSpeedCounter = 0;
		}

		//RENDER_OUTPUT============================
		
		//Draw Field
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFileHight; y++)
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pFiled[y * nFieldWidth + x]];

		//Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

		//Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}