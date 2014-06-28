	#include <stdlib.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/time.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <syslog.h>
	#include <string.h>
	#include <sys/select.h>
	#include <sys/wait.h>
	#include <arpa/inet.h>
	#include <time.h>
	#include <sys/file.h>
	#include <signal.h>
	#define MAX_REQ_LINE 1024
	
	int check;
	//getconf
	char	conf[5];
	int	iport;
	char 	*LOGFILE = "http.log";
	char 	*LOCKFILE = "daemon.lock";
	char	ROOT[500];
	char	PORT[7] = "8080";
	char	lmessage[300] = {0};
	int 	setDaemon = 0;
	int 	setChroot = 0;
	int 	sysLogOpened = 0;

	// socket
	int	listenfd;

	//Request info
	char	buf[100] = {0};
	enum 	reqMethod { GET, NOTIMPLEMENTED };
	int 	lockfd;


	struct  reqInfo 
	{
		 enum reqMethod method;
		 char *resource;
		 int status;
	};
	
	//time
	time_t	currentTime;
	char   	*stringWithTime;

	
	int 		SetChroot		(void);
	int		initializeServer	(char *port);
	int 		Request			(int n);
	int 		getRequest		(int n, struct reqInfo * reqinfo);
	void	 	configFile		(void);
	void 		Log			(char* message);
	int    		trim      		(char * buffer);
	ssize_t 	ReadLine  		(int sockd, char *vptr, size_t maxlen);
	ssize_t 	WriteLine 		(int sockd, char *vptr, size_t n);
	int 		parseHTTPheader 	(char* buffer, struct reqInfo * reqinfo );
	int 		returnResource  	(int n, int getRes, struct reqInfo * reqinfo );
	void 		createDaemon		(void);
	void 		isOneInstanceOfDaemon   (char *lockfile);
	void 		initReqInfo		(struct reqInfo * reqinfo);
	void 		freeReqInfo		(struct reqInfo * reqinfo);
	int 		Error_Message		(int n, struct reqInfo * reqinfo);
	void inline 	getCurrTime		(void);
	void 		die			(const char*, int);
	int 		getaddrinfo		(const char *node,
						 const char *service,
						 const struct addrinfo *hints,
						 struct addrinfo **res);
	int 		checkPath		(char *checkingPath);
	
	void 		my_handler(int signum)
	{
		if( (flock(lockfd, LOCK_UN)) < 0 )
		{
			die("Error unlocking LOCKFILE\n", LOG_USER);
		}
		else
		{
			die("LOCKFILE unlocked successfully\n", LOG_USER);
		}
		
		if (remove(LOCKFILE) < 0 )
		{
			die("Error removing LOCKFILE\n", LOG_USER);
		}
		else
		{
			die("LOCKFILE removed!\n", LOG_USER);
		}
		
		free(LOGFILE);
		closelog();
		
		exit(1);
	}
	


	int main(int argc, char *argv[])
	{
		char	c;
		int	connected;
		pid_t 	pid;	

		configFile();
		
			while ((c = getopt(argc,argv, "p:r:c:l:d:")) != -1)
				switch(c)
				{
					case 'r':
						strcpy(ROOT, optarg);
						break;
					case 'p':
						strcpy(PORT, optarg);
						break;
					case 'c':
						setChroot = atoi(optarg);
						break;
					case 'l':
						if((LOGFILE = malloc(strlen(optarg))) == NULL)
							die("Error alocating memory: LOGFILE", LOG_USER);;
						strcpy(LOGFILE, optarg);
						break;
					case 'd':
						setDaemon = atoi(optarg);
						break;
					case '?':
						fprintf(stderr, "Wrong arguments!\n");
						exit(EXIT_FAILURE);
					default:
						exit(EXIT_FAILURE);
				}
		
		iport = atoi(PORT);
		
		if(setChroot)	
		{
			setChroot = SetChroot();
		}

		
		
		initializeServer(PORT);
		
		
		if ((check = checkPath(ROOT)) == -1)
		{
			printf("\nPath was incorrect! Now path looks like this : |%s|\n", ROOT);
		}
		else if (check == 0)
		{
			printf("\nPath is correct!\n");
		}

		
		printf("Server is starting at port no. |%s|, with root directory as |%s|"
			"\nLogfile has been created: %s and chroot has%sbeen set\n"
			, PORT, ROOT, LOGFILE, setChroot?" ":"n't ");
		
		if(setDaemon)
		{	
			isOneInstanceOfDaemon(LOCKFILE);
			createDaemon();
		}

		while(1){
			
			if( (connected = accept(listenfd, NULL, NULL)) < 0)
				perror("Error accept()");
			
			sprintf(lmessage, "Client connected at port %d. ", iport);		
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


		free(LOGFILE);
		return EXIT_FAILURE;
	}

	int initializeServer(char *port)
	{
		int ret;
		struct addrinfo hints, *serverInfo, *p;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		if( (iport < 1024) && getuid() != 0)
		{
			printf("\nThese numbers of ports <1 - 1024> you can use only as root!\n"
				"Default number of port has been set: 9000\n");
			
			port = "9000";
			strcpy(PORT, port);
			iport = 9000; 
		}
		else if (iport > 65535)
		{
			printf("You can't use ports bigger than 65535\n"
				"Default number of port has been set: 9000\n");	
			
			port = "9000";
			strcpy(PORT, port);
			iport = 9000; 
		}

		if(ret = getaddrinfo(NULL, port, &hints, &serverInfo) != 0){
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
	
	int checkPath(char *checkingPath)
	{
		while(checkingPath)
		{	
			if(*checkingPath++ == '\0')
			{
				checkingPath -= 2;
				if(*checkingPath == '/')
				{
					return 0;
				}
				else
				{
					*(++checkingPath) = '/';
					*(++checkingPath) = '\0';
					return -1;
				}
			}
			
		
		}
	}

	void configFile(){
		
		FILE 	*confFile;
		int 	i;
		
		confFile = fopen("http.conf", "r");

		while (fgets(conf, 5, confFile))
		{
			if(strcmp(conf, "ROOT") == 0)
			{
				while( (ROOT[i++] = fgetc(confFile)) != '\n');
					ROOT[--i] = '\0';
			}
			
			i = 0;

			if(strcmp(conf, "PORT") == 0)
			{
				while( isalnum(PORT[i++] = fgetc(confFile)));
				PORT[--i] = '\0';
				iport = atoi(PORT);
								     
			}
			
			if(strcmp(conf, "CONS") == 0)
			{
				setDaemon = ( fgetc(confFile) == '1' ? 0 : 1 );
			}
		}
		fclose(confFile);

	}

	void Log (char* logMessage)
	{
		FILE 	*logFile;
		char 	tmp[30] = {0};
			
		if(!fopen(LOGFILE, "r"))
		{
			logFile = fopen(LOGFILE, "w");
		}
		else
		{
			logFile = fopen(LOGFILE, "a");
		}
		
		getCurrTime();
		sprintf(tmp, "Time: %s", stringWithTime);
		fputs(logMessage, logFile);
		fputs(tmp, logFile);
		fclose(logFile);	

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

			strcat(ROOT, reqinfo.resource);
			resource = open(ROOT, O_RDONLY);	
			sprintf(buf, "HTTP/1.0 200 OK.\n\n");
			WriteLine(n, buf, strlen(buf));
			
			if( returnResource(n, resource, &reqinfo) )
			{
				die("Error Return_Resource()", LOG_USER);
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
		FILE *daemonfile;
		int 	i;
		pid_t	pid, sid;
		char daemonpid[10] = {0};
		pid = fork();
		int pidd = 0;	
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
		
		strcpy(pathToLockFile, ROOT);
		strcat(pathToLockFile, LOCKFILE);
		LOCKFILE = strdup(pathToLockFile);
		
		if( lockfd = open(LOCKFILE, O_RDWR|O_CREAT|O_EXCL) < 0 )
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
		else if ( rval = 0 )
		{
			return -1;
		}
		else
		{
			ReadLine(n, buff, MAX_REQ_LINE - 1);
			trim(buff);
			
			if( buff[0] == '\0' )
				return 0;
			if( parseHTTPheader(buff, reqinfo) )
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

	int parseHTTPheader( char* buffer, struct reqInfo * reqinfo )
	{
		static int 	first_header = 1;
		char		*endptr;
		int 		len;

		if (first_header)
		{
			if( !strncmp(buffer, "GET ", 4) )
			{
				reqinfo->method = GET;
				buffer += 5;
			}
			else
			{
				reqinfo->method = NOTIMPLEMENTED;
				reqinfo->status = 501;
				return -1;
			}
			
			//Beggining of the resource
			while ( *buffer && isspace(*buffer) )
				buffer++;
			
			//Calculate string
			endptr = strchr(buffer, ' ');
			if ( endptr == NULL)
				len = strlen(buffer);
			else
				len = endptr - buffer;

			if ( len == 0 ) {
				reqinfo->status = 400;
				return -1;
			}

			//Store in request information structure
			
			reqinfo->resource = calloc(len+1, sizeof(char));
			strncpy(reqinfo->resource, buffer, len);

			first_header = 0;
			return 0;
		}

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


	void inline getCurrTime()
	{
		currentTime = time(NULL);
		stringWithTime = ctime(&currentTime);
	}
	
	void die(const char *message, int logPriority)
	{
		if (sysLogOpened)
		{
			syslog(logPriority, message);
		}
		else if(errno && !sysLogOpened)
		{	
			perror(message);
		}
		else
		{
			printf("ERROR: %s\n", message);
		}
			
	}
	
