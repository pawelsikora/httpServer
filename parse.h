#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#define HASH_KEY_ROOT 1
#define HASH_KEY_PORT 2
#define HASH_KEY_COM  3	

int	parse_check_path(char *checkingPath);
int 	parse_config_file(void);
