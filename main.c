#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "gameInfo.h"
#include "performConnection.h"
#include "think.h"

// user guide
void printHelp() {
    printf("\nUSER GUIDE\n\n");
    printf("Required parameters:\n\n");
    printf("  -g <game-ID>            : ID of the game you want to join.\n");
    printf("The game ID must be exactly 13 characters long.\n\n");
    printf("  -p <{1, 2}>             : preferred player number (1 or 2).\n\n\n");
    printf("Optional parameters:\n\n");
    printf("  -c <configuration-data>: configuration data for the server connection.\n\n");
}

int main(int argc, char** argv) {

    char *players  = NULL;
    char *filePath = NULL;
    char *gameID   = NULL;

    // read command line parameters
    int ret;
    while ((ret = getopt(argc, argv, "g:p:c:")) != -1) {
        switch (ret) {
        case 'g':
            gameID = optarg;
            break;
        case 'p':
            players = optarg;
            break;
        case 'c':
            filePath = optarg;
            break;
        default:
            printHelp();
            break;
        }
    }

    // check if all parameters have been entered
    if (NULL == gameID || NULL == players) {
        fprintf(stderr, "Error: incorrect parameters.\n");
        printHelp();
        return EXIT_FAILURE;
    }

    // check if the game ID is exactly 13 characters long
    if (strlen(gameID) != 13) {
        fprintf(stderr, "Error: incorrect game ID.\n");
        printHelp();
        return EXIT_FAILURE;
    }

    // check if player number (1 or 2) has been entered correctly
    long numPlayers = 0;
    char *end       = NULL;
    numPlayers      = strtol(players, &end, 10);
    if (players == end || (numPlayers != 1L && numPlayers != 2L)) {
        fprintf(stderr, "Error: incorrect player number.\n");
        printHelp();
        return EXIT_FAILURE;
    }

    // check if configuration data have been entered
    char configFile[255] = "client.conf"; // standard configuration data
    if (filePath != NULL) {
        strncpy(configFile, filePath, 254);
        configFile[254] = '\0';
    }

    connectionProfile profile;
    if (readConfig(&profile, configFile) == NULL) {
        fprintf(stderr, "Error: could not read the configuration data.\n");
        return EXIT_FAILURE;
    }

    if (connectWith(&profile) == -1) {
        fprintf(stderr, "Server connection error.\n");
        return EXIT_FAILURE;
    }

    // Ask for SHM Segment
    int shmID = shmget(IPC_PRIVATE, sizeof(gameInfo), IPC_CREAT|0666);
    if (-1 == shmID) {
        perror("shmget");
        return EXIT_FAILURE;
    }

    // Attach SHM Segment to a gameInfo struct
    info = (gameInfo*) shmat(shmID, NULL, 0);
    if (NULL == info) {
        perror("shmat");
        return EXIT_FAILURE;
    }

    strncpy(info->gameID, gameID, GAMEIDLENGTH - 1);
    info->gameID[GAMEIDLENGTH - 1] = 0;
    info->playerNum = numPlayers;

    signal(SIGUSR1, think);

    if(pipe(fd) < 0) { // create a pipe
        fprintf(stderr, "Pipe error.\n");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    switch (pid) {
        case -1:
            printf("Error: fork().\n");
            break;
        case 0: // Child - Connector
            // Deinstall handler in Child Process
            signal(SIGUSR1, SIG_DFL);

            close(fd[1]); // close the pipe's write end

            // Set PID and PPID
            info->childPID  = getpid();
            info->parentPID = getppid();

            if (performConnection(&profile) == -1) {
                fprintf(stderr, "Connector: server connection error.\n");
                // Detach SHM Segment
                if (shmdt(info) == -1) {
                    perror("shmdt");
                    break;
                }
                // Mark SHM Segment for removal
                if (shmctl(shmID, IPC_RMID, 0) == -1) {
                    perror("shmctl");
                    break;
                }
                break;
            }

            close(profile.sock);

            // Detach SHM Segment
            if (shmdt(info) == -1) {
                perror("shmdt");
                break;
            }
            // Mark SHM Segment for removal
            if (shmctl(shmID, IPC_RMID, 0) == -1) {
                perror("shmctl");
                break;
            }
            break;
        default: // Parent - Thinker
            while (waitpid(pid, NULL, WNOHANG) == 0);
            close(fd[0]); // close the pipe's read end
            break;
    }
    close(fd[1]);

    free(profile.hostname);
    free(profile.gamekindname);

    return 0;
}
