#include "../include/lnetx.h"
#include "../include/utils.h"
#include <string.h>

static int resolve_alias(const char *name, char *out_ip, FILE *alias_file)
{
	char	line[82];
	char*	equals;

	rewind(alias_file);
	while (fgets(line, sizeof(line), alias_file) != NULL)
	{
		equals = strchr(line, '=');
		if (equals == NULL)
			continue;
		if ((size_t)(equals - line) != strlen(name))
			continue;
		if (strncmp(line, name, (size_t)(equals - line)) != 0)
			continue;
		strncpy(out_ip, equals + 1, INET_ADDRSTRLEN);
		out_ip[strcspn(out_ip, "\n")] = '\0';
		return (0);
	}
	return (1);
}

static int	is_valid_ip(const char* ip)
{
	struct in_addr	addr;

	if (inet_pton(AF_INET, ip, &addr) != 1)
		return (0);
	return (1);
}

static int	is_valid_alias_line(const char *line)
{
	const char*		equals;
	char			ip[32];
	char*			new_line;
	size_t			alias_len;

	equals = strchr(line, '=');
	if (equals == NULL)
		return (0);
	strcpy(ip, equals + 1);
	new_line = strchr(ip, '\n');
	if (new_line != NULL)
		*new_line = '\0';
	if (strchr(ip, '=') != NULL)
		return (0);
	alias_len = (size_t)(equals - line);
	if (alias_len == 0 || alias_len > 64)
		return (0);
	if (memchr(line, '.', alias_len) != NULL)
		return (0);
	if (!is_valid_ip(ip))
		return (0);
	return (1);
}

static int	does_alias_exist(const char *name, FILE *alias_file)
{
	char	line[82];
	char*	equals;
	size_t	alias_len;

	rewind(alias_file);
	while (fgets(line, sizeof(line), alias_file) != NULL)
	{
		equals = strchr(line, '=');
		if (!is_valid_alias_line(line))
			continue;
		alias_len = (size_t)(equals - line);
		if (strlen(name) == alias_len && strncmp(line, name, alias_len) == 0)
			return (1);
	}
	return (0);
}

static int	list_aliases(FILE* alias_file)
{
	char	line[82];

	rewind(alias_file);
	while (fgets(line, sizeof(line), alias_file) != NULL)
	{
		if (!is_valid_alias_line(line))
			continue;
		printf("%s", line);
	}
	return (0);
}

int	alias(const char* name, char* inout_ip, const int mode) // 0 = create new, 1 = list, 2 = resolve
{
	char*	xdg_folder;
	char	home_folder[PATH_MAX];
	char	config_folder[PATH_MAX];
	char	file_path[PATH_MAX];
	FILE*	file_ptr;

	if ((mode == 0 || mode == 2) && strlen(name) > 64)
	{
		fprintf(stderr, "✘ Alias name cant be more than 64 characters.\n");
		return (1);
	}
	if ((mode == 0 || mode == 2) && strchr(name, '.') != NULL)
	{
		fprintf(stderr, "✘ Alias name cant contain a '.'\n");
		return (1);
	}
	xdg_folder = getenv("XDG_CONFIG_HOME");
	if (xdg_folder == NULL)
	{
		if (get_home_dir(home_folder))
		{
			fprintf(stderr, "✘ Couldnt get home directory.\n");
			return (1);
		}
		snprintf(config_folder, sizeof config_folder, "%s/.config/lnetx", home_folder);
	}
	else
		snprintf(config_folder, sizeof config_folder, "%s/lnetx", xdg_folder);
	if (mkdir_p(config_folder))
	{
		fprintf(stderr, "✘ Couldnt make config directory.\n");
		return (1);
	}
	snprintf(file_path, sizeof file_path, "%s/aliases.conf", config_folder);
	if (mode == 0)
	{
		file_ptr = fopen(file_path, "a+");
		if (file_ptr == NULL)
		{
			fprintf(stderr, "✘ Couldnt open alias file\n");
			return (1);
		}
		rewind(file_ptr);
		if (does_alias_exist(name, file_ptr))
		{
			fprintf(stderr, "✘ An alias named '%s' already exists\n", name);
			fclose(file_ptr);
			return (1);
		}
		if (!is_valid_ip(inout_ip))
		{
			fprintf(stderr, "✘ Invalid IP\n");
			fclose(file_ptr);
			return (1);
		}
		if (fprintf(file_ptr, "%s=%s\n", name, inout_ip) < 0)
		{
		    fprintf(stderr, "✘ Couldnt write to alias file\n");
			fclose(file_ptr);
		    return (1);
		}
		fclose(file_ptr);
		return (0);
	}
	else if (mode == 1)
	{
		file_ptr = fopen(file_path, "r");
		if (file_ptr == NULL)
		{
			if (errno == ENOENT)
			{
				fprintf(stderr, "✘ No aliases exist yet.\n");
				return (1);
			}
			fprintf(stderr, "✘ Couldnt open alias file\n");
			return (1);
		}
		rewind(file_ptr);
		list_aliases(file_ptr);
		fclose(file_ptr);
		return (0);
	}
	else if (mode == 2)
	{
		file_ptr = fopen(file_path, "r");
		if (file_ptr == NULL)
		{
			if (errno == ENOENT)
			{
				fprintf(stderr, "✘ No aliases exist yet.\n");
				return (1);
			}
			fprintf(stderr, "✘ Couldnt open alias file\n");
			return (1);
		}
		rewind(file_ptr);
		if (!does_alias_exist(name, file_ptr))
		{
			fprintf(stderr, "✘ Alias %s doesnt exist\n", name);
			fclose(file_ptr);
			return (1);
		}
		if (resolve_alias(name, inout_ip, file_ptr))
		{
			fprintf(stderr, "✘ Resolving alias %s failed.\n", name);
			fclose(file_ptr);
			return (1);
		}
		fclose(file_ptr);
		return (0);
	}
	else
	{
		fprintf(stderr, "✘ Unkown error\n");
		return (1);
	}
}
