#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <time.h>

#define SIZE 4

void getColor(uint8_t value, uint8_t *fg, uint8_t *bg) {
    uint8_t colors[] = {225, 8, 234, 223, 234, 222, 234, 215, 234, 208, 234, 210, 234, 204, 234, 203, 234, 196, 234, 81, 234, 39, 234, 25};
    *fg = colors[0 + value * 2];
    *bg = colors[1 + value * 2];
}

uint8_t getDigitCount(uint32_t number) {
    uint8_t count = 0;
    do {
        number /= 10;
	count += 1;
    } while (number);
    
    return count;
}

void drawBoard(uint8_t board[SIZE][SIZE], uint32_t score) {
    uint8_t x;
    uint8_t y;
    uint8_t fg;
    uint8_t bg;
    
    printf("\033[H");
    printf("%d pts\n", score);
    
    for (y = 0; y < SIZE; ++y) {
	for (x = 0; x < SIZE; ++x) {
            getColor(board[x][y], &fg, &bg);
            printf("\033[38;5;%d;48;5;%dm", fg, bg);
            printf("       ");
            printf("\033[m");
	}	
        printf("\n");
        
        for (x = 0; x < SIZE; ++x) {
            getColor(board[x][y], &fg, &bg);
            printf("\033[38;5;%d;48;5;%dm", fg, bg);
            
            if (board[x][y] != 0) {
                uint32_t number = 1 << board[x][y];
                uint8_t t = 7 - getDigitCount(number);
                printf("%*s%u%*s", t - t / 2, "", number, t / 2, "");
            }
            else
                printf("   ·   ");

            printf("\033[m");
        }
        printf("\n");
        
        for (x = 0; x < SIZE; ++x) {
            getColor(board[x][y], &fg, &bg);
            printf("\033[38;5;%d;48;5;%dm", fg, bg);
            printf("       ");
            printf("\033[m");
        }
        printf("\n");
    }
    
    printf("\n       ←,↑,→,↓ or q       \n");
    printf("\033[A");
}

uint8_t findTarget(uint8_t array[SIZE], uint8_t x, uint8_t stop) {
    if (x == 0)
        return x;
    
    for (uint8_t t = x - 1; ; --t) {
        if (array[t] != 0) {
            if (array[t] != array[x])
                return t + 1;
			
	    return t;
        }
        else {
            if (t == stop)
                return t;	
        }
    }
    return x;
}

bool slideArray(uint8_t array[SIZE], uint32_t *score) {
    uint8_t t;
    uint8_t stop = 0;
    bool success = false;

    for (uint8_t x = 0; x < SIZE; ++x) {
        if (array[x] != 0) {
            uint8_t t = findTarget(array, x, stop);
            
            if (t != x) {
                if (array[t] == 0)
                    array[t] = array[x];
                else if (array[t] == array[x]) {
                    array[t]++;
                    *score += (uint32_t)1 << array[t];
                    stop = t + 1;
                }

                array[x] = 0;
                success = true;
            }
        }
    }
    
    return success;
}

void rotateBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t n = SIZE;
    uint8_t tmp;
    
    for (uint8_t i = 0; i < n / 2; ++i)	{
        for (uint8_t j = i; j < n - i - 1; ++j) {
            tmp = board[i][j];
            board[i][j] = board[j][n - i - 1];
            board[j][n - i - 1] = board[n - i - 1][n - j - 1];
            board[n - i - 1][n - j - 1] = board[n - j - 1][i];
            board[n - j - 1][i] = tmp;
        }
    }
}

bool moveUp(uint8_t board[SIZE][SIZE], uint32_t *score) {
    bool success = false;
    for (uint8_t x = 0; x < SIZE; ++x)
        success |= slideArray(board[x], score);
    
    return success;
}

bool moveDown(uint8_t board[SIZE][SIZE], uint32_t *score) {
    bool success;
    
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board, score);
    rotateBoard(board);
    rotateBoard(board);
    
    return success;
}

bool moveLeft(uint8_t board[SIZE][SIZE], uint32_t *score) {
    bool success;
    
    rotateBoard(board);
    success = moveUp(board, score);
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);

    return success;
}

bool moveRight(uint8_t board[SIZE][SIZE], uint32_t *score) {
    bool success;
    
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board, score);
    rotateBoard(board);
    
    return success;
}

bool findPairDown(uint8_t board[SIZE][SIZE]) {
    bool success = false;
    
    for (uint8_t x = 0; x < SIZE; ++x) {
        for (uint8_t y = 0; y < SIZE - 1; ++y) {
            if (board[x][y] == board[x][y + 1])
                return true;
        }
    }

    return success;
}

uint8_t countEmpty(uint8_t board[SIZE][SIZE]) {
    uint8_t count = 0;

    for (uint8_t x = 0; x < SIZE; ++x) {
        for (uint8_t y = 0; y < SIZE; ++y) {
            if (board[x][y] == 0)
                ++count;
        }
    }
    
    return count;
}

bool gameEnded(uint8_t board[SIZE][SIZE]) {
    bool ended = true;

    if (countEmpty(board) > 0)
        return false;
    if (findPairDown(board))
        return false;
    rotateBoard(board);
    
    if (findPairDown(board))
        ended = false;
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    
    return ended;
}

void addRandom(uint8_t board[SIZE][SIZE]) {
    static bool initialized = false;
    uint8_t x;
    uint8_t y;
    uint8_t len = 0;
    uint8_t list[SIZE * SIZE][2];
    
    if (!initialized) {
        srand(time(NULL));
        initialized = true;
    }

    for (x = 0; x < SIZE; ++x) {
        for (y = 0; y < SIZE; ++y) {
            if (board[x][y] == 0) {
                list[len][0] = x;
                list[len][1] = y;
                ++len;
            }
        }
    }

    if (len > 0) {
        uint8_t r = rand() % len;
        x = list[r][0];
        y = list[r][1];
        board[x][y] = (rand() % 10) / 9 + 1;
    }
}

void initBoard(uint8_t board[SIZE][SIZE]) {    
    for (uint8_t x = 0; x < SIZE; ++x) {
        for (uint8_t y = 0; y < SIZE; ++y)
            board[x][y] = 0;
    }
    
    addRandom(board);
    addRandom(board);
}

void setBufferedInput(bool enable) {
    static bool enabled = true;
    static struct termios old;
    struct termios new;
    
    if (enable && !enabled) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        enabled = true;
    }
    else if (!enable && enabled) {
        tcgetattr(STDIN_FILENO, &new);
        old = new;
        new.c_lflag &= (~ICANON & ~ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new);
        enabled = false;
    }
}

void signalCallback(int signum) {
    printf("       TERMINATED       \n");
    setBufferedInput(true);
    printf("\033[?25h\033[m");
    exit(signum);
}
