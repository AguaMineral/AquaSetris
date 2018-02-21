//Aqua Tetris, el tetris de caca hecho por aqua
#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>

#include <stdio.h>

using namespace std;

//Las piezas
wstring piezas[7];

//el campo del tetris
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

//screen dimension
int nScreenWidth = 80; //Columnas
int nScreenHeigth = 30; //Filas

						//Funcion de rotacion de las piezas
int Rotate(int px, int py, int r)
{
	int pi = 0;
	switch (r % 4)//entre 4 porque 0, 90, 180, 270 
	{				// tienes resto 0,1,2,3
	case 0: pi = py * 4 + px; break; //0 grados
	case 1: pi = 12 + py - (px * 4); break;//90 grados
	case 2: pi = 15 - (py * 4) - px; break;//180 grados
	case 3: pi = 3 - py + (px * 4); break;//270 grados
	}
	return pi;
}

bool EncajaLaPieza(int nPiezas, int nRotation, int nPosX, int nPosY)
{
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			//Posicion de la pieza
			int pi = Rotate(px, py, nRotation);

			int fi = (nPosY + py)* nFieldWidth + (nPosX + px);
			//Collision detection
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					if (piezas[nPiezas][pi] != L'.' && pField[fi] != 0)
						return false; //No encaja en el primer intento
				}
			}
		}
	return true;
}
int main()
{
	//Las piezas creadas
	piezas[0].append(L"..X...X...X...X.");
	piezas[1].append(L"..X..XX...X.....");
	piezas[2].append(L".....XX..XX.....");
	piezas[3].append(L"..X..XX..X......");
	piezas[4].append(L".X...XX...X.....");
	piezas[5].append(L".X...X...XX.....");
	piezas[6].append(L"..X...X..XX.....");

	pField = new unsigned char[nFieldWidth*nFieldHeight];//el inicio del array del campo
	for (int x = 0; x < nFieldWidth; x++)
		for (int y = 0; y < nFieldHeight; y++)
			pField[y*nFieldWidth + x ] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	//Función para confgurar la consola como un buffer
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeigth];
	for (int i = 0; i < nScreenWidth*nScreenHeigth; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);//son funciones de windows
	DWORD dwBytesWritten = 0;

	//Game logic
	bool bGameOver = false;

	int nCurrentPieza = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;//en el medio y en el tope
	int nCurrentY = 0;


	bool bKey[4];
	bool bRotateContinua = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	int nPieceCount = 0;
	int nScore = 0;

	//Para eliminar los iguales
	vector<int> vLines;

	//Imprimir CADA frame
	while (!bGameOver)
	{
		//Timing ========================================
		this_thread::sleep_for(50ms);//game tic
		nSpeedCounter++;//aumento de la velocidad por tic
		bForceDown = (nSpeedCounter == nSpeed);


		//Input =========================================
		//Comprueba si alguna de las cuatro teclas está presionada, si es asi retorna true.
		for (int k = 0; k < 4; k++)								// R   L   D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		//que caigan las piezas poco a poco


		//Game logic ====================================
		if (bForceDown)
		{	//si la pieza encaja en el espacio, seguir bajando
			if (EncajaLaPieza(nCurrentPieza, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++;
			else
			{
				//encajar la pieza en el campo
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (piezas[nCurrentPieza][Rotate(px, py, nCurrentRotation)] == L'X')
							pField[(nCurrentY + py)*nFieldWidth + (nCurrentX + px)] = nCurrentPieza + 1;
				
				//Aumentar velocidad de las piezas
				nPieceCount++;
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10) nSpeed--;

				//Comprobar si hago linea
				for ( int py = 0; py < 4; py++)
					if(nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine)
						{
							//conertir la linea al simbolo igual
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;//el símbolo de igual
							vLines.push_back(nCurrentY + py);
						}
					}

				//Score 
				nScore += 25;
				if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

				//siguiente pieza
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPieza = rand() % 7;

				//si no caben mas GAME OVER
				bGameOver = !EncajaLaPieza(nCurrentPieza, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nSpeedCounter = 0;
		}


		//MOVIMIENTO CON LAS TECLAS
		//A la derecha
		if (bKey[0])
		{
			if(EncajaLaPieza(nCurrentPieza, nCurrentRotation, nCurrentX + 1, nCurrentY))
			{
				nCurrentX = nCurrentX + 1;
			}
		}
		// a la izquierda
		if (bKey[1])
		{
			if(EncajaLaPieza(nCurrentPieza, nCurrentRotation, nCurrentX - 1, nCurrentY))
			{
				nCurrentX = nCurrentX - 1;
			}
		}
		//abajo
		if (bKey[2])
		{
			if(EncajaLaPieza(nCurrentPieza, nCurrentRotation, nCurrentX, nCurrentY + 1))
			{
				nCurrentY = nCurrentY + 1;
			}
		}

		//rotar piezas con restriccion a presion de la tecla continua, para que no gire como una peonza 
		if (bKey[3])
		{
			nCurrentRotation += (!bRotateContinua && EncajaLaPieza(nCurrentPieza, nCurrentRotation, nCurrentX, nCurrentY));
			bRotateContinua = true;
		}
		else
			bRotateContinua = false;


		//Render output =================================
		//Imprimir el capmpo
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				screen[(y + 2)*nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y*nFieldWidth + x]];

		//Imprimir la pieza que no está encajada en el campo
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
			{	//se comprueba si está la pieza, y se adquiere la rotacion
				//correcta con la funcion rotate para que te de el index correcto
				//comprueba tmb si es == X, entonces habrá que dibujar en pantalla, si no, es espacio vacío.
				if (piezas[nCurrentPieza][Rotate(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py + 2)* nScreenWidth + (nCurrentX + px + 2)] = nCurrentPieza + 65;
			}//el interior del if dibuja la pieza en su letra correspondiente y con su offset de posicion + los offset del campo(2, 2 en cada coordenada).

		// Imprimir Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"PUNTOS %8d", nScore);
		
		//Comprobar si hay lineas
		if (!vLines.empty())
		{
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeigth, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			//Eliminar lineas y bajar todas las piezas que no lo son
			for(auto &v : vLines)
			{
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}
			}
			vLines.clear();
		}
		//imprimir frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeigth, { 0,0 }, &dwBytesWritten);
		
		
	}
	CloseHandle(hConsole);
	cout << "\n\n\t\tLa cagaste colegga, tus puntillos: " << nScore << endl << endl;;
	system("pause");
	return 0;
}

