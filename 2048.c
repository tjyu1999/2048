#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

int main(int argc, char *argv[]) {
    uint8_t board[SIZE][SIZE];
    uint32_t score = 0;
    char c;
    bool success;

    printf("\033[?25l\033[2J");
    signal(SIGINT, signal_callback_handler);
    initBoard(board);
    setBufferedInput(false);
    drawBoard(board, score);
    
    while (1) {
        c = getchar();
        if (c == -1) {
            puts("\nError! Cannot read keyboard input!");
            break;
        }
        
        switch (c) {
	    case 65: // up arrow
		success = moveUp(board, &score);
		break;
	    
	    case 66: // down arrow
		success = moveDown(board, &score);
		break;
		
            case 68: // left arrow
                success = moveLeft(board, &score);
                break;
	        
	    case 67: // right arrow
		success = moveRight(board, &score);
		break;
	    
	    default:
                success = false;
	}
	
	if (success) {
            drawBoard(board, score);
            usleep(150000);
            addRandom(board);
            drawBoard(board, score);
	    
	    if (gameEnded(board)) {
                printf("GAME OVER\n");
                break;
	    }
	}
	
        if (c == 'q') {
	    printf("QUIT? (y/n)\n");
            c = getchar();
	    if (c == 'y')
	        break;
	        
            drawBoard(board, score);
	}
	
	if (c == 'r') {
            printf("RESTART? (y/n)\n");
            c = getchar();
            if (c == 'y') {
	        initBoard(board);
		score = 0;
            }
            
            drawBoard(board, score);
	}
    }
    
    setBufferedInput(true);
    printf("\033[?25h\033[m");

    return EXIT_SUCCESS;
}