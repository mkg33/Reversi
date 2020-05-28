#ifndef __PERFORM_CONNECTION_H__
#define __PERFORM_CONNECTION_H__

#include "config.h"
#include "gameInfo.h"

int performConnection(connectionProfile*);
int connectWith(connectionProfile*);

int fd[2];

#endif /* __PERFORM_CONNECTION_H__ */
