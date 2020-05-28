#ifndef __GAMEINFO_H__
#define __GAMEINFO_H__

#include <stdbool.h>
#include <sys/types.h>

#define MAXPLAYERS 10
#define IDLENGTH 10
#define NAMELENGTH 20
#define GAMEIDLENGTH 14
#define MAXBOARDSIZE 50

typedef struct playerInfo {
    char id[IDLENGTH];
    char name[NAMELENGTH];
    bool ready;
    bool free;
} playerInfo;

typedef struct gameInfo {
    pid_t parentPID;
    pid_t childPID;
    char  gameID[GAMEIDLENGTH];
    long  totalPlayers;
    playerInfo players[MAXPLAYERS];
    bool think;
    int rows;
    int columns;
    char field[MAXBOARDSIZE][MAXBOARDSIZE];
    long moveDuration;
    bool player0Won;
    bool player1Won;
    int playerNum;
    bool gameover;
} gameInfo;

gameInfo *info;

void initializePlayers(gameInfo*);

// Add player to list
playerInfo *pushPlayer(gameInfo*, char*, char*, long);

#endif
