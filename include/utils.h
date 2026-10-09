#include <stdint.h>

int in_range(int a, int x, int b);

void bytes_to_pretty(uint64_t bytes, char* out_pretty_str);

void hash_to_pretty(const uint8_t *hash, char *out);

int hash_file(char* full_path, uint8_t* out_hash);

int	get_home_dir(char* out_home_dir);

void print_progress(uint64_t bytes_done, uint64_t total_bytes);

int mkdir_p(const char* full_path);
