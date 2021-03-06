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
// Задержка падения
const int delay = 300;

int g_gameField[M][N] = { 0 };

struct Point
{
	int x, y;
};

// Каждая фигура состоит из четырех блоков, выбранных из двумерного массива 2х8 следующим образом
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

void MoveFigure(Point *figure, const char direction);

bool DoGameStep(HDC hdc, HPEN *hPen, Point *a, double *timer, double *delay, int *colorNum);

void MakeNewFigure(Point *a, int *colorNum, double *delay);

bool DeleteFilledLines();

void RedrawField(HDC hdc, HPEN *hPen);

void RedrawFigure(HDC hdc, HPEN *hPen, Point *a, const int colorNum);

void InitHPen(HPEN *hPen);

bool IsGameOver();

void DoGameOver(HDC hdc, HPEN *hPen);

void StartMenu(int switcher);

void Game(int var);

void CallHelp(int switcher);//функция помощи игроку

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
			CallHelp(0);
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
				DoGameOver(hdc, hPen);
				return;
				break;
			default:
				;
			}
		}

		if (timer > delay)
		{
			if (!DoGameStep(hdc, hPen, figure, &timer, &delay, &colorNum))
			{
				DoGameOver(hdc, hPen);
				return;
			}
		}

		if (DeleteFilledLines())
		{
			RedrawField(hdc, hPen);
		}
		RedrawFigure(hdc, hPen, figure, colorNum);
	}

	DeletePen(hPen);
	ReleaseDC(hwnd, hdc);
}

void CallHelp(int switcher)
{
	system("cls");
	printf("ПЕРЕМЕЩЕНИЕ: СТРЕЛКИ ВВЕРХ/ВНИЗ\n ПОВОРОТ: СТРЕЛКА ВВЕРХ\n  ВЕРНУТЬСЯ: ESCAPE\n");
	if (_getch() == 27)
		StartMenu(2);
}


// Возможно ли такое положение фигуры относительно поля
bool IsPlaceCorrect(Point *figure)
{
	// Для каждого блока фигуры проверяем
	for (int i = 0; i < 4; i++)
	{
		// Не вышли ли за пределы поля по горизонтали слева и справа, по вертикали снизу
		if (figure[i].x < 0 || figure[i].x >= N || figure[i].y >= M)
		{
			return false;
		}
		// Не занято ли другой фигурой 
		else if (g_gameField[figure[i].y][figure[i].x] != 0)
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
void MoveFigure(Point *figure, const char direction)
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
bool DoGameStep(HDC hdc, HPEN *hPen, Point *figure, double *timer, double *stepDelay, int *colorNum)
{
	Point tempFigure[4];
	for (int i = 0; i < 4; i++)
	{
		tempFigure[i] = figure[i];
	}
	// Фигура опускается вниз относительно поля на одну клетку
	for (int i = 0; i < 4; i++)
	{
		(figure[i].y)++;
	}
	// Если фигура упёрлась в другую фигуру снизу
	if (!IsPlaceCorrect(figure))
	{
		// Делаем фигуру частью поля
		for (int i = 0; i < 4; i++)
		{
			g_gameField[tempFigure[i].y][tempFigure[i].x] = *colorNum;
		}
		// Проверяем не закончилась ли игра
		if (!IsGameOver())
		{
			return false;
		}
		// Создаём новую фигуру
		MakeNewFigure(figure, colorNum, stepDelay);
		// Перерисовываем поле
		RedrawField(hdc, hPen);
	}
	*timer = 0;
	return true;
}

// Создать новую фигуру
void MakeNewFigure(Point *figure, int *colorNum, double *stepDelay)
{
	// Выберем случайным образом цвет фигуры
	*colorNum = 1 + rand() % 7;
	// И её форму
	int figureType = rand() % 7;
	// Зададим начальные кординаты блокам фигуры
	for (int i = 0; i < 4; i++)
	{
		figure[i].x = figures[figureType][i] % 2;
		figure[i].y = figures[figureType][i] / 2;
	}
	// Сбросим ускоритель падения
	*stepDelay = delay;
}

// Удалим заполненные линии
bool DeleteFilledLines()
{
	bool result = false;
	// Вертикальная координата линии
	int lineIndex = M - 1;
	// Перенесём в новое поле только незаполненне линии
	for (int i = M - 1; i > 0; i--)
	{
		int blocksInLine = 0;
		// Посчитаем количество блоков в линии
		for (int j = 0; j < N; j++)
		{
			if (g_gameField[i][j] != 0)
			{
				blocksInLine++;
			}
			// Переносим линию в очищенное поле поблочно
			g_gameField[lineIndex][j] = g_gameField[i][j];
		}
		if (blocksInLine < N)
		{
			lineIndex--;
		}
		else
		{
			// Если линия заполнена
			result = true;
		}
	}
	return result;
}

// Перерисовать поле
void RedrawField(HDC hdc, HPEN *hPen)
{
	// Очистим экран
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	Rectangle(hdc, 0, 0, 320, 480);
	
	// Нарисуем поле
	for (int i = 0; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{
			if (g_gameField[i][j] != 0)
			{
				SelectPen(hdc, hPen[g_gameField[i][j] - 1]);
				SelectObject(hdc, GetStockObject(WHITE_BRUSH));
				Rectangle(hdc, j * blockSize, i * blockSize, (j + 1) * blockSize, (i + 1) * blockSize);
			}
		}
	}
}

// Перерисовать фигуру
void RedrawFigure(HDC hdc, HPEN *hPen, Point *figure, const int colorNum)
{
	// Нарисуем фигуру
	SelectPen(hdc, hPen[colorNum - 1]);
	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	for (int k = 0; k < 4; k++)
	{
		int x = figure[k].x * blockSize;
		int y = figure[k].y * blockSize;
		BOOL bxt = Rectangle(hdc, x, y, x + blockSize, y + blockSize);
	}
	Sleep(40);
	// Сотрём фигуру
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	for (int k = 0; k < 4; k++)
	{
		int x = figure[k].x * blockSize;
		int y = figure[k].y * blockSize;
		BOOL bxt = Rectangle(hdc, x, y, x + blockSize, y + blockSize);
	}
}

// Создадим палитру цветов фигур
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

// Проверка на конец игры
bool IsGameOver()
{
	for (int i = 0; i < N; i++)
	{
		if (g_gameField[1][i] != 0)
		{
			return false;
		}
	}
	return true;
}

//Конец игры
void DoGameOver(HDC hdc, HPEN *hPen)
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
	// Спрячем курсор
	HANDLE hCons = GetStdHandle(STD_OUTPUT_HANDLE);	//получение хендла
	CONSOLE_CURSOR_INFO cursor = { 1, false };	//число от 1 до 100 размер курсора в процентах; false\true - видимость
	SetConsoleCursorInfo(hCons, &cursor);	//применение заданных параметров курсора
	printf("\n\n\n\n\n\n\n\n\n\n\n     ____  ____  ____  ____  __  ____\n    (_  _)(  __)(_  _)(  _ )(  )/  __)\n      )(   ) _)   )(   )  <  )( \\__  \\\n     (__) (____) (__) (__\\_)(__)(____/\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}