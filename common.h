#include "defs.h"

enum 	reqMethod { GET, NOTIMPLEMENTED };

struct  reqInfo 
{
	 enum reqMethod method;
	 char *resource;
	 int status;
} reqInfo;
