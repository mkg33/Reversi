#ifndef __CONFIG_H__
#define __CONFIG_H__

typedef struct connectionProfile {
    int sock;
    char *hostname;
    unsigned int portnumber;
    char *gamekindname;

} connectionProfile;

connectionProfile *readConfig(connectionProfile*, const char*);

#endif
