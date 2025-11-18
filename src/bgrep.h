#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

int bgrep(bool pattern_flag, bool context_flag, char *pattern, char **file_arr, int file_count);