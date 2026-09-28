#include "../include/lnetx.h"
#include <stdio.h>

void fill_spinner(char* spinner[])
{
    spinner[0] = "⠋";
    spinner[1] = "⠙";
    spinner[2] = "⠹";
    spinner[3] = "⠸";
    spinner[4] = "⠼";
    spinner[5] = "⠴";
    spinner[6] = "⠦";
    spinner[7] = "⠧";
    spinner[8] = "⠇";
    spinner[9] = "⠏";
}

int server(void)
{
    int                 server_fd;
    int                 client_fd;
    int                 recv_rtrn;
    int                 tmp;
    int                 i;
    socklen_t           client_size;
    struct sockaddr_in  address;
    struct pollfd       poll_cursor;
    char*               spinner[10];
    char                buf[3000];
    
    address.sin_family = AF_INET;
    address.sin_port = htons(4000);
    address.sin_addr.s_addr = INADDR_ANY;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        fprintf(stderr, "Getting TCP socket failed.\n");
        return (1);
    }
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        fprintf(stderr, "Binding to TCP socket failed.\n");
        return (1);
    }
    if (listen(server_fd, 10) == -1)
    {
        fprintf(stderr, "Setting TCP socket to listen failed.\n");
        return (1);
    }
    poll_cursor.fd = server_fd;
    poll_cursor.events = POLLIN;
    fill_spinner(spinner);
    i = 0;
    printf("%s Waiting for connection...", spinner[i]);
    fflush(stdout);
    i++;
    while (1)
    {
        tmp = poll(&poll_cursor, 1, 120);
        if (tmp > 0)
        {
            if (poll_cursor.revents == POLLIN)
            {
                client_fd = accept(server_fd, (struct sockaddr *)&address, &client_size);
                break ;
            }
        }
        if (tmp == -1)
        {
            fprintf(stderr, "Polling client connection failed.\n");
            return (1);
        }
        printf("\r%s Waiting for connection...", spinner[i]);
        fflush(stdout);
        if (i == 9)
            i = 0;
        else
            i++;
    }
    if (client_fd == -1)
    {
        fprintf(stderr, "Accepting client failed.\n");
        return (1); 
    }
    printf("\r\033[2KGot connection ✓\n");
    recv_rtrn = recv(client_fd, buf, sizeof buf, 0);
    if (recv_rtrn == 0)
    {
        printf("Client closed its connection.\n");
        return (0); 
    }
    else if (recv_rtrn == -1)
    {
        fprintf(stderr, "Couldnt receive data.\n");
        return (1);
    }
    printf("Received message: %s\n", buf);
    close(client_fd);
    close(server_fd);
    return (0);
}