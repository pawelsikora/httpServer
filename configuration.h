
	#define MAX_REQ_LINE 1024
	#define DEFAULT_PORT 8080
	#define DEFAULT_PORT_STR "8080"
	#define DEFAULT_PATH "/home/sikor/"
	#define LOGFILE_NAME "http.log"
	#define LOCKFILE_NAME "daemon.lock"
	#define MAX_SIZE_OF_PORT 7 
	#define MAX_SIZE_OF_PATH 500
	#define MAX_SIZE_OF_FILE_NAME 40
	#define SIZE_OF_ARRAY_ONE_EL 2
	#define MAX_SIZE_OF_CONF_FILE 200

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