#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <poll.h>
#include <inttypes.h>

typedef struct
{
    unsigned int recv : 1;
    unsigned int send : 1;
} cmds;

typedef struct
{
    uint16_t port;
    char* ip;
    char* path;
} send_opts;

int client(const char* ip, const int port, const char* msg);

int server(void);