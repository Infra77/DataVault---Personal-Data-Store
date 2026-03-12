#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pds4.h"

struct DBInfo db_info;


// INIT
void init_table(struct TableInfo* t){
    t->rec_count=0;
    t->ndxfile=NULL;
    t->tfile=NULL;
    strcpy(t->tname, "");
    strcpy(t->ndxname, "");
}

void init_rel(struct RelInfo* r){
    strcpy(r->ptname, "");
    strcpy(r->rtname, "");
    strcpy(r->reltname, "");
    r->rel_count=0;
    r->reltfile=NULL;
    r->rel_status=REL_CLOSE;
}

void init(int num_tables, int num_relations){
    db_info.table=malloc(num_tables * sizeof(struct TableInfo));
    db_info.table_capacity=num_tables;
    db_info.num_table=0;
    db_info.rel_info=malloc(num_relations * sizeof(struct RelInfo));
    db_info.rel_capacity=num_relations;
    db_info.num_relations=0;
    db_info.db_status=DB_CLOSE;
    strcpy(db_info.dbname, "");
    for(int i=0; i<num_tables; i++) init_table(&db_info.table[i]);
    for(int i=0; i<num_relations; i++) init_rel(&db_info.rel_info[i]);
}


// CREATE
int create_table(char* tname){
    char fname[100];
    char ndxname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");
    strcpy(ndxname, tname);
    strcat(ndxname, ".ndx");

    FILE* tfile=fopen(fname, "wb+");
    FILE* ndxfile=fopen(ndxname, "wb+");
    if(tfile==NULL || ndxfile==NULL){
        if(tfile) fclose(tfile);
        if(ndxfile) fclose(ndxfile);
        return FAILURE;
    }
    int zero=0;
    fwrite(&zero, sizeof(int), 1, ndxfile);
    fclose(tfile);
    fclose(ndxfile);
    return SUCCESS;
}

int create_db(int num_tables, char** tnames){
    for(int i=0; i<num_tables; i++){
        if(create_table(tnames[i])!=SUCCESS) return FAILURE;
    }
    db_info.db_status=DB_CLOSE;
    return SUCCESS;
}

int create_relation(char* ptname, char* rtname, char* reltname){
    struct RelInfo r;
    init_rel(&r);
    strcpy(r.ptname, ptname);
    strcat(r.ptname, ".dat");
    strcpy(r.rtname, rtname);
    strcat(r.rtname, ".dat");
    strcpy(r.reltname, reltname);
    strcat(r.reltname, ".dat");

    r.reltfile=fopen(r.reltname, "wb+");
    if(r.reltfile==NULL) return FAILURE;
    int zero=0;
    fwrite(&zero, sizeof(int), 1, r.reltfile);
    fwrite(r.ptname, sizeof(char), 100, r.reltfile);
    fwrite(r.rtname, sizeof(char), 100, r.reltfile);
    fclose(r.reltfile);
    return SUCCESS;
}


// OPEN
int open_table(char* tname, int rec_size){
    if(db_info.num_table>=db_info.table_capacity) return DB_FULL;

    struct TableInfo* t=&db_info.table[db_info.num_table];
    init_table(t);
    strcpy(t->tname, tname);
    strcat(t->tname, ".dat");
    strcpy(t->ndxname, tname);
    strcat(t->ndxname, ".ndx");

    t->tfile=fopen(t->tname, "rb+");
    t->ndxfile=fopen(t->ndxname, "rb+");
    if(t->tfile==NULL || t->ndxfile==NULL){
        if(t->tfile) fclose(t->tfile);
        if(t->ndxfile) fclose(t->ndxfile);
        t->tfile=NULL;
        t->ndxfile=NULL;
        return FAILURE;
    }
    t->rec_size=rec_size;
    t->rec_count=0;
    fread(&t->rec_count, sizeof(int), 1, t->ndxfile);
    fread(t->ndxarray, sizeof(struct RecNdx), t->rec_count, t->ndxfile);
    db_info.num_table++;
    return SUCCESS;
}

int open_relation(char* reltname){
    if(db_info.num_relations>=db_info.rel_capacity) return DB_FULL;

    struct RelInfo* r=&db_info.rel_info[db_info.num_relations];
    init_rel(r);
    strcpy(r->reltname, reltname);
    strcat(r->reltname, ".dat");

    r->reltfile=fopen(r->reltname, "rb+");
    if(r->reltfile==NULL) return FAILURE;
    fread(&r->rel_count, sizeof(int), 1, r->reltfile);
    fread(r->ptname, sizeof(char), 100, r->reltfile);
    fread(r->rtname, sizeof(char), 100, r->reltfile);
    fread(r->relations, sizeof(struct RelPair), r->rel_count, r->reltfile);
    r->rel_status=REL_OPEN;
    db_info.num_relations++;
    return SUCCESS;
}

int open_db(char* dbname, int num_tables, char** tnames, int* rec_sizes, int num_rels, char** reltnames){
    if(db_info.db_status==DB_OPEN) return FAILURE;
    strcpy(db_info.dbname, dbname);

    if(load_schema(dbname)==SUCCESS) return SUCCESS;

    for(int i=0; i<num_tables; i++){
        if(open_table(tnames[i], rec_sizes[i])!=SUCCESS) return FAILURE;
    }
    for(int i=0; i<num_rels; i++){
        if(open_relation(reltnames[i])!=SUCCESS) return FAILURE;
    }
    db_info.db_status=DB_OPEN;
    return SUCCESS;
}


// STORE
int store_table(int key, void* c, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t=NULL;
    for(int i=0; i<db_info.num_table; i++){
        if(strcmp(db_info.table[i].tname, fname)==0){
            t=&db_info.table[i];
            break;
        }
    }
    if(t==NULL) return REC_NOT_FOUND;

    fseek(t->tfile, 0L, SEEK_END);
    int loc=ftell(t->tfile);
    fwrite(c, t->rec_size, 1, t->tfile);

    struct RecNdx* ndx=&t->ndxarray[t->rec_count];
    ndx->key=key;
    ndx->old_key=key;
    ndx->loc=loc;
    ndx->is_deleted=0;
    t->rec_count++;
    return SUCCESS;
}

int store_relation(char* reltname, int pkey, int fkey){
    char fname[100];
    strcpy(fname, reltname);
    strcat(fname, ".dat");

    struct RelInfo* r=NULL;
    for(int i=0; i<db_info.num_relations; i++){
        if(strcmp(db_info.rel_info[i].reltname, fname)==0){
            r=&db_info.rel_info[i];
            break;
        }
    }
    if(r==NULL || r->rel_status==REL_CLOSE) return FAILURE;

    struct RelPair rp;
    rp.pkey=pkey;
    rp.fkey=fkey;
    rp.is_deleted=0;
    fseek(r->reltfile, 0L, SEEK_END);
    fwrite(&rp, sizeof(struct RelPair), 1, r->reltfile);
    r->relations[r->rel_count]=rp;
    r->rel_count++;
    return SUCCESS;
}


// GET
int get_table(int key, void* c, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t=NULL;
    for(int i=0; i<db_info.num_table; i++){
        if(strcmp(db_info.table[i].tname, fname)==0){
            t=&db_info.table[i];
            break;
        }
    }
    if(t==NULL) return REC_NOT_FOUND;

    for(int i=0; i<t->rec_count; i++){
        if(t->ndxarray[i].is_deleted) continue;
        if(t->ndxarray[i].key==key){
            fseek(t->tfile, t->ndxarray[i].loc, SEEK_SET);
            fread(c, t->rec_size, 1, t->tfile);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int get_relation(void* related_rec, int pkey, char* reltname){
    char fname[100];
    strcpy(fname, reltname);
    strcat(fname, ".dat");

    struct RelInfo* r=NULL;
    for(int i=0; i<db_info.num_relations; i++){
        if(strcmp(db_info.rel_info[i].reltname, fname)==0){
            r=&db_info.rel_info[i];
            break;
        }
    }
    if(r==NULL || r->rel_status==REL_CLOSE) return FAILURE;

    for(int i=0; i<r->rel_count; i++){
        if(r->relations[i].is_deleted) continue;
        if(r->relations[i].pkey==pkey){
            int fkey=r->relations[i].fkey;
            char tname[100];
            strcpy(tname, r->rtname);
            char* dot=strrchr(tname, '.');
            if(dot) *dot='\0';
            return get_table(fkey, related_rec, tname);
        }
    }
    return REC_NOT_FOUND;
}

int get_table_by_field(void* result, void* field_val, int field_offset, int field_size, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t=NULL;
    for(int i=0; i<db_info.num_table; i++){
        if(strcmp(db_info.table[i].tname, fname)==0){
            t=&db_info.table[i];
            break;
        }
    }
    if(t==NULL) return REC_NOT_FOUND;

    char buf[1024];
    for(int i=0; i<t->rec_count; i++){
        if(t->ndxarray[i].is_deleted) continue;
        fseek(t->tfile, t->ndxarray[i].loc, SEEK_SET);
        fread(buf, t->rec_size, 1, t->tfile);
        if(memcmp(buf+field_offset, field_val, field_size)==0){
            memcpy(result, buf, t->rec_size);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}


// UPDATE
int update_table(int key, void* c, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t=NULL;
    for(int i=0; i<db_info.num_table; i++){
        if(strcmp(db_info.table[i].tname, fname)==0){
            t=&db_info.table[i];
            break;
        }
    }
    if(t==NULL) return REC_NOT_FOUND;

    for(int i=0; i<t->rec_count; i++){
        if(t->ndxarray[i].is_deleted) continue;
        if(t->ndxarray[i].key==key){
            fseek(t->tfile, t->ndxarray[i].loc, SEEK_SET);
            fwrite(c, t->rec_size, 1, t->tfile);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}


// DELETE
int delete_table(int key, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t=NULL;
    for(int i=0; i<db_info.num_table; i++){
        if(strcmp(db_info.table[i].tname, fname)==0){
            t=&db_info.table[i];
            break;
        }
    }
    if(t==NULL) return REC_NOT_FOUND;

    for(int i=0; i<t->rec_count; i++){
        if(t->ndxarray[i].is_deleted) continue;
        if(t->ndxarray[i].key==key){
            t->ndxarray[i].old_key=key;
            t->ndxarray[i].key=-1;
            t->ndxarray[i].is_deleted=1;
            delete_relation(key, tname);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int delete_relation(int key, char* tname){
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    if(db_info.num_relations==0) return FAILURE;

    int found=0;
    for(int i=0; i<db_info.num_relations; i++){
        struct RelInfo* r=&db_info.rel_info[i];
        if(strcmp(r->ptname, fname)==0){
            for(int j=0; j<r->rel_count; j++){
                if(r->relations[j].pkey==key && !r->relations[j].is_deleted){
                    r->relations[j].is_deleted=1;
                    found=1;
                    break;
                }
            }
        }
        else if(strcmp(r->rtname, fname)==0){
            for(int j=0; j<r->rel_count; j++){
                if(r->relations[j].fkey==key && !r->relations[j].is_deleted){
                    r->relations[j].is_deleted=1;
                    found=1;
                    break;
                }
            }
        }
    }
    return found ? SUCCESS : REC_NOT_FOUND;
}


// UNDELETE
int undelete_table(int key, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t=NULL;
    for(int i=0; i<db_info.num_table; i++){
        if(strcmp(db_info.table[i].tname, fname)==0){
            t=&db_info.table[i];
            break;
        }
    }
    if(t==NULL) return REC_NOT_FOUND;

    for(int i=0; i<t->rec_count; i++){
        if(t->ndxarray[i].old_key==key && t->ndxarray[i].is_deleted){
            t->ndxarray[i].key=key;
            t->ndxarray[i].is_deleted=0;
            undelete_relation(key, tname);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}

int undelete_relation(int key, char* tname){
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    if(db_info.num_relations==0) return FAILURE;

    int found=0;
    for(int i=0; i<db_info.num_relations; i++){
        struct RelInfo* r=&db_info.rel_info[i];
        if(strcmp(r->ptname, fname)==0){
            for(int j=0; j<r->rel_count; j++){
                if(r->relations[j].pkey==key && r->relations[j].is_deleted){
                    r->relations[j].is_deleted=0;
                    found=1;
                    break;
                }
            }
        }
        else if(strcmp(r->rtname, fname)==0){
            for(int j=0; j<r->rel_count; j++){
                if(r->relations[j].fkey==key && r->relations[j].is_deleted){
                    r->relations[j].is_deleted=0;
                    found=1;
                    break;
                }
            }
        }
    }
    return found ? SUCCESS : REC_NOT_FOUND;
}


// CLOSE
int close_table(struct TableInfo* t){
    fseek(t->ndxfile, 0L, SEEK_SET);
    fwrite(&t->rec_count, sizeof(int), 1, t->ndxfile);
    fwrite(t->ndxarray, sizeof(struct RecNdx), t->rec_count, t->ndxfile);
    fclose(t->tfile);
    fclose(t->ndxfile);
    t->tfile=NULL;
    t->ndxfile=NULL;
    return SUCCESS;
}

int close_relation(){
    for(int i=0; i<db_info.num_relations; i++){
        struct RelInfo* r=&db_info.rel_info[i];
        if(r->reltfile==NULL) continue;
        fseek(r->reltfile, 0L, SEEK_SET);
        fwrite(&r->rel_count, sizeof(int), 1, r->reltfile);
        fwrite(r->ptname, sizeof(char), 100, r->reltfile);
        fwrite(r->rtname, sizeof(char), 100, r->reltfile);
        fwrite(r->relations, sizeof(struct RelPair), r->rel_count, r->reltfile);
        fclose(r->reltfile);
        r->reltfile=NULL;
        r->rel_status=REL_CLOSE;
    }
    db_info.num_relations=0;
    return SUCCESS;
}

int close_db(){
    if(strlen(db_info.dbname)>0){
        save_schema(db_info.dbname);
    }
    for(int i=0; i<db_info.num_table; i++){
        close_table(&db_info.table[i]);
    }
    db_info.num_table=0;
    close_relation();
    db_info.db_status=DB_CLOSE;
    return SUCCESS;
}


// SCHEMA
int save_schema(char* dbname){
    char fname[110];
    strcpy(fname, dbname);
    strcat(fname, ".sch");

    FILE* f=fopen(fname, "wb");
    if(f==NULL) return FAILURE;
    fwrite(&db_info.num_table, sizeof(int), 1, f);
    fwrite(&db_info.num_relations, sizeof(int), 1, f);
    for(int i=0; i<db_info.num_table; i++){
        struct TableInfo* t=&db_info.table[i];
        fwrite(t->tname, sizeof(char), 100, f);
        fwrite(t->ndxname, sizeof(char), 100, f);
        fwrite(&t->rec_size, sizeof(int), 1, f);
        fwrite(&t->rec_count, sizeof(int), 1, f);
        fwrite(t->ndxarray, sizeof(struct RecNdx), t->rec_count, f);
    }
    for(int i=0; i<db_info.num_relations; i++){
        struct RelInfo* r=&db_info.rel_info[i];
        fwrite(r->ptname, sizeof(char), 100, f);
        fwrite(r->rtname, sizeof(char), 100, f);
        fwrite(r->reltname, sizeof(char), 100, f);
        fwrite(&r->rel_count, sizeof(int), 1, f);
        fwrite(r->relations, sizeof(struct RelPair), r->rel_count, f);
    }
    fclose(f);
    return SUCCESS;
}

int load_schema(char* dbname){
    char fname[110];
    strcpy(fname, dbname);
    strcat(fname, ".sch");

    FILE* f=fopen(fname, "rb");
    if(f==NULL) return FAILURE;
    int num_tables=0, num_rels=0;
    fread(&num_tables, sizeof(int), 1, f);
    fread(&num_rels, sizeof(int), 1, f);

    db_info.table=malloc(num_tables * sizeof(struct TableInfo));
    db_info.table_capacity=num_tables;
    db_info.num_table=num_tables;
    for(int i=0; i<num_tables; i++){
        struct TableInfo* t=&db_info.table[i];
        fread(t->tname, sizeof(char), 100, f);
        fread(t->ndxname, sizeof(char), 100, f);
        fread(&t->rec_size, sizeof(int), 1, f);
        fread(&t->rec_count, sizeof(int), 1, f);
        fread(t->ndxarray, sizeof(struct RecNdx), t->rec_count, f);
        t->tfile=fopen(t->tname, "rb+");
        t->ndxfile=fopen(t->ndxname, "rb+");
    }

    db_info.rel_info=malloc(num_rels * sizeof(struct RelInfo));
    db_info.rel_capacity=num_rels;
    db_info.num_relations=num_rels;
    for(int i=0; i<num_rels; i++){
        struct RelInfo* r=&db_info.rel_info[i];
        fread(r->ptname, sizeof(char), 100, f);
        fread(r->rtname, sizeof(char), 100, f);
        fread(r->reltname, sizeof(char), 100, f);
        fread(&r->rel_count, sizeof(int), 1, f);
        fread(r->relations, sizeof(struct RelPair), r->rel_count, f);
        r->reltfile=fopen(r->reltname, "rb+");
        r->rel_status=REL_OPEN;
    }

    fclose(f);
    strcpy(db_info.dbname, dbname);
    db_info.db_status=DB_OPEN;
    return SUCCESS;
}