#include "libraries.h"
#include "common.h"

#define ERR_BAD_FILE_DESC 9

const struct option long_options[] = {
{ "help"	, 0, NULL, 'h' },
{ "rootdir"	, 1, NULL, 'r' },
{ "port"	, 1, NULL, 'p' },
{ "daemon"	, 1, NULL, 'd' },
{ "chroot"	, 1, NULL, 'c' },
{  NULL		, 0, NULL,  0  }
};
struct entry_s
{
	char *key;
	char *value;
	struct entry_s *next;
};

typedef struct entry_s entry_t;


int bCheckPath;

char	lmessage[300] = {0};
int 	setDaemon = 0;
int 	setChroot = 0;
int 	sysLogOpened = 0;

// socket
int	listenfd;

//Request info
char	buf[100] = {0};
int 	lockfd;



int 		SetChroot		(void);
int		initializeServer	(char *port);
int 		Request			(int n);
int 		getRequest		(int n, struct reqInfo * reqinfo);
int    		trim      		(char * buffer);
ssize_t 	ReadLine  		(int sockd, char *vptr, size_t maxlen);
ssize_t 	WriteLine 		(int sockd, char *vptr, size_t n);
int 		parse_http_header 	(char* buffer, struct reqInfo * reqinfo );
int 		returnResource  	(int n, int getRes, struct reqInfo * reqinfo );
void 		createDaemon		(void);
void 		isOneInstanceOfDaemon   (char *lockfile);
void 		initReqInfo		(struct reqInfo * reqinfo);
void 		freeReqInfo		(struct reqInfo * reqinfo);
int 		Error_Message		(int n, struct reqInfo * reqinfo);
void 		die			(const char*, int);
int 		getaddrinfo		(const char *node,
					 const char *service,
					 const struct addrinfo *hints,
					 struct addrinfo **res);

int 		print_info		(char * programName);

void 		my_handler		(int signum)
{
	if( (flock(lockfd, LOCK_UN)) < 0 )
	{
		die("Error unlocking LOCKFILE\n", LOG_USER);
	}
	else
	{
		die("LOCKFILE unlocked successfully\n", LOG_USER);
	}
	
	if (remove(configuration.lockfile) < 0 )
	{
		die("Error removing LOCKFILE\n", LOG_USER);
	}
	else
	{
		die("LOCKFILE removed!\n", LOG_USER);
	}
	
	
	closelog();
	
	exit(1);
}



int main(int argc, char *argv[])
{
	char	c;
	int	connected;
	pid_t 	pid;
	

	initialize_configuration();
	parse_config_file();
	
		while ((c = getopt_long(argc,argv, "hp:r:c:l:d:", long_options, NULL)) != -1)
			switch(c)
			{
				case 'h':
					system("clear");
					print_info(argv[0]);
					exit(0);
					break;
				case 'r':
					strcpy(configuration.root, optarg);
					break;
				case 'p':
					strcpy(configuration.port.name, optarg);
					break;
				case 'c':
					setChroot = atoi(optarg);
					break;
				case 'l':
					if(((strcpy(configuration.logfile, optarg)) == NULL))
						die("Error alocating memory: LOGFILE", LOG_USER);
					strcpy(configuration.logfile, optarg);
					break;
				case 'd':
					configuration.setDaemon.val = atoi(optarg);
					break;
				case '?':
					fprintf(stderr, "Wrong arguments!\n");
					exit(EXIT_FAILURE);
				default:
					exit(EXIT_FAILURE);
			}
	
	
	if(setChroot)	
	{
		setChroot = SetChroot();
	}


	initializeServer(configuration.port.name);
	
	
	if ((bCheckPath = parse_check_path(configuration.root)) == -1)
	{
		printf("\nPath was incorrect! Now path looks like this : |%s|\n", configuration.root);
	}
	else if (bCheckPath == 0)
	{
		printf("\nPath is correct!\n");
	}

	
	printf("Server is starting at port no. |%s|, with root directory as |%s|"
		"\nLogfile has been created: %s and chroot has%sbeen set\n"
		, configuration.port.name, configuration.root, configuration.logfile, setChroot?" ":"n't ");
	
	if(configuration.setDaemon.val)
	{	
		isOneInstanceOfDaemon(configuration.lockfile);
		createDaemon();
	}

	while(1){
		
		if( (connected = accept(listenfd, NULL, NULL)) < 0)
			perror("Error accept()");
		
		sprintf(lmessage, "Client connected at port %d. ", configuration.port.val);		
		Log(lmessage);
		
		
		if( (pid = fork()) == 0 ){
			
			if( close(listenfd)  < 0)
			{
				die("Error closing listening socket in child", LOG_USER);
			}

			Request(connected);
			
			if( close(connected) < 0 )
			{
				die("Error closing connection socket", LOG_USER);
			}
			else
			{
				Log("Client disconnected. ");
				exit(EXIT_SUCCESS);
			}
		}
		
		if( close(connected) < 0 )
		{
			die("Error closing listening socket in child", LOG_USER);
		}

		waitpid(-1, NULL, WNOHANG);
		

	}


	free(configuration.logfile);

	return 0;
}


int initialize_configuration( void )
{
	 
	configuration.port.val = DEFAULT_PORT;
	strcpy(configuration.port.name, DEFAULT_PORT_STR);
	strcpy(configuration.root, DEFAULT_PATH);
	strcpy(configuration.logfile,LOGFILE_NAME);
	strcpy(configuration.lockfile,LOCKFILE_NAME);

}

int initializeServer(char *port)
{
	int ret;
	struct addrinfo hints, *serverInfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if( (atoi(port) < 1024) && getuid() != 0)
	{
		printf("\nThese numbers of ports <1 - 1024> you can use only as root!\n"
			"Default number of port has been set: 9000\n");
		
		port = "9000";
		strcpy(configuration.port.name, port);
		configuration.port.val = 9000; 
	}
	else if (atoi(port) > 65535)
	{
		printf("debug port.val = %d\n\n", &configuration.port.val);
		printf("You can't use ports bigger than 65535\n"
			"Default number of port has been set: 9000\n");	
		
		port = "9000";
		strcpy(configuration.port.name, port);
		configuration.port.val = 9000; 
	}

	if((ret = (getaddrinfo(NULL, port, &hints, &serverInfo)) != 0)){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(ret));
		exit(1);
	}


	for(p = serverInfo; p != NULL; p = p->ai_next)
	{
		listenfd = socket(p->ai_family, p->ai_socktype, 0);
		
		if(listenfd == -1) continue;
		
		if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	
	if(p==NULL)
	{
		perror("socket() or bind() error");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(serverInfo);
	
	if (listen(listenfd, 1024) != 0)
	{
		perror("listen() error");
		exit(EXIT_FAILURE);
	}

	return listenfd;
}

int SetChroot()
{
	if(chdir("/") != 0)
	{
		fprintf(stderr, "chdir failed\n");
		exit(EXIT_FAILURE);

	}
	
	if( (chroot("/") != 0) )
	{
		fprintf(stderr, "chroot failed\n");
		exit(EXIT_FAILURE);
		return 0;
	}

	return 1;

}


void freeReqInfo(struct reqInfo * reqinfo)
{
	if( reqinfo->resource )
		free(reqinfo->resource);

}

void initReqInfo(struct reqInfo * reqinfo)
{
	reqinfo->resource = NULL;
	reqinfo->method   = NOTIMPLEMENTED;
	reqinfo->status   = 200;
}

int Request(int n)
{
	struct reqInfo	reqinfo;
	int		resource = 0;

	initReqInfo(&reqinfo);
	
	if(getRequest(n, &reqinfo) < 0 )
		return -1;
	
	if(reqinfo.status == 200)
	{	
		strcat(configuration.root, reqinfo.resource);
		resource = open(configuration.root, O_RDONLY);	
		sprintf(buf, "HTTP/1.0 200 OK.\n\n");
		WriteLine(n, buf, strlen(buf));
		
		if( returnResource(n, resource, &reqinfo) )
		{
			printf("Error returnResource(), error code: %d\n",errno);
			
			if (errno == ERR_BAD_FILE_DESC)	
				printf("Please check if the following path with requested file is valid: %s\n", configuration.root);
			
			exit(EXIT_FAILURE);
		}

	}
	else
	{
		Error_Message(n, &reqinfo);
	}
	
	if (resource > 0)
	{	
		if ( close(resource) < 0 )
		{
			die("Error closing resource.", LOG_USER);
		}
	}
	freeReqInfo(&reqinfo);

	return 0;
}
	


void createDaemon()
{
	FILE*	daemonfile;
	int 	i;
	pid_t	pid, sid;
	char	daemonpid[10] = {0};
	int	pidd	      = 0;	
	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);
	
	if (pid > 0)
		exit(EXIT_SUCCESS);
		
	umask(0);

	sid = setsid();
	
	signal(SIGCHLD, SIG_IGN);
	

	pid = fork();
	
	signal(SIGUSR1, &my_handler);
	
	if (pid == -1)
	{
		perror("Error fork while daemonising");

	} 
	else if (pid == 0)
	{
		exit(EXIT_SUCCESS);
	}
	
	if (sid < 0)
	{
		perror("Error creating Daemon");
		exit(EXIT_FAILURE);
	}
	
	sprintf(daemonpid, "%d", getpid());
	
	daemonfile = fopen("daemon.pid", "w");
	fputs(daemonpid, daemonfile);
	fclose(daemonfile);
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	openlog("my-server", 0, LOG_USER);
	
	sysLogOpened = 1;
}

void isOneInstanceOfDaemon(char *lockfile)
{
	static char	pathToLockFile[100] = {0};
	
	strcpy(pathToLockFile, configuration.root);
	strcat(pathToLockFile, configuration.lockfile);
	strcpy(configuration.lockfile, pathToLockFile);
	
	if( (lockfd = open(pathToLockFile, O_RDWR|O_CREAT|O_EXCL) < 0) )
	{
		perror("Error open lockfile! ");
	}
	
	if (errno == EEXIST)
	{	
		die("\nDaemon for this document root already exists\n", LOG_USER);
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("This is the first instance of Daemon\n");
		close(lockfd);
	}

}

int Error_Message(int n, struct reqInfo * reqinfo)
{
	char 	buffer[100];

	sprintf(buffer, "<HTML>\n<HEAD>\n<TITLE>Server Error %d</TITLE>\n"
			"</HEAD>\n\n", reqinfo->status);
	
	WriteLine(n, buffer, strlen(buffer));

	sprintf(buffer, "<BODY>\n<H1>Server Error %d</H1>\n", reqinfo->status);
	
	WriteLine(n, buffer, strlen(buffer));

	sprintf(buffer, "<P>The request could not be completed.</P>\n"
			"</BODY>\n</HTML>\n");
	
	WriteLine(n, buffer, strlen(buffer));

    return 0;
}

int getRequest(int n, struct reqInfo * reqinfo)
{
	char	buff[MAX_REQ_LINE] = {0};
	int	rval;
	fd_set	fds;
	struct  timeval tv;

	//Set timeout for input to be ready
	tv.tv_sec  = 5;
	tv.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET (n, &fds);

	rval = select(n+1, &fds, NULL, NULL, &tv);

	if ( rval < 0 )
	{
		die("Error select()", LOG_USER);
	}
	else if ( (rval = 0) )
	{
		return -1;
	}
	else
	{
		ReadLine(n, buff, MAX_REQ_LINE - 1);
		trim(buff);
		
		if( buff[0] == '\0' )
			return 0;
		if( parse_http_header(buff, reqinfo) )
			return 0;
	}
	
	return 0;

}

int trim(char* buffer)
{
	int	 n = strlen(buffer) - 1;

	while( !isalnum(buffer[n]) && n >= 0 )
		buffer[n--] = '\0';

	return 0;
}


int returnResource( int n, int getRes, struct reqInfo * reqinfo )
{
	char 	c;
	int  	i;
	
	while ((i = read(getRes, &c, 1)))
	{
		if ( i < 0 )
		{
			die("Error reading from file (resource)", LOG_USER);
			return 1;
		}
	
		if (write(n, &c, 1) < 1 )
		{
			die("Error sending file (resource)", LOG_USER);
		}
	}

	return 0;

}

ssize_t WriteLine( int sockd, char *vptr, size_t n)
{
	ssize_t		nleft;
	ssize_t		nwritten;
	char		*buffer;

	buffer = vptr;
	nleft = n;

	while ( nleft > 0 ) {
		if ( (nwritten = write(sockd, buffer, nleft)) <= 0 )
		{
			if ( errno == EINTR )
			{
				nwritten = 0;
			}
			else
			{
				die("Error in writeLine()", LOG_USER);
			}
		}
		nleft -= nwritten;
		buffer += nwritten;
	}

	return n;

}

ssize_t ReadLine ( int sockd, char *vptr, size_t maxlen)
{
	ssize_t		n, rc;
	char		c, *buffer;

	buffer = vptr;

	for ( n = 1; n < maxlen; n++) {
		
		if( (rc = read(sockd, &c, 1)) == 1 ) 
		{
			*buffer++ = c;
			
			if ( c == '\n' )
				break;
		
		}
		else if ( rc == 0 ) 
		{
			
			if ( n == 1 )
				return 0;
			else 
				break;
		}
		else
		{
			if ( errno == EINTR )
				continue;
			
			die("Error in readLine()", LOG_USER);
		}
	}

	*buffer = 0;
	return n;
}



void die(const char *message, int logPriority)
{
	if (sysLogOpened){
		syslog(logPriority, message);
	}
	else if(errno && !sysLogOpened){	
		perror(message);
	}
	else
	{
		printf("ERROR: %s\n", message);
	}
		
}

int print_info(char * programName)
{
	printf(
		"--------------------------------------------Simple Http Server------------------------------------------------\n\n"\
		"\n\nThis is a simple http server, which supports only GET command. Every other command\n"\
		"will be consider as UNSUPPORTED and will produce 501 error, which was objective as\n"\
		"a foundation of this program. Features:\n\n"\
		"- Running as a daemon\n- You can set chroot\n- Logs about connection are putting to logfile\n- Default settings loads from 'httpconf' file\n\n"\

		"\nType name of program: \"%s\" to command line with these possible parameters:\n\n"\
		"--rootdir	or	-r	Directory, which will be using as a server directory.\n"\
		"--port		or	-p	Port on which server will be working.\n"\
		"--daemon	or	-d	If you want to have your program working as a daemon, give 1 after this parameter.\n"\
		"				Otherwise 0. ex. ( -d 0 to run your program from terminal)\n"\
		"--chroot	or	-c	1 if you want chroot, otherwise 0.\n"\
		"--logfile	or	-l	After this parameter you should write name of the logfile you will create.\n\n", programName);
}

