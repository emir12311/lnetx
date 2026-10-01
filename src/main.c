#include "../include/lnetx.h"
#include "../include/utils.h"

static void	print_usage(void)
{
	fprintf
	(stderr,
	"Usage:\n"
	"lnetx <COMMANDS> <COMMAND ARGS> [OPTIONS]\n"
	"Commands:\n"
	"receive, send\n"
	"Command Arguments:\n"
	"receive: N/A\n"
	"ex: lnetx receive"
	"send: ip, port, message\n"
	"ex: lnetx send 127.0.0.1 4000 'hello'\n"
	"Options:\n"
	"--help                                     Print this text.\n"
	);
}

static int	parse_args(const int argc, char* argv[], cmds* commands, send_opts* send_options)
{
	uint16_t	port_tmp;
	
	if (strcmp(argv[1], "receive") == 0)
	{
		if (argv[2] == NULL)
		{
			commands->recv = 1;
			return (0);
		}
		else
		{
			print_usage();
			return (1);
		}	
	}
	else if (strcmp(argv[1], "send") == 0)
	{
		port_tmp = (uint16_t)atoi(argv[3]);
		if (argc >= 5 && in_range(1, port_tmp, 65535))
		{
			commands->send = 1;
			send_options->ip = argv[2];
			send_options->port = port_tmp;
			send_options->path = argv[4];
			return (0);
		}
		else
		{
			print_usage();
			return (1);
		}
	}
	else
	{
		print_usage();
		return (1);
	}
	return (0);
}

int	main(int argc, char* argv[])
{
	cmds		commands;
	send_opts	send_options;
	struct stat	st;
	
	if (parse_args(argc, argv, &commands, &send_options))
		return (1);
	if (commands.recv)
	{
		if (server())
			return (1);
	}
	if (commands.send)
	{
		if (lstat(argv[4], &st) == -1)
		{
			fprintf(stderr, "Passed path is invalid.\n");
			return (1);
		}
		if (client(send_options.ip, send_options.port, send_options.path))
			return (1);
	}
	return (0);
}