#include "configuration.h"
#include "parse.h"
#include "logging.h"

char	lmessage[300] = {0};
int 	sysLogOpened = 0;

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

void getCurrTime()
{
	currentTime = time(NULL);
	stringWithTime = ctime(&currentTime);
}

void Log (char* logMessage)
{
	FILE 	*logFile;
	char 	tmp[30] = {0};
		
	if(!fopen(configuration.logfile, "r"))
	{
		logFile = fopen(configuration.logfile, "w");
	}
	else
	{
		logFile = fopen(configuration.logfile, "a");
	}
	
	getCurrTime();
	sprintf(tmp, "Time: %s", stringWithTime);
	fputs(logMessage, logFile);
	fputs(tmp, logFile);
	fclose(logFile);	

}

