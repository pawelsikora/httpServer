#include <sys/time.h>
#include <stdio.h>
#include <time.h>
#include <syslog.h>

void		getCurrTime		(void);
void 		Log			(char* message);
void		die			(const char*, int);

time_t	currentTime;
char   	*stringWithTime;

