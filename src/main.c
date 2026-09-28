#include "../include/lnetx.h"

int main(int argc, char* argv[])
{
    if (strcmp(argv[1], "recieve") == 0)
    {
        if (server())
            return (1);
        return (0);
    }
    if (strcmp(argv[1], "send") == 0)
    {
        if (client(argv[2], atoi(argv[3]), argv[4]))
            return (1);
        return (0);
    }
}