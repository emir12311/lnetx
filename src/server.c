#include "../include/lnetx.h"
#include "../include/utils.h"
#include "../lib/blake3/blake3.h"

static void	fill_spinner(char* spinner[])
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

static int	load_spin(struct pollfd* poll_stru, const char* message)
{
	char*	spinner[10];
	int		tmp;
	int		i;

	fill_spinner(spinner);
	i = 0;
	printf("%s %s", spinner[i], message);
	fflush(stdout);
	i++;
	while (1)
	{
		tmp = poll(poll_stru, 1, 120);
		if (tmp > 0)
			return (poll_stru->revents);
		if (tmp == -1)
			return (-1);
		printf("\r%s %s", spinner[i], message);
		fflush(stdout);
		if (i == 9)
			i = 0;
		else
			i++;
	}
}

static int	receive_x_bytes(const int fd, const size_t x, char* buf)
{
	size_t	offset;
	ssize_t	got;

	offset = 0;
	while (offset != x)
	{
		got = recv(fd, buf + offset, x - offset, 0);
		if (got == 0)
		{
			fprintf(stderr, "Client closed connection.\n");
			return (-2);
		}
		if (got == -1)
		{
			fprintf(stderr, "Couldnt receive file.\n");
			return (-1);
		}
		offset += (size_t)got;
	}
	return ((int)offset);
}

static int	write_x_bytes(const int fd, const size_t x, char* buf)
{
	size_t	offset;
	ssize_t	written_bytes;

	offset = 0;
	while (offset != x)
	{
		written_bytes = write(fd, buf + offset, x - offset);
		if (written_bytes == -1)
		{
			fprintf(stderr, "Couldnt write data.\n");
			return (-1);
		}
		offset += (size_t)written_bytes;
	}
	return ((int)offset);
}

static int	get_metadata(const int fd, char* out_hash, char* out_file_name, uint64_t* out_file_size)
{
	char		file_size_buf[21]; // size of the max numbers in 64 bit unsigned integer as chars + 1 for the null terminator
	int			x;

	x = receive_x_bytes(fd, 256, out_file_name);
	if (x == -1)
		return (-1);
	else if (x == -2)
		return (1);
	x = receive_x_bytes(fd, 21, file_size_buf);
	if (x == -1)
		return (-1);
	else if (x == -2)
		return (1);
	x = receive_x_bytes(fd, 65, out_hash);
	if (x == -1)
		return (-1);
	else if (x == -2)
		return (1);
	*out_file_size = strtoull(file_size_buf, NULL, 10);
	return (0);
}

static int	receive_transfer(const int fd, char* path, uint64_t file_size, const char* org_hash)
{
	int				file_fd;
	int				chk;
	int				chk_send;
	uint64_t		remaining;
	unsigned char	file_hash_raw[33];
	char			file_hash[65];
	char			buf[65536];

	file_fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
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
			chk = receive_x_bytes(fd, sizeof buf, buf);
		else if (remaining < sizeof buf)
			chk = receive_x_bytes(fd, remaining, buf);
		if (chk == -1 || chk == -2)
		{
			fprintf(stderr, "Receiving file failed.\n");
			return (1);
		}
		chk_send = write_x_bytes(file_fd, (size_t)chk, buf);
		if (chk_send == -1)
		{
			fprintf(stderr, "Writing file failed.\n");
			return (1);
		}
		else if (chk_send == 1)
			return (1);
		remaining -= (uint64_t)chk;
	}
	printf("✓ Got file\n");
	printf("... Integrity check:\n");
	hash_file(path, file_hash_raw);
	hash_to_pretty(file_hash_raw, file_hash);
	printf("Original: %s\nReceived: %s\n", org_hash, file_hash);
	if (strcmp(file_hash, org_hash) != 0)
	{
		printf("INTEGRITY FAILED.\n");
		return (1);
	}
	printf("✓ Integrity passed\n");
	close(file_fd);
	return (0);
}

static void	clean_stdin(void)
{
	int	c;
	while ((c = getchar()) != '\n' && c != EOF)
		;
}

int	server(void)
{
	int					server_fd;
	int					client_fd;
	int					load_value;
	socklen_t			client_size;
	struct sockaddr_in	address;
	struct pollfd		poll_cursor;
	char				file_name[NAME_MAX + 1];
	char				hash[65]; // normal 256 bit hash = 32 bytes = 64 bytes as chars + null terminator 
	char				file_size_buf[32];
	char				buf[32];
	char				downloads_folder[PATH_MAX];
	char				home_folder[PATH_MAX];
	uint64_t			file_size;
	
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
	load_value = load_spin(&poll_cursor, "Waiting for connection...");
	if (load_value == -1)
	{
		fprintf(stderr, "Polling connection failed.\n");
		return (1);
	}
	client_size = sizeof(address);
	client_fd = accept(server_fd, (struct sockaddr *)&address, &client_size);
	if (client_fd == -1)
	{
		fprintf(stderr, "Accepting client failed.\n");
		return (1); 
	}
	printf("\r\033[2K✓ Got connection\n");
	if (get_metadata(client_fd, hash, file_name, &file_size) != 0)
	{
		fprintf(stderr, "Couldnt receive metadata.\n");
		return (1);
	}
	bytes_to_pretty(file_size, file_size_buf);
	printf("Received metadata:\nFilename %s\nSize: %s\nBLAKE3: %s\n", file_name, file_size_buf, hash);
	printf("Continue? [Y/n] ");
	fflush(stdout);
	if (fgets(buf, sizeof buf, stdin) == NULL)
	{
		fprintf(stderr, "fgets returned NULL");
		return (1);
	}
	if (strchr(buf, '\n') == NULL)
		clean_stdin();
	if (buf[0] == 'n' || buf[0] == 'N')
		return (1);
	if (get_home_dir(home_folder))
	{
		fprintf(stderr, "Couldnt get home folder.\n");
		return (1);
	}
	snprintf(downloads_folder, sizeof downloads_folder, "%s/Downloads/%s", home_folder, file_name);
	if (receive_transfer(client_fd, downloads_folder, file_size, hash))
	{
		fprintf(stderr, "Couldnt receive file.\n");
		return (1);
	}
	close(client_fd);
	close(server_fd);
	return (0);
}