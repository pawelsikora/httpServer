
	#include <stdlib.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/time.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <sys/stat.h>
	#include <errno.h>
	#include <string.h>
	#include <sys/select.h>
	#include <sys/wait.h>
	#include <arpa/inet.h>
	#include <time.h>
	#include <sys/file.h>
	#include <signal.h>
	#include <getopt.h>
	#include <glib.h>
	#include <stdbool.h>
	#include <getopt.h>
	
	
int bCheckPath;

char	lmessage[300] = {0};
int 	setDaemon = 0;
int 	setChroot = 0;
int 	sysLogOpened = 0;

// socket
int	listenfd;

//Request info
char	buf[100] = {0};
enum 	reqMethod { GET, NOTIMPLEMENTED, SEND, CHAT};
int 	lockfd;
	
const struct option long_options[] = {
{ "help"	, 0, NULL, 'h' },
{ "rootdir"	, 1, NULL, 'r' },
{ "port"	, 1, NULL, 'p' },
{ "daemon"	, 1, NULL, 'd' },
{ "chroot"	, 1, NULL, 'c' },
{  NULL		, 0, NULL,  0  }
};

struct  reqInfo 
{
	 enum reqMethod method;
	 char *resource;
	 int status;
};
	
//time
time_t	currentTime;
char   	*stringWithTime;
