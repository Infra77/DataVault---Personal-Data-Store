#include <stdio.h>
struct Rec_ndx{
    int old_key;
    int key;
    int loc;
    int is_deleted;
};

struct Info{
    FILE* ndxfile;
    char ndxname[50];
    FILE* dbfile;
    char dbname[50];
    int status;
    struct Rec_ndx ndxArray[1000];
    int rec_count;
    int rec_size;
};

extern struct Info db_info;

#define SUCCESS 0
#define FAILURE 1
#define REC_NOT_FOUND 1
#define DB_OPEN 0
#define DB_CLOSE 1
#define DB_FULL 1

void Info_init();
int create_db(char* dbname);
int open_db(char *dbname, int rec_size);
int store_db(int key, void* c);
int get_db(int key, void* c);
int close_db();
int update_db(int key, void* c);
int delete_db(int key);
int undelete_db(int key);