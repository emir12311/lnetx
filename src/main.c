#include "../include/lnetx.h"
#include "../include/utils.h"

static void	print_usage(void)
{
	fprintf
	(stderr,
		"Usage:\n"
		"  lnetx receive\n"
		"  lnetx send <ip|alias> <file>\n"
		"  lnetx alias list\n"
		"  lnetx alias create <name> <ip>\n"
		"  lnetx --help\n"
		"\n"
		"Commands:\n"
		"  receive                  Start the receive server.\n"
		"  send <ip|alias> <file>   Send a file to an IP or alias.\n"
		"  alias list               List configured aliases.\n"
		"  alias create <name> <ip> Create a new IP alias.\n"
		"\n"
		"Options:\n"
		"  --help                   Show this help message.\n"
	);
}

static int	parse_args(int argc, char* argv[], cmds* commands, send_opts* send_options, alias_opts* alias_options)
{
	struct in_addr	addr;
	char	out_ip[16];

	if (argc < 2)
	{
		print_usage();
		return (1);
	}
	if (strcmp(argv[1], "--help") == 0)
	{
		print_usage();
		return (0);
	}
	if (strcmp(argv[1], "receive") == 0)
	{
		if (argc != 2)
		{
			print_usage();
			return (1);
		}
		commands->recv = 1;
		return (0);
	}
	if (strcmp(argv[1], "send") == 0)
	{
		if (argc != 4)
		{
			print_usage();
			return (1);
		}
		commands->send = 1;
		if (snprintf(send_options->path, sizeof send_options->path, "%s",
				argv[3]) >= (int)sizeof send_options->path)
		{
			fprintf(stderr, "✘ Path is too long.\n");
			return (1);
		}
		if (inet_pton(AF_INET, argv[2], &addr) != 1)
		{
			if (alias(argv[2], out_ip, 2))
			{
				fprintf(stderr, "✘ Invalid IP / Alias\n");
				return (1);
			}
			strcpy(send_options->ip, out_ip);
			return (0);
		}
		strcpy(send_options->ip, argv[2]);
		return (0);
	}
	if (strcmp(argv[1], "alias") == 0)
	{
		if (argc < 3)
		{
			print_usage();
			return (1);
		}
		if (strcmp(argv[2], "list") == 0)
		{
			if (argc != 3)
			{
				print_usage();
				return (1);
			}
			commands->alias = 1;
			alias_options->mode = 1;
			return (0);
		}

		if (strcmp(argv[2], "create") == 0)
		{
			if (argc != 5)
			{
				print_usage();
				return (1);
			}
			commands->alias = 1;
			alias_options->mode = 0;
			alias_options->name = argv[3];
			alias_options->ip = argv[4];
			return (0);
		}
		print_usage();
		return (1);
	}
	print_usage();
	return (1);
}

int	main(int argc, char* argv[])
{
	cmds		commands = {0};
	send_opts	send_options = {0};
	alias_opts	alias_options = {0};
	struct stat	st;

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	if (parse_args(argc, argv, &commands, &send_options, &alias_options))
		return (1);
	if (commands.recv)
	{
		if (server())
			return (1);
	}
	if (commands.send)
	{
		if (lstat(argv[3], &st) == -1)
		{
			fprintf(stderr, "✘ Passed path is invalid.\n");
			return (1);
		}
		if (client(send_options.ip, PORT, send_options.path))
			return (1);
	}
	if (commands.alias)
	{
		if (alias(alias_options.name, alias_options.ip, alias_options.mode))
			return (1);
	}
	return (0);
}
