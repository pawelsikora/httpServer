#define HASH_KEY_ROOT 1
#define HASH_KEY_PORT 2
#define HASH_KEY_COM  3	

//Hash table:


struct entry_s
{
	char *key;
	char *value;
	struct entry_s *next;
};

typedef struct entry_s entry_t;
