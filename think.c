#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "gameInfo.h"
#include "think.h"
#include "performConnection.h"

#define MOVESIZE 12

static void printBoard();
static char columnToLetter (int);
static char rowToChar (int);
static char *computeMove();
static void prepareMove(char column, char row, int j, int i, char* move);
static void moveUp(char column, char row, int j, int i, char* move);
static void moveDown(char column, char row, int j, int i, char* move);
static void moveLeft(char column, char row, int j, int i, char* move);
static void moveRight(char column, char row, int j, int i, char* move);
static void moveUpRight(char column, char row, int j, int i, char* move);
static void moveDownRight(char column, char row, int j, int i, char* move);
static void moveUpLeft(char column, char row, int j, int i, char* move);
static void moveDownLeft(char column, char row, int j, int i, char* move);

int nwrite = 0;
bool moveFound = false;
int counter = 1;
char computer = '\0';
char opponent = '\0';

static char* computeMove() { // very simple idea: find the first valid move
    char* move    = calloc(MOVESIZE, sizeof(char));
    char row      = '\0';
    char column   = '\0';
    moveFound   = false;
    if (info->playerNum == 1) {
        computer = 'B';
        opponent = 'W';
    } else {
        computer = 'W';
        opponent = 'B';
    }
    while (moveFound != true) {
        for (int i = 0; i < info->rows; i++) {
            for (int j = 0; j < info->columns; j++) {
                if (info->field[i][j] == opponent) {
                    //printf("i=%i and j=%i\n", i, j); // for debugging
                    switch(counter) {
                        case 1: if (info->field[i+1][j] == computer && moveFound != true) { // up
                                    moveUp(column, row, j, i, move);
                                }
                                counter = 2;
                        case 2: if (info->field[i-1][j] == computer && moveFound != true) { // down
                                    moveDown(column, row, j, i, move);
                                }
                                counter = 3;
                        case 3: if (info->field[i][j+1] == computer && moveFound != true) { // left
                                    moveLeft(column, row, j, i, move);
                                }
                                counter = 4;
                        case 4: if (info->field[i][j-1] == computer && moveFound != true) { // right
                                    moveRight(column, row, j, i, move);
                                }
                                counter = 5;
                        case 5: if (info->field[i+1][j-1] == computer && moveFound != true) { // up right
                                    moveUpRight(column, row, j, i, move);
                                }
                                counter = 6;
                        case 6: if (info->field[i-1][j-1] == computer && moveFound != true) { // down right
                                    moveDownRight(column, row, j, i, move);
                                }
                                counter = 7;
                        case 7: if (info->field[i+1][j+1] == computer && moveFound != true) { // up left
                                    moveUpLeft(column, row, j, i, move);
                                }
                                counter = 8;
                        case 8: if (info->field[i-1][j+1] == computer && moveFound != true) { // down left
                                    moveDownLeft(column, row, j, i, move);
                                }
                        counter = 1;
                    }
                }
            }
            if (moveFound != false) {
                break;
            }
        }
        if (moveFound != true) {
            printf("Move not found!\n");
            break;
        }
    }
    return move;
}

static void prepareMove(char column, char row, int j, int i, char *move) {
    column = columnToLetter(j);
    row = rowToChar(i);
    strncpy(move, "PLAY ", MOVESIZE);
    strncat(move, &column, 1);
    strncat(move, &row, 1);
    strncat(move, "\n", 1);
}

static void moveUp(char column, char row, int j, int i, char* move) {
    //printf("Trying up at %i and %i\n", i, j); // for debugging
    for (int k = i - 1; k >= 0; k--){
        if (info->field[k][j] == '*') {
            prepareMove(column, row, j+1, k+1, move);
            moveFound = true;
            //printf("UP\n"); // for debugging
            break;
         } else if (info->field[k][j] == computer) {
             break;
         }
     }
}

static void moveDown(char column, char row, int j, int i, char* move) {
    //printf("Trying down at %i and %i\n", i, j); // for debugging
    for (int k = i + 1; k < info->rows; k++) {
        if (info->field[k][j] == '*') {
            prepareMove(column, row, j+1, k+1, move);
            moveFound = true;
            //printf("DOWN\n"); // for debugging
            break;
        } else if (info->field[k][j] == computer) {
            break;
        }
    }
}

static void moveLeft(char column, char row, int j, int i, char* move) {
    //printf("Trying left at %i and %i\n", i, j); // for debugging
    for (int k = j - 1; k >= 0; k--){
        if (info->field[i][k] == '*') {
            prepareMove(column, row, k+1, i+1, move);
            moveFound = true;
            //printf("LEFT\n"); // for debugging
            break;
        } else if (info->field[i][k] == computer) {
            break;
        }
    }
}

static void moveRight(char column, char row, int j, int i, char* move) {
    //printf("Trying right at %i and %i\n", i, j); // for debugging
    for (int k = j; k < info->columns; k++) {
        if (info->field[i][k] == '*') {
            prepareMove(column, row, k+1, i+1, move);
            moveFound = true;
            //printf("RIGHT\n"); // for debugging
            break;
        } else if (info->field[i][k] == computer) {
            break;
        }
    }
}

static void moveUpRight(char column, char row, int j, int i, char* move) {
    //printf("Trying up right at %i and %i\n", i, j); // for debugging
    while (i >= 0 && j < info->columns) {
        if (info->field[i][j] == '*') {
            prepareMove(column, row, j+1, i+1, move);
            moveFound = true;
            //printf("UP RIGHT\n");
            break;
        } else if (info->field[i][j] == computer) {
            break;
        }
        i--;
        j++;
    }
}

static void moveDownRight(char column, char row, int j, int i, char* move) {
    //printf("Trying down right at %i and %i\n", i, j); // for debugging
    while (i < info->rows && j < info->columns) {
        if (info->field[i][j] == '*') {
            prepareMove(column, row, j+1, i+1, move);
            moveFound = true;
            //printf("DOWN RIGHT\n");
            break;
        } else if (info->field[i][j] == computer) {
            break;
        }
        i++;
        j++;
    }
}

static void moveUpLeft(char column, char row, int j, int i, char* move) {
    //printf("Trying up left at %i and %i\n", i, j);
    while (i >= 0 && j >= 0) {
        if (info->field[i][j] == '*') {
            prepareMove(column, row, j+1, i+1, move);
            moveFound = true;
            //printf("UP LEFT\n");
            break;
        } else if (info->field[i][j] == computer) {
            break;
        }
        i--;
        j--;
    }
}

static void moveDownLeft(char column, char row, int j, int i, char* move) {
    //printf("Trying down left at %i and %i\n", i, j);
    while (i < info->rows && j >= 0) {
        if (info->field[i][j] == '*') {
            prepareMove(column, row, j+1, i+1, move);
            moveFound = true;
            //printf("DOWN LEFT\n");
            break;
        } else if (info->field[i][j] == computer) {
            break;
        }
        i++;
        j--;
    }
}

void think(int signal) {
    if (signal == SIGUSR1 && info->think == true) {
        printBoard();
        char *readyMove = computeMove();
        nwrite = strlen("PLAY XY\n") + 1;
        if ((write(fd[1], readyMove, nwrite)) != nwrite) {
            printf("Pipe fehler beim Schreiben!\n");
            free(readyMove);
        }
        memset(readyMove, 0, MOVESIZE);
        info->think = false;
        free(readyMove);
    } else if (info->gameover && signal == SIGUSR1) {
        if (info->player0Won && !info->player1Won) {
            if (info->playerNum == 1) {
                printf("S: %s won.\n", info->players[0].name);
            } else {
                printf("S: %s won.\n", info->players[1].name);
            }
        } else if (info->player1Won && !info->player0Won) {
            if (info->playerNum == 1) {
                printf("S: %s won.\n", info->players[1].name);
            } else {
                printf("S: %s won.\n", info->players[0].name);
            }
        } else if (info->player0Won && info->player1Won) {
            printf("S: It's a draw.\n");
        }
        printf("C: Here's the final board:\n");
        printBoard();
        printf("S: Exiting.\n");
        info->gameover = false;
    }
}

static void printBoard() {
    printf("   A B C D E F G H   \n");
    printf(" +-----------------+ \n");
    for (int i = 0; i < info->rows; i++) {
        printf("%d ", i + 1);
        printf("|");
        for (int j = 0; j < info->columns; j++) {
            if (info->field[i][j] == '*') printf(".");
            if (info->field[i][j] == 'B') printf("#");
            if (info->field[i][j] == 'W') printf("O");
            printf("|");
        }
        printf(" %d", i + 1);
        printf("\n");
    }
    printf(" +-----------------+ \n");
    printf("   A B C D E F G H   \n");
    if (info->playerNum == 1) {
        printf("C: Your pieces: #\nC: Your opponent's pieces: O\nC: Empty fields: .\n");
    } else {
        printf("C: Your pieces: O\nC: Your opponent's pieces: #\nC: Empty fields: .\n");
    }
}

static char columnToLetter (int column) {
    switch (column) {
        case 1: return 'A';
        case 2: return 'B';
        case 3: return 'C';
        case 4: return 'D';
        case 5: return 'E';
        case 6: return 'F';
        case 7: return 'G';
        case 8: return 'H';
        default: return '0';
    }
}

static char rowToChar (int row) {
    switch (row) {
        case 1: return '8';
        case 2: return '7';
        case 3: return '6';
        case 4: return '5';
        case 5: return '4';
        case 6: return '3';
        case 7: return '2';
        case 8: return '1';
        default: return '0';
    }
}
