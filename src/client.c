#include "../include/lnetx.h"
#include "../include/utils.h"
#include "../lib/blake3/blake3.h"

static int	send_x_bytes(const int fd, const size_t x, const char* buf)
{
	size_t	offset;
	ssize_t	sent;

	offset = 0;
	while (offset != x)
	{
		sent = send(fd, buf + offset, x - offset, 0);
		if (sent == 0)
		{
			fprintf(stderr, "Client closed connection.\n");
			return (1);
		}
		if (sent == -1)
		{
			fprintf(stderr, "Couldnt send data.\n");
			return (-1);
		}
		offset += (size_t)sent;
	}
	return (0);
}

static int	read_x_bytes(const int fd, const size_t x, char* out_buf)
{
	size_t	offset;
	ssize_t	read_bytes;

	offset = 0;
	while (offset != x)
	{
		read_bytes = read(fd, out_buf + offset, x - offset);
		if (read_bytes == 0)
			return ((int)offset);
		if (read_bytes == -1)
		{
			fprintf(stderr, "Couldnt read data.\n");
			return (-1);
		}
		offset += (size_t)read_bytes;
	}
	return ((int)offset);
}

static int	send_metadata(const int fd, const char* file_name, const char* file_size_buf, const char* hash)
{
	int			x;

	x = send_x_bytes(fd, 256, file_name);
	if (x == -1)
		return (-1);
	else if (x == 1)
		return (1);
	x = send_x_bytes(fd, 21, file_size_buf);
	if (x == -1)
		return (-1);
	else if (x == 1)
		return (1);
	x = send_x_bytes(fd, 65, hash);
	if (x == -1)
		return (-1);
	else if (x == 1)
		return (1);
	return (0);
}

static int	start_transfer(const int fd, char* path, uint64_t file_size)
{
	int			file_fd;
	int			chk;
	int			chk_send;
	uint64_t	remaining;
	char		buf[65536];

	file_fd = open(path, O_RDONLY);
	if (file_fd == -1)
	{
		fprintf(stderr, "Couldnt open file.\n");
		return (1);
	}
	chk = -2;
	chk_send = -2;
	remaining = file_size;
	while (remaining != 0)
	{
		if (remaining > sizeof buf)
			chk = read_x_bytes(file_fd, sizeof buf, buf);
		else if (remaining < sizeof buf)
			chk = read_x_bytes(file_fd, remaining, buf);
		if (chk == -1)
		{
			fprintf(stderr, "Reading file failed.\n");
			return (1);
		}
		printf("Read %d bytes, remaining before: %" PRIu64 "\n", chk, remaining);
		chk_send = send_x_bytes(fd, (size_t)chk, buf);
		if (chk_send == -1)
		{
			fprintf(stderr, "Sending file failed.\n");
			return (1);
		}
		else if (chk_send == 1)
			return (1);
		printf("Sent %d bytes\n", chk);
		remaining -= (uint64_t)chk;
	}
	close(file_fd);
	return (0);
}

int	client(const char* ip, const int port, const char* file)
{
	int					client_fd;
	int					tmp;
	struct sockaddr_in	address;
	struct stat			st;
	char				path_buf[PATH_MAX];
	char				file_name[NAME_MAX + 1];
	char				file_size_buf[21]; // explained in server.c
	char*				slash_ptr;
	char				hash_buf[65]; // converted to a string, the hash is 64 bytes, + null terminator
	uint8_t				hash[33]; // 32 bytes of the hash + null terminator

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
	if (realpath(file, path_buf) == NULL)
	{
		fprintf(stderr, "Couldnt get file path\n");
		return (1);
	}
	slash_ptr = strrchr(path_buf, '/');
	if (slash_ptr == NULL)
	{
		fprintf(stderr, "Path is invalid\n");
		return (1);	
	}
	strcpy(file_name, slash_ptr + 1);
	if (hash_file(path_buf, hash))
	{
		fprintf(stderr, "Couldnt hash file.\n");
		return (1);	
	}
	if (stat(path_buf, &st) == -1)
	{
		fprintf(stderr, "Couldnt call stat() on file\n");
		return (1);		
	}
	hash_to_pretty(hash, hash_buf);
	printf("%s\n", file_name);
	snprintf(file_size_buf, sizeof(file_size_buf), "%" PRIu64, st.st_size);
	if (send_metadata(client_fd, file_name, file_size_buf, hash_buf) != 0)
	{
		fprintf(stderr, "Sending metadata failed\n");
		return (1);
	}
	if (start_transfer(client_fd, path_buf, strtoull(file_size_buf, NULL, 10)))
	{
		fprintf(stderr, "Transfer failed.\n");
		return (1);
	}
	close(client_fd);
	return (0);
}