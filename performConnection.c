#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>

#include "performConnection.h"
#include "config.h"
#include "gameInfo.h"
#include "utility.h"

#define BUFSIZE 1024

static ssize_t sendMessage(int, const char*);
static ssize_t readMessage(int, char*, const size_t);
static char* toString(int);

int performConnection(connectionProfile *profile) {

    fd_set rfds;
    struct timeval timeout;
    int retval = 0;
    int socketSelect = 0;
    int pipeSelect = 0;

    char *buffer = calloc(BUFSIZE, sizeof(char));
    if (NULL == buffer) return -1;

    char *moveBuffer = calloc(BUFSIZE, sizeof(char)); // Buffer for the pipe
    if (NULL == moveBuffer) return -1;

    int sock = profile->sock;
    char playerID[IDLENGTH];
    char playerName[NAMELENGTH];

    ssize_t nread = 0;
    int counter = 0;
    bool counterStart = false; // Flag to start reading the board
    bool counterMoveDuration = true; // print max move duration only once
    bool gameover = false;

    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    FD_SET(fd[0], &rfds);

    if (counterMoveDuration != true) {
        if (info->moveDuration >= 3000) {
            timeout.tv_sec = 3;
            timeout.tv_usec = 0;
        } else if (info->moveDuration < 3000 && info->moveDuration >= 2000) {
            timeout.tv_sec = 2; // just to be on the safe side
            timeout.tv_usec = 0;
        } else if (info->moveDuration < 2000 && info->moveDuration >= 1000) {
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
        } else {
            timeout.tv_sec = 0;
            timeout.tv_usec = 500;
        }
    } else {
        timeout.tv_sec = 3; // start value
        timeout.tv_usec = 0;
    }

    retval = select(6, &rfds, NULL, NULL, &timeout);

    if (retval == -1) {
        perror("Select() error.\n");
    } else if (retval) {
        socketSelect = FD_ISSET(sock, &rfds);
        pipeSelect   = FD_ISSET(fd[0], &rfds);
    } else {
        perror("Timeout!\n");
    }

    while (readMessage(sock, buffer, BUFSIZE) > 0 && socketSelect) { // While receiving messages
        char **lines = splitBy(buffer, "\n"); // Split message into lines
        if (NULL == lines) break;

        bool error = true;
        char **line = lines;

        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);
        FD_SET(fd[0], &rfds);

        socketSelect = FD_ISSET(sock, &rfds); // has to be re-initialised due to the loop

        while (*line != NULL) { // Until all lines are processed
            error = false;

            socketSelect = FD_ISSET(sock, &rfds); // same as above

            if (strncmp(*line, "+ MNM", 5) == 0) {
                memset(buffer, 0, BUFSIZE);
                strncpy(buffer, "VERSION 2.3\n", BUFSIZE);
                sendMessage(sock, buffer);
                printf("C: Welcome!\nC: Sending client version to server.\n");
            } else if (strncmp(*line, "+ Client", 8) == 0) {
                printf("S: Client version accepted.\nS: Waiting for the game ID.\n");
                memset(buffer, 0, BUFSIZE);
                strncpy(buffer, "ID ", BUFSIZE);
                strncat(buffer, info->gameID, strnlen(info->gameID, 13));
                strncat(buffer, "\n", 1);
                sendMessage(sock, buffer);
                printf("C: Sending game ID to server.\n");
            } else if (strncmp(*line, "+ PLAYING", 9) == 0) { // Read game kind
                if (strncmp(*line + 10, profile->gamekindname, 7) != 0) break;
                printf("S: Game ID accepted.\nS: Playing Reversi.\n");
                readMessage(sock, buffer, BUFSIZE);
                char **tokens = splitBy(*line, "+ \n");
                if (NULL == tokens) break;
                printf("S: Game details:\ndate: %s\ntime: %s\n", tokens[2], tokens[3]);
                free(tokens);
                memset(buffer, 0, BUFSIZE);
                if (info->playerNum == 1) {
                    strncpy(buffer, "PLAYER 0\n", BUFSIZE);
                } else {
                    strncpy(buffer, "PLAYER 1\n", BUFSIZE);
                }
                printf("C: Player number: %i\n", info->playerNum);
                sendMessage(sock, buffer);
            } else if (strncmp(*line, "+ YOU", 5) == 0) { // Read player ID and name
                char **tokens = splitBy(*line, "+ \"\n");
                if (NULL == tokens || tokenLen(tokens) < 3) break;
                strncpy(playerID,   tokens[1], IDLENGTH - 1);
                strncpy(playerName, tokens[2], NAMELENGTH - 1);
                printf("S: You are playing as %s.\n", playerName);
                free(tokens);
            } else if (strncmp(*line, "+ TOTAL", 7) == 0) { // Read number of players
                char **tokens = splitBy(*line, "+ \n");
                if (NULL == tokens || tokenLen(tokens) < 2) break;
                info->totalPlayers = strtol(tokens[1], NULL, 10);
                free(tokens);
                if (info->totalPlayers <= 0) break;
                char* number = toString(info->totalPlayers);
                printf("S: %s players are playing the game.\n", number);

                // Put data into gameInfo struct
                initializePlayers(info);
                if (NULL == pushPlayer(info, playerID, playerName, 1)) break;
                printf("S: Player %s is ready.\n", info->players[0].name);
                info->totalPlayers--;
            } else if (info->totalPlayers > 0 && strncmp(*line, "+", 1) == 0) { // Read competitor information
                char **tokens = splitBy(*line, "+ \"\n");
                if (NULL == tokens || tokenLen(tokens) < 3) break;
                long ready = strtol(tokens[2], NULL, 10);
                if (ready != 0 && ready != 1) {
                    printf("C: Exiting.\n");
                    free(tokens);
                    break;
                }
                if (NULL == pushPlayer(info, tokens[0], tokens[1], ready)) {
                    printf("C: Exiting.\n");
                    free(tokens);
                    break;
                }
                free(tokens);
                info->totalPlayers--;
                printf("S: You are playing against %s.\n", info->players[1].name);
                if (ready == 1) {
                    printf("S: Player %s is ready.\n", info->players[1].name);
                } else {
                    printf("S: Player %s is not ready.\n", info->players[1].name);
                }
            } else if (counterMoveDuration == true && strncmp(*line, "+ MOVE", 6) == 0 && strncmp(*line, "+ MOVEOK", 8) != 0) { // Get max move duration
                char **tokens = splitBy(*line, "+ \n");
                if (NULL == tokens || tokenLen(tokens) < 2) break;
                info->moveDuration = strtol(tokens[1], NULL, 10);
                printf("S: Maximum move duration is %li usec.\n", info->moveDuration);
                counterMoveDuration = false;
                free(tokens);
                if (info->moveDuration <= 0) break;
            } else if (strncmp(*line, "+ FIELD", 7) == 0) { // Read game board
                char **tokens = splitBy(*line, "+, \n");
                if (NULL == tokens || tokenLen(tokens) < 3) break;
                info->columns = strtol(tokens[1], NULL, 10);
                info->rows    = strtol(tokens[2], NULL, 10);
                free(tokens);
                if (info->columns <= 0 || info->rows <= 0) break;
                if (counter >= 7) {
                    counter = 0;
                }
                counterStart = true;
            } else if (strncmp(*line, "+", 1) == 0 && counterStart == true && counter < 8 && strncmp(*line, "+ WAIT", 6) != 0) {
                char **tokens = splitBy(*line, "+12345678 \n");
                for(int j = 0; j < info->columns; j++) {
                    info->field[counter][j] = *tokens[j];
                }
                free(tokens);
                counter++;
            } else if (strncmp(*line, "+ ENDFIELD", 10) == 0) {
                if (!gameover) {
                    strncpy(buffer, "THINKING\n", BUFSIZE);
                    sendMessage(sock, buffer);
                    printf("C: Computing the next move...\nC: Here's the current board:\n");
                    counterStart = false;
                    info->think = true;
                    kill(info->parentPID, SIGUSR1);
                }
            } else if (strncmp(*line, "+ OKTHINK", 9) == 0) {
                printf("S: Waiting for %s's move.\n", info->players[0].name);
                pipeSelect   = FD_ISSET(fd[0], &rfds);
                if (pipeSelect) {
                    nread = read (fd[0], moveBuffer, BUFSIZE);
                    if (0 == nread || -1 == nread) {
                        printf("0 Bytes aus der Pipe gelesen! Fehler.\n");
                        error = true;
                    }
                    printf("C: %s's move: %s", info->players[0].name, moveBuffer); // for debugging
                    strncpy(buffer, moveBuffer, BUFSIZE); // send move to server
                    sendMessage(sock, buffer);
                } else {
                    printf("Error: pipe timeout.\n");
                    error = true;
                }
            } else if (strncmp(*line, "+ WAIT", 6) == 0) {
                printf("C: Waiting for %s's move.\n", info->players[1].name);
                strncpy(buffer, "OKWAIT\n", BUFSIZE);
                sendMessage(sock, buffer);
            } else if (strncmp(*line, "+ GAMEOVER", 10) == 0) {
                printf("S: Game over!\n");
                gameover = true;
            } else if (strncmp(*line, "+ PLAYER0WON", 12) == 0) {
                char **tokens = splitBy(*line, "+ \n");
                if (NULL == tokens || tokenLen(tokens) < 2) break;
                if (strncmp(tokens[1], "Yes", 3) == 0) {
                    info->player0Won = true;
                } else {
                    info->player0Won = false;
                }
                free(tokens);
            } else if (strncmp(*line, "+ PLAYER1WON", 12) == 0) {
                char **tokens = splitBy(*line, "+ \n");
                if (NULL == tokens || tokenLen(tokens) < 2) break;
                if (strncmp(tokens[1], "Yes", 3) == 0) {
                    info->player1Won = true;
                } else {
                    info->player1Won = false;
                }
                free(tokens);
            } else if (strncmp(*line, "+ QUIT", 6) == 0) {
                free(buffer);
                free(moveBuffer);
                free(lines);
                info->gameover = true; // tell thinker to print final board
                kill(info->parentPID, SIGUSR1);
                return 0;
            } else if (strncmp(*line, "-", 1) == 0) {
                printf("S: Error!\n");
                error = true;
            }
            error = false;
            line++;
        }
        if (error == true) break;
        free(lines);
    }
    free(buffer);
    free(moveBuffer);

    return -1;
}

// Connect to a host
int connectWith(connectionProfile *profile) {

    struct hostent *host = gethostbyname(profile->hostname);
    if (NULL == host) return -1;

    profile->sock = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == profile->sock) return -1;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port   = htons(profile->portnumber);
    memcpy(&address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    if (connect(profile->sock, (struct sockaddr*) &address, sizeof(address)) == 0){
        printf("C: Successfully connected to the gameserver at %s.\n", inet_ntoa(address.sin_addr));
    } else {
        perror("Connection error.");
        return -1;
    }
    return profile->sock;
}

// Send param message to the socket
static ssize_t sendMessage(int sock, const char *message) {
    // //debugging (with const char *prefix in the definition): printf("%s%s", prefix, message);
    ssize_t bytes_sent = send(sock, message, strnlen(message, BUFSIZE), 0);
    if (-1 == bytes_sent) {
        perror("send");
        return -1;
    }
    return bytes_sent;
}

// Receive a message from the socket and save it in param buffer
static ssize_t readMessage(int sock, char *buffer, const size_t bufsize) {
    ssize_t bytes_received = recv(sock, buffer, bufsize - 1, 0);
    if (-1 == bytes_received) {
        perror("recv");
        return -1;
    }
    if (bytes_received >= 0) {
        buffer[bytes_received] = '\0';
    }
    //debugging (with const char *prefix in the definition):
    // printf("%s%s", prefix, buffer);
    return bytes_received;
}

// Print totalPlayers as a string
static char* toString(int number) {
    switch(number) {
        case 0: return "Zero";
        case 1: return "One";
        case 2: return "Two";
        default: return "";
    }
}
