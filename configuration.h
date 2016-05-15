#include "defs.h"
#include <getopt.h>

int initialize_configuration	( void );
int initializeServer		(char *port);

struct server_config 
{
	char root[MAX_SIZE_OF_PATH];
	char logfile[MAX_SIZE_OF_FILE_NAME];
	char lockfile[MAX_SIZE_OF_FILE_NAME];

	struct port_conf
	{
		int  	 val;
		char 	 name[MAX_SIZE_OF_PORT];
	} port;

	struct daemon_var {
		char	 str[SIZE_OF_ARRAY_ONE_EL];
		int	 val;
	} setDaemon;

} configuration;

