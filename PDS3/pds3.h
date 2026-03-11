#include <stdio.h>
#include <fcntl.h>

#define SUCCESS 0
#define FAILURE 1
#define REC_NOT_FOUND 1
#define DB_OPEN 0
#define DB_CLOSE 1
#define DB_FULL 1
#define REL_OPEN 0
#define REL_CLOSE 1

struct RecNdx{
    int old_key;
    int key;
    int loc;
    int is_deleted;
};
struct TableInfo{
    char tname[100];
    char ndxname[100];
    FILE* tfile;
    FILE* ndxfile;
    int rec_size;
    int rec_count;
    struct RecNdx ndxarray[100];
};
struct RelPair{
    int pkey;
    int fkey;
    int is_deleted;
};
struct RelInfo{
    char ptname[50];
    char rtname[50];
    char reltname[100];
    FILE* reltfile;
    struct RelPair relations[100];
    int rel_count;
};
struct DBInfo{
    struct RelInfo rel_info;
    int rel_status;
    struct TableInfo table[2];
    int num_table;
    int db_status;
};

void init_table(struct TableInfo* t);
void init(int n);
void init_rel(struct RelInfo* r);

int create_table(char* tablename);
int create_db(char* dbname1, char* dbname2);
int create_relation(char* ptname, char* rtname, char* reltname);

int open_table(char* tname, int rec_size);
int open_db(char* tname1, int rec_size1, char* tname2, int rec_size2);
int open_relation(char* reltname);

int store_table(int key, void* c, char* tname);
int store_relation(int pkey, int fkey);

int get_table(int key, void* c, char* tname);
int get_relation(void* related_rec, int pkey);

int update_table(int key, void* c, char* tname);

int delete_table(int key, char* tname);
int delete_relation(int key, char* tname);

int undelete_table(int key, char* tname);
int undelete_relation(int key, char* tname);

int close_table(struct TableInfo* t);
int close_relation();
int close_db();

