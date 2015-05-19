#include <fcntl.h>
#include <syslog.h>
#include <stdio.h>
#include "configuration.h"

extern int lockfd;
void exit(int status);
int 		checkPath		(char *checkingPath);
void 		my_handler		(int signum);
int initialize_configuration( void );
