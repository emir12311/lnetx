#include <linux/limits.h>
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
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <inttypes.h>

#define PORT 56389

#define GO		'G'
#define DECLINE	'N'
#define OK		'K'
#define ERR		'E'

typedef struct
{
    unsigned int recv : 1;
    unsigned int send : 1;
    unsigned int alias : 1;
} cmds;

typedef struct
{
    char ip[16];
    char path[PATH_MAX];
} send_opts;

typedef struct
{
    char* name;
    char* ip;
    int mode;
} alias_opts;

int client(const char* ip, const int port, const char* msg);

int server(void);

int	alias(const char* name, char* inout_ip, const int mode);
