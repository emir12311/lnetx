#include "../include/lnetx.h"

int	client(const char* ip, const int port, const char* msg)
{
	int					client_fd;
	int					tmp;
	struct sockaddr_in	address;
	char				buf[3000];

	address.sin_family = AF_INET;
	address.sin_port = htons((uint16_t)port);
	tmp = inet_pton(AF_INET, ip, &address.sin_addr.s_addr);
	if (tmp == 0)
	{
		fprintf(stderr, "Invalid IP\n");
		return (1);
	}
	if (tmp == -1)
	{
		fprintf(stderr, "Invalid IP range\n");
		return (1);
	}
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd == -1)
	{
		fprintf(stderr, "Getting TCP socket failed.\n");
		return (1);
	}
	if (connect(client_fd, (struct sockaddr *)&address, sizeof address) == -1)
	{
		fprintf(stderr, "Connection to server failed\n");
		return (1);
	}
	strcpy(buf, msg);
	send(client_fd, buf, sizeof buf, 0);
	close(client_fd);
	return (0);
}