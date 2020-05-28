# Compiler flags
CC = clang
CFLAGS = -Wall -Wextra -Werror

# Environment variables
ENVGAMEID = ${GAME_ID}
ENVPLAYER = ${PLAYER}

# Executable name
EXEC = sysprak-client

all: $(EXEC)

$(EXEC): main.o performConnection.o gameInfo.o utility.o config.o think.o
	$(CC) $(CFLAGS) -o $(EXEC) main.o performConnection.o gameInfo.o utility.o config.o think.o

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

performConnection.o: performConnection.c
	$(CC) $(CFLAGS) -c -o performConnection.o performConnection.c

gameInfo.o: gameInfo.c
	$(CC) $(CFLAGS) -c -o gameInfo.o gameInfo.c

utility.o: utility.c
	$(CC) $(CFLAGS) -c -o utility.o utility.c

config.o: config.c
	$(CC) $(CFLAGS) -c -o config.o config.c

think.o: think.c
	$(CC) $(CFLAGS) -c -o think.o think.c

clean:
	rm *.o $(EXEC)

play: $(EXEC)
	./$(EXEC) -g $(ENVGAMEID) -p $(PLAYER)

# --tracechildren=yes hinzufuegen wenn fork implementiert
valgrind:
	valgrind --leak-check=full ./$(EXEC)
