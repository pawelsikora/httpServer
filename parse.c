#include "common.h"
#include "parse.h"
#include "configuration.h"

int parse_config_file(){
	
	FILE * fp;
	const char s[SIZE_OF_ARRAY_ONE_EL] = "=";
	const char e[SIZE_OF_ARRAY_ONE_EL] = "\n";
	char *token;
	char *string_from_file;
	char tmp[MAX_SIZE_OF_CONF_FILE];
	int len = 0;
	int bIfSecond = 0;
	int start = 1;
	int *ptrFromHashTableLookup;
	gint HASH_ROOT;
	gint HASH_PORT;
	gint HASH_COM;
	

	GHashTable* hash = g_hash_table_new(g_str_hash, g_int_equal);
	HASH_ROOT = 1;
	HASH_PORT = 2;
	HASH_COM  = 3;
	g_hash_table_insert(hash, "ROOT", &HASH_ROOT);
	g_hash_table_insert(hash, "PORT", &HASH_PORT);
	g_hash_table_insert(hash, "CONS" , &HASH_COM );

	fp = fopen("http.conf", "r");
	while((tmp[len] = fgetc(fp)) != EOF)
		len++;
	
	string_from_file = (char*)calloc(len+3, sizeof(tmp[0]));
	memcpy(string_from_file, tmp, len);
	
	fclose(fp);
	
	do{
	  
		if((token = (bIfSecond) ? strtok(NULL, e) : ( start ? strtok(string_from_file, s) : strtok(NULL, s))) == NULL)
			break;	
		
		if(!bIfSecond)
			if((ptrFromHashTableLookup = g_hash_table_lookup(hash, token)) == NULL)
			{
				printf("ptrFromHashTableLookup is NULL\n");
				exit(1);
			}

		start=0;	
		  
		   if(((*ptrFromHashTableLookup == HASH_KEY_ROOT) && bIfSecond))
			{
				memcpy(configuration.root, token, strlen(token));

			}
			else if ((*ptrFromHashTableLookup == HASH_KEY_PORT) && bIfSecond)
			{
				memcpy(configuration.port.name, token, strlen(token));
			}
			else if ((*ptrFromHashTableLookup == HASH_KEY_COM) && bIfSecond)
			{
			configuration.setDaemon.str[0] = token[0];
			configuration.setDaemon.str[1] = 0;
			configuration.setDaemon.val = ( atoi(&configuration.setDaemon.str[0]) ? 0 : 1 );

			}
			
			bIfSecond = !bIfSecond;
   } while ( token != NULL ); 

   return(0);
}

int parse_http_header( char* buffer, struct reqInfo * reqinfo )
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
	
	return 0;
}

int parse_check_path(char *checkingPath)
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
