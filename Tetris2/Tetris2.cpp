// SpaceInvader.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#define UP		72
#define LEFT	75
#define RIGHT	77
#define DOWN	80
#define ESC		27

#define BLUE	RGB(68, 134, 240)
#define PURPLE  RGB(145, 108, 206)
#define RED		RGB(255, 0, 0)
#define GREEN	RGB(106, 194, 48)
#define YELLOW	RGB(255, 216, 0)
#define NAVY	RGB(56, 195, 232)
#define ORANGE	RGB(255, 106, 0)

// Высота поля
const int M = 20;
// Ширина поля
const int N = 10;
// Размер клетки
const int blockSize = 24;
const int delay = 300;

int gameField[M][N] = { 0 };

struct Point
{
	int x, y;
};

int figures[7][4] = {
	1,3,5,7, // I
	2,4,5,7, // Z
	3,5,4,6, // S
	3,5,4,7, // T
	2,3,5,7, // L
	3,5,7,6, // J
	2,3,4,5, // O
};

bool IsPlaceCorrect(Point *figure);

void RotateFigure(Point *figure);

void MoveFigure(Point *figure, char direction);

bool GameStep(HDC hdc, HPEN *hPen, Point *a, double *timer, double *delay, int *colorNum);

void MakeNewFigure(Point *a, int *colorNum, double *delay);

bool IsLineFilled();

void RedrawField(HDC hdc, HPEN *hPen);

void RedrawFigure(HDC hdc, HPEN *hPen, Point *a, int colorNum);

void InitHPen(HPEN *hPen);

bool IsGameOver();

void GameOver(HDC hdc, HPEN *hPen);

void StartMenu(int switcher);

void Game(int var);

void Help(int switcher);//функция помощи игроку

void TopChart();//функция "ЗАЛ СЛАВЫ" - отображает список лидеров

void PrepareWindow();

int main()
{
	PrepareWindow();

	Sleep(1500);

	StartMenu(1);

	return 0;
}

void StartMenu(int switcher)
{
	system("cls");
	switch (switcher)
	{
	case 1:
		printf("\n\n\n\n\n\n\n\n\n             <<  NEW GAME!  >>\n\n                   HELP!\n\n                  RESULTS\n\n                   EXIT");
		break;
	case 2:
		printf("\n\n\n\n\n\n\n\n\n                 NEW GAME!\n\n              <<   HELP!  >>\n\n                  RESULTS\n\n                   EXIT");
		break;
	case 3:
		printf("\n\n\n\n\n\n\n\n\n                 NEW GAME!\n\n                   HELP!\n\n               << RESULTS >>\n\n                   EXIT");
		break;
	case 4:
		printf("\n\n\n\n\n\n\n\n\n                 NEW GAME!\n\n                   HELP!\n\n                  RESULTS\n\n              <<   EXIT   >>\n");
		break;
	}
	int choice = _getch();
	if (choice == 224)
		choice = _getch();
	if (choice == 72)
		if (switcher != 1)
			StartMenu(switcher - 1);
		else
			StartMenu(4);
	if (choice == 80)
		if (switcher != 4)
			StartMenu(switcher + 1);
		else
			StartMenu(1);
	if (choice == 13 || choice == 32)
	{
		if (switcher == 1)
			Game(1);
		if (switcher == 2)
			Help(0);
		if (switcher == 3)
			//TopChart();
			if (switcher == 4)
				_exit(0);
	}
}

void Game(int var)
{
	Point figure[4];
	Point b[4];

	int colorNum;
	double delay = 300;

	srand(int(time(0)));
	system("mode con cols=80 lines=25");

	HWND hwnd = GetConsoleWindow();
	MoveWindow(hwnd, 100, 100, 257, 519, TRUE);
	HDC hdc = GetDC(hwnd);
	HPEN hPen[7];
	InitHPen(hPen);

	MakeNewFigure(figure, &colorNum, &delay);

	for (double timer = 0; ; timer += 35)
	{
		if (_kbhit())
		{
			switch (_getch())
			{
			case UP:
				RotateFigure(figure);
				break;
			case LEFT:
				MoveFigure(figure, LEFT);
				break;
			case RIGHT:
				MoveFigure(figure, RIGHT);
				break;
			case DOWN:
				delay /= 100;
				break;
			case ESC:
				GameOver(hdc, hPen);
				return;
				break;
			default:
				;
			}
		}

		if (timer > delay)
		{
			if (!GameStep(hdc, hPen, figure, &timer, &delay, &colorNum))
			{
				GameOver(hdc, hPen);
				return;
			}
		}

		if (IsLineFilled())
		{
			RedrawField(hdc, hPen);
		}
		RedrawFigure(hdc, hPen, figure, colorNum);
	}

	DeletePen(hPen);
	ReleaseDC(hwnd, hdc);
}

void Help(int switcher)
{
	system("cls");
	printf("ПЕРЕМЕЩЕНИЕ: СТРЕЛКИ ВВЕРХ/ВНИЗ\n ПОВОРОТ: СТРЕЛКА ВВЕРХ\n  ВЕРНУТЬСЯ: ESCAPE\n");
	if (_getch() == 27)
		StartMenu(2);
}


// Возможно ли такое положение фигуры
bool IsPlaceCorrect(Point *figure)
{
	for (int i = 0; i < 4; i++)
	{
		// Не вышли ли за пределы поля
		if (figure[i].x < 0 || figure[i].x >= N || figure[i].y >= M)	
		{
			return false;
		}
		// Не занято ли другой фигурой
		else if (gameField[figure[i].y][figure[i].x] != 0)	
		{
			return false;
		}
	}

	return true;
}

// Повернуть фигуру
void RotateFigure(Point *figure)
{
	Point tempFigure[4];
	for (int i = 0; i < 4; i++)
	{
		tempFigure[i] = figure[i];
	}
	// Поворачиваем относительно второго блока
	Point rotationCenter = figure[1]; 

	for (int i = 0; i < 4; i++)
	{
		int x = figure[i].y - rotationCenter.y;
		int y = figure[i].x - rotationCenter.x;

		figure[i].x = rotationCenter.x - x;
		figure[i].y = rotationCenter.y + y;
	}
	// Если положение невозможно, то возвращаем обратно
	if (!IsPlaceCorrect(figure))
	{
		for (int i = 0; i < 4; i++)
		{
			figure[i] = tempFigure[i];
		}
	}
}

// Сдвинуть фигуру
void MoveFigure(Point *figure, char direction)
{
	Point tempFigure[4];
	for (int i = 0; i < 4; i++)
	{
		tempFigure[i] = figure[i];
	}
	for (int i = 0; i < 4; i++)
	{
		switch (direction)
		{
		case LEFT:
			figure[i].x--;
			break;
		case RIGHT:
			figure[i].x++;
			break;
		default:
			;
		}
	}
	// Если положение невозможно, то возвращаем обратно
	if (!IsPlaceCorrect(figure))
	{
		for (int i = 0; i < 4; i++)
		{
			figure[i] = tempFigure[i];
		}
	}
}

// Шаг игры
bool GameStep(HDC hdc, HPEN *hPen, Point *figure, double *timer, double *stepDelay, int *colorNum)
{
	Point tempFigure[4];
	for (int i = 0; i < 4; i++)
	{
		tempFigure[i] = figure[i];
	}
	// Фигура "падает" на одну клетку
	for (int i = 0; i < 4; i++)
	{
		(figure[i].y)++;
	}
	// Если "упёрлись"
	if (!IsPlaceCorrect(figure))
	{
		// Делаем фигуру частью поля
		for (int i = 0; i < 4; i++)
		{
			gameField[tempFigure[i].y][tempFigure[i].x] = *colorNum;
		}
		if (!IsGameOver())
		{
			return false;
		}
		MakeNewFigure(figure, colorNum, stepDelay);
		RedrawField(hdc, hPen);
	}
	*timer = 0;
	return true;
}

// Создать новую фигуру
void MakeNewFigure(Point *figure, int *colorNum, double *stepDelay)
{
	*colorNum = 1 + rand() % 7;
	int figureType = rand() % 7;
	for (int i = 0; i < 4; i++)
	{
		figure[i].x = figures[figureType][i] % 2;
		figure[i].y = figures[figureType][i] / 2;
	}
	// Сбросим ускоритель падения
	*stepDelay = delay; 
}

// Заполнена ли линия
bool IsLineFilled()
{
	bool result = false;
	int line = M - 1;
	for (int i = M - 1; i > 0; i--)
	{
		int blocksInLine = 0;
		for (int j = 0; j < N; j++)
		{
			if (gameField[i][j] != 0)
			{
				blocksInLine++;
			}
			gameField[line][j] = gameField[i][j];
		}
		if (blocksInLine < N)
		{
			line--;
		}
		else
		{
			result = true;
		}
	}
	return result;
}

// Перерисовать поле
void RedrawField(HDC hdc, HPEN *hPen)
{
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	Rectangle(hdc, 0, 0, 320, 480);

	for (int i = 0; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{
			if (gameField[i][j] != 0)
			{
				SelectPen(hdc, hPen[gameField[i][j] - 1]);
				SelectObject(hdc, GetStockObject(WHITE_BRUSH));
				Rectangle(hdc, j * blockSize, i * blockSize, (j + 1) * blockSize, (i + 1) * blockSize);
			}
		}
	}
}

// Перерисовать фигуру
void RedrawFigure(HDC hdc, HPEN *hPen, Point *figure, int colorNum)
{
	int x;
	int y;

	SelectPen(hdc, hPen[colorNum - 1]);
	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	for (int k = 0; k < 4; k++)
	{
		x = figure[k].x * blockSize;
		y = figure[k].y * blockSize;
		BOOL bxt = Rectangle(hdc, x, y, x + blockSize, y + blockSize);
	}
	Sleep(40);
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	for (int k = 0; k < 4; k++)
	{
		x = figure[k].x * blockSize;
		y = figure[k].y * blockSize;
		BOOL bxt = Rectangle(hdc, x, y, x + blockSize, y + blockSize);
	}
}

void InitHPen(HPEN *hPen)
{
	hPen[0] = CreatePen(PS_INSIDEFRAME, 2, BLUE);
	hPen[1] = CreatePen(PS_INSIDEFRAME, 2, PURPLE);
	hPen[2] = CreatePen(PS_INSIDEFRAME, 2, RED);
	hPen[3] = CreatePen(PS_INSIDEFRAME, 2, GREEN);
	hPen[4] = CreatePen(PS_INSIDEFRAME, 2, YELLOW);
	hPen[5] = CreatePen(PS_INSIDEFRAME, 2, NAVY);
	hPen[6] = CreatePen(PS_INSIDEFRAME, 2, ORANGE);
}

bool IsGameOver()
{
	for (int i = 0; i < N; i++)
	{
		if (gameField[1][i] != 0)
		{
			return false;
		}
	}
	return true;
}

void GameOver(HDC hdc, HPEN *hPen)
{
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	BOOL bxt = bxt = Rectangle(hdc, 0, 0, 320, 480);

	printf("GAME OVER...\n");
}

void TopChart()
{

}

void PrepareWindow()
{
	system("mode con cols=43 lines=28");
	system("title Tetris");
	system("color 0A");
	HANDLE hCons = GetStdHandle(STD_OUTPUT_HANDLE);	//получение хендла
	CONSOLE_CURSOR_INFO cursor = { 1, false };	//число от 1 до 100 размер курсора в процентах; false\true - видимость
	SetConsoleCursorInfo(hCons, &cursor);	//применение заданных параметров курсора
	printf("\n\n\n\n\n\n\n\n\n\n\n     ____  ____  ____  ____  __  ____\n    (_  _)(  __)(_  _)(  _ )(  )/  __)\n      )(   ) _)   )(   )  <  )( \\__  \\\n     (__) (____) (__) (__\\_)(__)(____/\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}