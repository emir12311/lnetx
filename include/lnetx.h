#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

typedef struct
{
    unsigned int recv : 1;
    unsigned int send : 1;
} cmds;

typedef struct
{
    uint16_t port;
    char* ip;
    char* message;
} send_opts;

int client(const char* ip, const int port, const char* msg);

int server(void);