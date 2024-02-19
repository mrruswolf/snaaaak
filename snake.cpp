#include <time.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

#define DELAY 30000
#define TIMEOUT 10

typedef enum
{
    LEFT,
    RIGHT,
    UP,
    DOWN
} direction_type;

typedef struct point
{
    int x;
    int y;
} point;

int x = 0,
    y = 0,
    maxY = 0,
    maxX = 0,
    nextX = 0,
    nextY = 0,
    tailLength = 5,
    score = 0;

bool gameOver = false;
point frame[1800];

direction_type currentDir = RIGHT;
point snakeParts[255] = {};
point food;

void createFood()
{
    food.x = (rand() % (maxX - 20)) + 10;

    food.y = (rand() % (maxY - 10)) + 5;
}

void drawSnake(point drawPoint)
{
    attron(COLOR_PAIR(1));
    mvprintw(drawPoint.y, drawPoint.x, "o");
}
void drawFood(point drawPoint)
{
    attron(COLOR_PAIR(2));
    mvprintw(drawPoint.y, drawPoint.x, "@");
}


void init()
{
    
    srand(time(NULL));
    initscr(); // создание окна
    noecho();  // Отсутствие сигнала о нажатии клавиш
    keypad(stdscr, TRUE);
    cbreak();
    timeout(TIMEOUT);
    curs_set(FALSE); 

    getmaxyx(stdscr, maxY, maxX);
    currentDir = RIGHT;
    tailLength = 5;
    gameOver = false;
    score = 0;

    clear(); // очищает экран

    // Устанавливаем начальные координаты змеи
    int j = 0;
    for (int i = tailLength; i >= 0; i--)
    {
        point currPoint;

        currPoint.x = i;
        currPoint.y = maxY / 2; // Начните игру с середины экрана по оси y

        snakeParts[j] = currPoint;
        j++;
    }

    createFood();

    refresh();
}

void shiftSnake()
{
    point tmp = snakeParts[tailLength - 1];

    for (int i = tailLength - 1; i > 0; i--)
    {
        snakeParts[i] = snakeParts[i - 1];
    }

    snakeParts[0] = tmp;
}

void Wall(int h, int w)
{ 
    // отстройка  Стены
    int s = 0;
    for (int x = 1; x < w - 1; x++)
    {
        frame[s].y = 1;
        frame[s++].x = x;
        mvaddch(frame[s - 1].y, frame[s - 1].x, '#');
    }
    for (int x = 1; x < w - 1; x++)
    {
        frame[s].y = h - 2;
        frame[s++].x = x;
        mvaddch(frame[s - 1].y, frame[s - 1].x, '#');
    }
    for (int y = 1; y < h - 1; y++)
    {
        frame[s].y = y;
        frame[s++].x = 1;
        mvaddch(frame[s - 1].y, frame[s - 1].x, '#');
    }
    for (int y = 1; y < h - 1; y++)
    {
        frame[s].y = y;
        frame[s++].x = w - 2;
        mvaddch(frame[s - 1].y, frame[s - 1].x, '#');
    }
}

void drawScreen()
{
    // Очищает экран,помещаем потом все функции рисования
    clear();

    Wall(maxY, maxX);
    // рисуем змею
    for (int i = 0; i < tailLength; i++)
    {
        drawSnake(snakeParts[i]);
    }

    // рисуем яблоки
    drawFood(food);

    // рисуем счет
    attron(COLOR_PAIR(3));
    mvprintw(1, 2, "Score: %i", score);

    // обновление ncurses
    refresh();

    // Задержка между движениями
    usleep(DELAY);
}

/* основаная часть */
int main(int argc, char *argv[])
{
    init();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    Wall(maxY, maxX);
    int ch;
    while (1)
    {
        // Глобальная переменная stdscr создается при вызове функции initscr().
        // Это говорит нам о максимальном размере окна терминала в любой момент времени
        getmaxyx(stdscr, maxY, maxX);
        //завершение игры
        if (gameOver)
        {
            clear();
            mvprintw(maxY / 2, maxX / 2 - 6, "Game Over!");
            mvprintw((maxY / 2) + 2, maxX / 2 - 4, "RETRY?");
             mvprintw((maxY / 2) + 3, maxX / 2 - 9, "Y - Yes   N - No");
            refresh();                                                     //обновление экрана
            sleep(3);
            timeout(15000);                          
            ch = getch();
            if ((ch == 'Y') || (ch == 'y')) {
                init();
            }
            else {
                nocbreak();
                echo();    
                endwin();                                 //завершение работы ncurses
                exit(0);   
            }
        }

        /* Обрабатываем ввод*/
        ch = getch();

        if ((ch == 'd' || ch == 'D' || ch == KEY_RIGHT) && (currentDir != RIGHT && currentDir != LEFT))
        {
            currentDir = RIGHT;
        }
        else if ((ch == 'a' || ch == 'A' || ch == KEY_LEFT) && (currentDir != RIGHT && currentDir != LEFT))
        {
            currentDir = LEFT;
        }
        else if ((ch == 's' || ch == 'S' || ch == KEY_DOWN) && (currentDir != UP && currentDir != DOWN))
        {
            currentDir = DOWN;
        }
        else if ((ch == 'w' || ch == 'W' || ch == KEY_UP) && (currentDir != UP && currentDir != DOWN))
        {
            currentDir = UP;
        }

        /* Движение */
        nextX = snakeParts[0].x;
        nextY = snakeParts[0].y;

        if (currentDir == RIGHT)
            nextX++;
        else if (currentDir == LEFT)
            nextX--;
        else if (currentDir == UP)
            nextY--;
        else if (currentDir == DOWN)
            nextY++;

        if (nextX == food.x && nextY == food.y)
        {
            point tail;
            tail.x = nextX;
            tail.y = nextY;

            snakeParts[tailLength] = tail;

            if (tailLength < (maxX - 2) * (maxY - 4))
                tailLength++;
            else
                tailLength = 5; // Если мы исчерпали массив, то просто сбрасываем длину змейки, но игрок продолжает накапливать свой счет. :)

            score += 5;
            createFood();
        }
        else
        {
            // рисуем змейку на экране
            for (int i = 0; i < tailLength; i++)
            {
                if (nextX == snakeParts[i].x && nextY == snakeParts[i].y)
                {
                    gameOver = true;
                    break;
                }
            }

            // Мы собираемся установить хвост в качестве новой головы
            snakeParts[tailLength - 1].x = nextX;
            snakeParts[tailLength - 1].y = nextY;
        }
        usleep(DELAY);
        // Сдвиг всех частей змеи
        shiftSnake();

        // Игра заканчивается, если игрок ударяется о края экрана
        if (((nextX + 3) >= (maxX) || (nextX - 3) < 0) || ((nextY + 3) >= (maxY) || (nextY - 3) < 0))
        {
            gameOver = true;
        }

        /* рисуем экран */
        drawScreen();
    }
}
