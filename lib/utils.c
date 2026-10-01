#include "../include/lnetx.h"
#include "../include/utils.h"
#include "../lib/blake3/blake3.h"

int in_range(int a, int x, int b)
{
    if (a <= x && x <= b)
    {
        return (1);
    }
    return (0);
}

static void    fill_units(char** units)
{
    units[0] = "B";
    units[1] = "KiB";
    units[2] = "MiB";
    units[3] = "GiB";
    units[4] = "TiB";
}

void    bytes_to_pretty(uint64_t bytes, char* out_pretty_str)
{
    int     unit;
    char*   units[5];
    double  value;

    unit = 0;
    fill_units(units);
    value = (double)bytes;
    while (value >= 1024.0 && unit < 4)
    {
        value /= 1024.0;
        unit++;
    }
    if (unit == 0)
        snprintf(out_pretty_str, 32, "%.0f B", value);
    else
        snprintf(out_pretty_str, 32, "%.1f %s", value, units[unit]);
}

void	hash_to_pretty(const uint8_t *hash, char *out)
{
	static const char	hex[] = "0123456789abcdef";
	int					i;

	i = 0;
	while (i < 32)
	{
		out[i * 2] = hex[hash[i] >> 4];
		out[i * 2 + 1] = hex[hash[i] & 0x0F];
		i++;
	}
	out[64] = '\0';
}

int hash_file(char* full_path, uint8_t* out_hash)
{
	blake3_hasher	hasher;
	int				fd;
	ssize_t			read_bytes;
	unsigned char	bytes[65536];

	blake3_hasher_init(&hasher);
	fd = open(full_path, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "Couldnt open file.\n");
		return (1);
	}
	read_bytes = -2;
	while (read_bytes != 0)
	{
		read_bytes = read(fd, bytes, sizeof bytes);
		if (read_bytes == -1)
		{
			fprintf(stderr, "Couldnt read from file.\n");
			return (1);
		}
		blake3_hasher_update(&hasher, bytes, (size_t)read_bytes);
	}
	blake3_hasher_finalize(&hasher, out_hash, 32);
	close(fd);
	return (0);
}

int	get_home_dir(char* out_home_dir)
{
	char*	tmp;

	tmp = getenv("HOME");
	if (tmp == NULL)
	{
		fprintf(stderr, "Couldnt get home directory.");
		return (1);
	}
	strcpy(out_home_dir, tmp);
	return (0);
}