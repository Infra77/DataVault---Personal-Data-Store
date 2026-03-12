#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS       0
#define FAILURE       1
#define REC_NOT_FOUND 1
#define DB_OPEN       0
#define DB_CLOSE      1
#define DB_FULL       1
#define REL_OPEN      0
#define REL_CLOSE     1

struct RecNdx {
    int old_key;
    int key;
    int loc;
    int is_deleted;
};

struct TableInfo {
    char tname[100];
    char ndxname[100];
    FILE* tfile;
    FILE* ndxfile;
    int rec_size;
    int rec_count;
    struct RecNdx ndxarray[1000];
};

struct RelPair {
    int pkey;
    int fkey;
    int is_deleted;
};

struct RelInfo {
    char ptname[100];
    char rtname[100];
    char reltname[100];
    FILE* reltfile;
    struct RelPair relations[1000];
    int rel_count;
    int rel_status;
};

struct DBInfo {
    char dbname[100];           /* stored so close_db can auto-save schema */
    struct RelInfo* rel_info;   /* fixed array of RelInfos, size set at init */
    int num_relations;
    int rel_capacity;
    struct TableInfo* table;    /* fixed array of TableInfos, size set at init */
    int table_capacity;
    int num_table;
    int db_status;
};

void init_table(struct TableInfo* t);
void init_rel(struct RelInfo* r);
void init(int num_tables, int num_relations);

int save_schema(char* dbname);
int load_schema(char* dbname);

int create_table(char* tablename);
int create_db(int num_tables, char** tnames);
int create_relation(char* ptname, char* rtname, char* reltname);

int open_table(char* tname, int rec_size);
int open_db(char* dbname, int num_tables, char** tnames, int* rec_sizes, int num_rels, char** reltnames);
int open_relation(char* reltname);

int store_table(int key, void* c, char* tname);
int store_relation(char* reltname, int pkey, int fkey);

int get_table(int key, void* c, char* tname);
int get_relation(void* related_rec, int pkey, char* reltname);
int get_table_by_field(void* result, void* field_val, int field_offset, int field_size, char* tname);

int update_table(int key, void* c, char* tname);

int delete_table(int key, char* tname);
int delete_relation(int key, char* tname);

int undelete_table(int key, char* tname);
int undelete_relation(int key, char* tname);

int close_table(struct TableInfo* t);
int close_relation();
int close_db();