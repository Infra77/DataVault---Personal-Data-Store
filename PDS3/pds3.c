#include <stdio.h>
#include <string.h>
#include "pds3.h"

struct DBInfo db_info;



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
}

void init(int n){
    for(int i = 0; i < n; i++){
        init_table(&db_info.table[i]);
    }
    init_rel(&db_info.rel_info);
    db_info.num_table=0;
    db_info.db_status=DB_CLOSE;
    db_info.rel_status=REL_CLOSE;
}




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

    int zero = 0;
    fwrite(&zero, sizeof(int), 1, ndxfile);

    fclose(tfile);
    fclose(ndxfile);
    return SUCCESS;
}

int create_db(char* tname1, char* tname2){
    if(create_table(tname1)!=SUCCESS) return FAILURE;
    if(create_table(tname2)!=SUCCESS) return FAILURE;
    db_info.db_status=DB_CLOSE;
    return SUCCESS;
}

int create_relation(char* ptname, char* rtname, char* reltname){
    strcpy(db_info.rel_info.ptname, ptname);
    strcat(db_info.rel_info.ptname, ".dat");

    strcpy(db_info.rel_info.rtname, rtname);
    strcat(db_info.rel_info.rtname, ".dat");

    strcpy(db_info.rel_info.reltname, reltname);
    strcat(db_info.rel_info.reltname, ".dat");

    db_info.rel_info.reltfile = fopen(db_info.rel_info.reltname, "wb+");
    if(db_info.rel_info.reltfile == NULL) return FAILURE;

    int zero = 0;
    fwrite(&zero, sizeof(int), 1, db_info.rel_info.reltfile);
    fwrite(db_info.rel_info.ptname, sizeof(char), 100, db_info.rel_info.reltfile);
    fwrite(db_info.rel_info.rtname, sizeof(char), 100, db_info.rel_info.reltfile);

    fclose(db_info.rel_info.reltfile);
    db_info.rel_info.reltfile = NULL;
    return SUCCESS;
}




int open_table(char* tname, int rec_size){
    struct TableInfo* t = &db_info.table[db_info.num_table];

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

int open_db(char* tname1, int rec_size1, char* tname2, int rec_size2){
    if(db_info.db_status == DB_OPEN) return FAILURE;
    if(open_table(tname1, rec_size1)!=SUCCESS) return FAILURE;
    if(open_table(tname2, rec_size2)!=SUCCESS) return FAILURE;
    db_info.db_status=DB_OPEN;
    return SUCCESS;
}

int open_relation(char* reltname){
    if(db_info.rel_status == REL_OPEN) return FAILURE;

    strcpy(db_info.rel_info.reltname, reltname);
    strcat(db_info.rel_info.reltname, ".dat");

    db_info.rel_info.reltfile = fopen(db_info.rel_info.reltname, "rb+");
    if(db_info.rel_info.reltfile == NULL) return FAILURE;

    db_info.rel_info.rel_count = 0;
    fread(&db_info.rel_info.rel_count, sizeof(int), 1, db_info.rel_info.reltfile);
    fread(db_info.rel_info.ptname, sizeof(char), 100, db_info.rel_info.reltfile);
    fread(db_info.rel_info.rtname, sizeof(char), 100, db_info.rel_info.reltfile);
    fread(db_info.rel_info.relations, sizeof(struct RelPair), db_info.rel_info.rel_count, db_info.rel_info.reltfile);

    db_info.rel_status = REL_OPEN;
    return SUCCESS;
}




int store_table(int key, void* c, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname); 
    strcat(fname, ".dat");

    struct TableInfo* t = NULL;
    for(int i=0;i<db_info.num_table;i++){
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
int store_relation(int pkey, int fkey){
    if(db_info.rel_status==REL_CLOSE) return FAILURE;

    struct RelPair rp;
    rp.pkey= pkey;
    rp.fkey= fkey;
    rp.is_deleted = 0;

    fseek(db_info.rel_info.reltfile, 0L, SEEK_END);
    fwrite(&rp, sizeof(struct RelPair), 1, db_info.rel_info.reltfile);

    db_info.rel_info.relations[db_info.rel_info.rel_count] = rp;
    db_info.rel_info.rel_count++;

    return SUCCESS;
}




int get_table(int key, void* c, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname); 
    strcat(fname, ".dat");

    struct TableInfo* t = NULL;
    for(int i=0;i<db_info.num_table;i++){
        if(strcmp(db_info.table[i].tname, fname)==0){
            t=&db_info.table[i];
            break;
        }
    }
    if(t==NULL) return REC_NOT_FOUND;

    int found=0;
    for(int i=0; i<t->rec_count;i++){
        if(t->ndxarray[i].is_deleted) continue;
        if(t->ndxarray[i].key==key){
            fseek(t->tfile, t->ndxarray[i].loc, SEEK_SET);
            fread(c, t->rec_size, 1, t->tfile);
            found=1;
            break;
        }
    }

    if(!found) return REC_NOT_FOUND;

    return SUCCESS;
}

int get_relation(void* related_rec, int pkey){
    if(db_info.rel_status==REL_CLOSE) return FAILURE;

    int fkey=-1;
    int found=0;
    for(int i=0;i<db_info.rel_info.rel_count;i++){
        struct RelPair* rp=&db_info.rel_info.relations[i];
        if(rp->pkey==pkey && !rp->is_deleted){
            fkey=rp->fkey;
            found=1;
            break;
        }
    }
    if(!found) return REC_NOT_FOUND;

    char tname[100];
    strcpy(tname, db_info.rel_info.rtname);
    char* dot = strrchr(tname, '.');
    if(dot) *dot = '\0';

    return get_table(fkey, related_rec, tname);
}




int update_table(int key, void* c, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname); 
    strcat(fname, ".dat");

    struct TableInfo* t = NULL;
    for(int i = 0; i < db_info.num_table; i++){
        if(strcmp(db_info.table[i].tname, fname) == 0){
            t = &db_info.table[i];
            break;
        }
    }
    if(t==NULL) return REC_NOT_FOUND;

    for(int i=0;i<t->rec_count;i++){
        if(t->ndxarray[i].is_deleted) continue;
        if(t->ndxarray[i].key == key){
            fseek(t->tfile, t->ndxarray[i].loc, SEEK_SET);
            fwrite(c, t->rec_size, 1, t->tfile);
            return SUCCESS;
        }
    }
    return REC_NOT_FOUND;
}




int delete_table(int key, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname); 
    strcat(fname, ".dat");

    struct TableInfo* t = NULL;
    for(int i=0;i<db_info.num_table;i++){
        if(strcmp(db_info.table[i].tname, fname) == 0){
            t = &db_info.table[i];
            break;
        }
    }
    if(t == NULL) return REC_NOT_FOUND;

    int found=0;
    for(int i = 0; i < t->rec_count; i++){
        if(t->ndxarray[i].is_deleted) continue;
        if(t->ndxarray[i].key == key){
            t->ndxarray[i].old_key=key;
            t->ndxarray[i].key=-1;
            t->ndxarray[i].is_deleted=1;
            delete_relation(key, tname);
            found=1;
            break;
        }
    }

    if(!found) return REC_NOT_FOUND;

    return SUCCESS;
}

int delete_relation(int key, char* tname){
    char fname[100];
    strcpy(fname, tname); 
    strcat(fname, ".dat");

    if(db_info.rel_status==REL_CLOSE) return FAILURE;
    int found=0;
    if(strcmp(db_info.rel_info.ptname, fname) == 0){
        for(int i=0;i<db_info.rel_info.rel_count;i++){
            struct RelPair* rp = &db_info.rel_info.relations[i];
            if(rp->pkey == key && !rp->is_deleted){
                rp->is_deleted=1;
                found=1;
                break;
            }
        }
    }
    else if(strcmp(db_info.rel_info.rtname, fname) == 0){
        for(int i=0;i<db_info.rel_info.rel_count;i++){
            struct RelPair* rp=&db_info.rel_info.relations[i];
            if(rp->fkey == key && !rp->is_deleted){
                rp->is_deleted = 1;
                found=1;
                break;
            }
        }
    }
    if(!found){
        return REC_NOT_FOUND;
    }

    return SUCCESS;
}




int undelete_table(int key, char* tname){
    if(db_info.db_status==DB_CLOSE) return FAILURE;
    char fname[100];
    strcpy(fname, tname); 
    strcat(fname, ".dat");

    struct TableInfo* t = NULL;
    for(int i=0;i<db_info.num_table;i++){
        if(strcmp(db_info.table[i].tname, fname) == 0){
            t=&db_info.table[i];
            break;
        }
    }
    if(t == NULL) return REC_NOT_FOUND;

    int found=0;
    for(int i=0;i<t->rec_count;i++){
        if(t->ndxarray[i].old_key == key && t->ndxarray[i].is_deleted){
            t->ndxarray[i].key=key;
            t->ndxarray[i].is_deleted=0;
            /* cascade: I dont know if this is right to do or no */
            undelete_relation(key, tname);
            found=1;
            break;
        }
    }
    if(!found) return REC_NOT_FOUND;
    return SUCCESS;
}

int undelete_relation(int key, char* tname){
    char fname[100];
    strcpy(fname, tname); 
    strcat(fname, ".dat");

    if(db_info.rel_status == REL_CLOSE) return FAILURE;

    int found=0;
    if(strcmp(db_info.rel_info.ptname, fname) == 0){
        for(int i = 0; i < db_info.rel_info.rel_count; i++){
            struct RelPair* rp = &db_info.rel_info.relations[i];
            if(rp->pkey == key && rp->is_deleted){
                rp->is_deleted = 0;
                found=1; 
                break;
            }
        }
    }
    else if(strcmp(db_info.rel_info.rtname, fname) == 0){
        for(int i = 0; i < db_info.rel_info.rel_count; i++){
            struct RelPair* rp = &db_info.rel_info.relations[i];
            if(rp->fkey == key && rp->is_deleted){
                rp->is_deleted = 0;
                found=1; 
                break;
            }
        }
    }
    if(!found){
        return REC_NOT_FOUND;
    }

    return SUCCESS;
}




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
    if(db_info.rel_info.reltfile == NULL) return FAILURE;

    fseek(db_info.rel_info.reltfile, 0L, SEEK_SET);
    fwrite(&db_info.rel_info.rel_count, sizeof(int), 1, db_info.rel_info.reltfile);
    fwrite(db_info.rel_info.ptname, sizeof(char), 100, db_info.rel_info.reltfile);
    fwrite(db_info.rel_info.rtname, sizeof(char), 100, db_info.rel_info.reltfile);

    fwrite(db_info.rel_info.relations, sizeof(struct RelPair), db_info.rel_info.rel_count, db_info.rel_info.reltfile);
    fclose(db_info.rel_info.reltfile);

    db_info.rel_info.reltfile=NULL;
    db_info.rel_status=REL_CLOSE;
    return SUCCESS;
}

int close_db(){
    for(int i=0;i<db_info.num_table;i++){
        close_table(&db_info.table[i]);
    }
    db_info.num_table=0;
    close_relation();
    db_info.db_status=DB_CLOSE;
    return SUCCESS;
}