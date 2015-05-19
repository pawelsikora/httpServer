#include "controller.h"

extern int setChroot;

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
		return 0;
	
	}
	
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
		
	int initialize_configuration( void )
	{
		 
		configuration.port.val = DEFAULT_PORT;
		strcpy(configuration.port.name, DEFAULT_PORT_STR);
		strcpy(configuration.root, DEFAULT_PATH);
		strcpy(configuration.logfile,LOGFILE_NAME);
		strcpy(configuration.lockfile,LOCKFILE_NAME);

	}
