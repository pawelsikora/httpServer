#include "configuration.h"
#include "logging.h"

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

