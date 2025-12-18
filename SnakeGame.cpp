#include <iostream>
#include <chrono>
#include <windows.h>
#include <conio.h>
#include <Deque>

#define MAP_SIZE_X 25
#define MAP_SIZE_Y 25

#define Tick 60.0
#define WIN_COUNT 5

using namespace std;
using namespace chrono;

enum class MapType
{
	ROAD,
	WALL,
	PLAYER,
	APPLE,
};

enum InputKey
{
	Up,
	Left,
	Down,
	Right,
};

enum Color
{
	Black = 0,
	Red = 12,
	Blue = 9,
	Yellow = 14,
	White = 15,
	Green = 10,
};

struct Pos
{
	int x;
	int y;
};

const double fixedDeltaTime = 1.0 / Tick;

int map[MAP_SIZE_Y][MAP_SIZE_X] = {};
deque<Pos> tail;
InputKey key = Right;

void CursorPos(int x, int y)
{
	COORD Pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

void ConsoleColor(Color color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

bool CollisionWall(int x, int y)
{
	if (y <= 0 || y >= MAP_SIZE_Y - 1 || x <= 0 || (x * 2) >= (MAP_SIZE_X * 2) - 2)
		return 1;

	return 0;
}


void SetMap(int x, int y, MapType _map)
{
	map[y][x] = (int)_map;
	switch (_map)
	{
	case MapType::ROAD:
		ConsoleColor(Yellow);
		break;
	case MapType::WALL:
		ConsoleColor(Red);
		break;
	case MapType::PLAYER:
		ConsoleColor(Blue);
		break;
	case MapType::APPLE:
		ConsoleColor(Green);
		break;
	}

	CursorPos(x * 2, y);
	cout << "■";
}

void SetTail(int x, int y)
{
	SetMap(x, y, MapType::PLAYER);
}

void MapGenerator()
{
	for (int y = 0; y < MAP_SIZE_Y; y++)
	{
		for (int x = 0; x < MAP_SIZE_X; x++)
		{
			if (CollisionWall(x, y))
				map[y][x] = (int)MapType::WALL;
			else
				map[y][x] = (int)MapType::ROAD;
		}
	}
}

void MapInitRendering()
{
	for (int y = 0; y < MAP_SIZE_Y; y++)
	{
		for (int x = 0; x < MAP_SIZE_X; x++)
		{
			switch ((MapType)map[y][x])
			{
			case MapType::ROAD:
				ConsoleColor(Yellow);
				break;
			case MapType::WALL:
				ConsoleColor(Red);
				break;
			case MapType::PLAYER:
				ConsoleColor(Blue);
				break;
			}
			cout << "■";
			ConsoleColor(White);
		}
		cout << '\n';
	}
	cout << "W : ↑\nA : ←\nS : ↓\nD : →";
}

void AddTail(int x, int y)
{
	tail.push_front({ x, y });
	SetTail(x, y);
}

Pos RemoveBackTail()
{
	if (tail.empty())
		return { -1, -1 };
	Pos pos = tail.back();
	tail.pop_back();
	SetMap(pos.x, pos.y, MapType::ROAD);
	return pos;
}


void GameOver()
{
	system("cls");
	ConsoleColor(Red);
	cout << "패배했습니다!";
	ConsoleColor(White);
	exit(0);
}

void GameWin()
{
	system("cls");
	ConsoleColor(Blue);
	cout << "승리했습니다!";
	ConsoleColor(White);
	exit(0);
}

bool IsTail(int x, int y)
{
	if (CollisionWall(x, y))
		return 0;

	if (map[y][x] == (int)MapType::PLAYER)
		return 1;

	return 0;
}

void InputTail()
{
	if (_kbhit())
	{
		switch (_getch())
		{
		case 'w':
		case 'W':
			if (key != Down)
				key = Up;
			break;
		case 'a':
		case 'A':
			if (key != Right)
				key = Left;
			break;
		case 's':
		case 'S':
			if (key != Up)
				key = Down;
			break;
		case 'd':
		case 'D':
			if (key != Left)
				key = Right;
			break;
		}
	}
}

Pos Direction(Pos tailPos, int headX, int headY)
{
	switch (key)
	{
	case Up:
		if (!CollisionWall(headX, headY - 1))
		{
			tailPos.x = headX;
			tailPos.y = headY - 1;
		}
		else
			GameOver();
		break;
	case Left:
		if (!CollisionWall(headX - 1, headY))
		{
			tailPos.x = headX - 1;
			tailPos.y = headY;
		}
		else
			GameOver();
		break;
	case Down:
		if (!CollisionWall(headX, headY + 1))
		{
			tailPos.x = headX;
			tailPos.y = headY + 1;
		}
		else
			GameOver();
		break;
	case Right:
		if (!CollisionWall(headX + 1, headY))
		{
			tailPos.x = headX + 1;
			tailPos.y = headY;
		}
		else
			GameOver();
		break;
	}

	return tailPos;
}


void Move()
{
	Pos front;
	front = tail.front();
	Pos pos = RemoveBackTail();
	Pos head = Direction(pos, front.x, front.y);
	AddTail(head.x, head.y);
}

void Start()
{
	MapGenerator();
	MapInitRendering();
	AddTail((MAP_SIZE_X / 4), (MAP_SIZE_Y / 4));
	InputTail();
}

bool isGroundApple = false;
Pos apple = {};

void AppleRandomPos()
{
	if (isGroundApple == false)
	{
		isGroundApple = true;
		do
		{
			apple.x = rand() % (MAP_SIZE_X - 2) + 1; //1 ~ 23
			apple.y = rand() % (MAP_SIZE_Y - 2) + 1;
		} while (map[apple.y][apple.x] == (int)MapType::PLAYER);
		SetMap(apple.x, apple.y, MapType::APPLE);
	}
}

int eat = 0;

void AppleEat()
{
	if (isGroundApple == true && map[apple.y][apple.x] == (int)MapType::ROAD)
	{
		int tailX = tail.front().x;
		int tailY = tail.front().y;
		Pos head = Direction(tail.back(), tailX, tailY);

		AddTail(head.x, head.y);
		isGroundApple = false;
		eat++;
		CursorPos(0, MAP_SIZE_Y + 5 + eat);
		cout << eat << "/" << WIN_COUNT << "개 획득";
	}

	if (eat >= WIN_COUNT)
	{
		GameWin();
	}
}

int tick = 0;
float speed = 5.0f;

void FixedUpdate()
{
	tick += speed;
	if (tick >= (int)Tick)
	{
		AppleRandomPos();
		InputTail();
		Move();
		tick -= Tick;
	}
}

void Update()
{
	AppleEat();
}

void OnExit()
{

}

int main()
{
	srand(time(NULL));
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
	system("mode con:cols=100 lines=40");
	Start();


	time_point last = steady_clock::now();
	double sum = 0.0;
	while (true)
	{
		time_point now = steady_clock::now();
		double delta = duration<double>(now - last).count();
		last = now;

		sum += delta;

		while (sum >= fixedDeltaTime)
		{
			FixedUpdate();

			sum -= fixedDeltaTime;
		}

		Update();
	}

	OnExit();
}