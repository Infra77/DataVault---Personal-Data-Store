#include <stdio.h>
#include <fcntl.h>

#define SUCCESS 0
#define FAILURE 1
#define REC_NOT_FOUND 1
#define DB_OPEN 0
#define DB_CLOSE 1
#define DB_FULL 1

struct RecNdx{
    int old_key;
    int key;
    int loc;
    int is_deleted;
};
struct TableInfo{
    char tname[100];
    char ndxname[100];
    int rec_size;
    FILE* tfile;
    FILE* ndxfile;
    struct RecNdx ndxarray[100];
    int rec_count;
};

struct DBInfo{
    struct TableInfo table[2];
    int num_table;
    int db_status;
};

void init_table(struct TableInfo* t);
void Info_init();

int create_table(char* tablename);
int create_db(char* dbname1, char* dbname2);

int open_table(char* tname, int rec_size);
int open_db(char* tname1, int rec_size1, char* tname2, int rec_size2);

int store_table(int key, void* c, char* tname);

int get_table(int key, void* c, char* tname);
int update_table(int key, void* c, char* tname);
int delete_table(int key, char* tname);
int undelete_table(int key, char* tname);
int close_table(struct TableInfo* t);
int close_db();

