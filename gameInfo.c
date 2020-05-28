#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "gameInfo.h"

static bool transformBool(long value) {
    if (1 == value) return true;
    return false;
}

// Initialize all players
void initializePlayers(gameInfo *info) {
    playerInfo *players = info->players;
    for (int i = 0; i < MAXPLAYERS; i++) {
        players[i].free = true;
    }
}

// Add new player to the list
playerInfo *pushPlayer(gameInfo *info, char *id, char *name, long ready) {
    // Find first free player
    playerInfo *player  = NULL;
    playerInfo *players = info->players;
    for (int i = 0; i < MAXPLAYERS; i++) {
        if (players[i].free == true) {
            player = &players[i];
            players[i].free = false;
            break;
        }
    }

    // If player list is full
    if (NULL == player) return NULL;

    // Set player data
    strncpy(player->id, id, IDLENGTH - 1);
    player->id[IDLENGTH - 1] = 0;
    strncpy(player->name, name, NAMELENGTH - 1);
    player->name[NAMELENGTH - 1] = 0;
    player->ready = transformBool(ready);
    return player;
}
