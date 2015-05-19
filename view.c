#include <stdio.h>

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
