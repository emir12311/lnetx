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

void	print_progress(uint64_t bytes_done, uint64_t total_bytes)
{
	static struct timespec	last_time;
	static uint64_t			last_bytes;
	static int				initialized;
	struct timespec			now;
	double					elapsed;
	double					speed;
	double					progress;
	double					eta;
	int						filled;
	int						i;

	clock_gettime(CLOCK_MONOTONIC, &now);
	if (!initialized)
	{
		last_time = now;
		last_bytes = bytes_done;
		initialized = 1;
		return ;
	}
	elapsed = (double)(now.tv_sec - last_time.tv_sec);
	elapsed += (double)(now.tv_nsec - last_time.tv_nsec) / 1000000000.0;
	if (elapsed < 0.08)
		return ;
	speed = (double)(bytes_done - last_bytes) / elapsed;
	progress = (double)bytes_done / (double)total_bytes;
	if (speed > 0.0)
		eta = (double)(total_bytes - bytes_done) / speed;
	else
		eta = 0.0;
	filled = (int)(progress * 30.0);

	printf("\033[1A\033[2K\r");
	printf("\033[2K[");
	i = 0;
	while (i < 30)
	{
		if (i < filled)
			printf("█");
		else
			printf("░");
		i++;
	}
	printf("] %3d%% %.1f KiB/s\n", (int)(progress * 100.0),
		speed / 1024.0);
	printf("\033[2KETA: %.1fs", eta);
	fflush(stdout);

	last_time = now;
	last_bytes = bytes_done;
}

int	mkdir_p(const char* full_path)
{
	const char*	slash_ptr;
	int			i;
	int			d;
	int			n;
	char		prev_buf[PATH_MAX];
	char		aft_buf[PATH_MAX];
	char		path_buf[PATH_MAX];

	memset(&prev_buf, 0, sizeof prev_buf);
	memset(&aft_buf, 0, sizeof aft_buf);
	memset(&path_buf, 0, sizeof path_buf);
	slash_ptr = &full_path[0];
	if (slash_ptr[0] == '/')
	{
		if (mkdir("/", 0755) != 0)
		{
			if (errno != EEXIST)
			{
				fprintf(stderr, "Couldnt make directory. Check permissions");
				return (1);
			}
		}
		strcpy(prev_buf, "/");
	}
	slash_ptr++;
	i = 0;
	while (strchr(slash_ptr + i, '/') != NULL)
	{
		d = 0;
		for (; slash_ptr[i] != '/'; i++, d++)
			aft_buf[d] = slash_ptr[i];
		n = snprintf(path_buf, sizeof path_buf, "%s%s", prev_buf, aft_buf);
		if (n < 0 || (size_t)n >= sizeof path_buf)
		{
			fprintf(stderr, "Couldnt make directory, path too long.");
			return (1);
		}
		if (mkdir(path_buf, 0755) != 0)
		{
			if (errno != EEXIST)
			{
				if (n < 0 || (size_t)n >= sizeof path_buf)
					return (1);
			}
		}
		i++;
		path_buf[i] = '/';
		strcpy(prev_buf, path_buf);
		memset(&aft_buf, 0, sizeof aft_buf);
	}
	if (mkdir(full_path, 0755) != 0)
	{
		if (errno != EEXIST)
		{
			fprintf(stderr, "Couldnt make directory. Check permissions");
			return (1);
		}
	}
	return (0);
}
