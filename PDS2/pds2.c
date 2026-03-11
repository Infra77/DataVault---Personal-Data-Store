#include <stdio.h>
#include <string.h>
#include "pds2.h"

struct DBInfo db_info;

void init_table(struct TableInfo* t){
    t->rec_count=0;
    t->ndxfile=NULL;
    t->tfile=NULL;
    strcpy(t->tname, "");
}
void Info_init(int n){
    for(int i=0;i<n;i++){
        init_table(&db_info.table[i]);
    }
    db_info.num_table=0;
    db_info.db_status=DB_CLOSE;
}



int create_table(char* tname){
    struct TableInfo t;

    char ndxname[100];
    strcpy(ndxname, tname);
    strcat(tname, ".dat");
    strcat(ndxname, ".ndx");
    fopen(tname, "wb+");
    fopen(ndxname, "wb+");

    t.tfile = fopen(tname, "wb+");
    t.ndxfile = fopen(ndxname, "wb+");
    if(t.tfile==NULL || t.ndxfile==NULL){
        fclose(t.tfile);
        fclose(t.ndxfile);
        return FAILURE;
    }
    return SUCCESS;

}
int create_db(char* tname1, char* tname2){
    create_table(tname1);
    create_table(tname2);
    db_info.db_status=DB_CLOSE;
    return SUCCESS;
}



int open_table(char* tname, int rec_size){
    struct TableInfo* t=&db_info.table[db_info.num_table];
    strcpy(t->tname, tname);
    strcat(t->tname, ".dat");

    strcpy(t->ndxname, tname);
    strcat(t->ndxname, ".ndx");

    t->tfile=fopen(t->tname, "rb+");
    t->ndxfile=fopen(t->ndxname, "rb+");

    if(t->tfile==NULL || t->ndxfile==NULL){
        fclose(t->tfile);
        fclose(t->ndxfile);
        return FAILURE;
    }

    t->rec_size=rec_size;    
    t->rec_count=0;
    int key=0;
    int n;
    fread(&t->rec_count, sizeof(int), 1, t->ndxfile);
    fread(t->ndxarray, sizeof(struct RecNdx), t->rec_count, t->ndxfile);
    
    db_info.num_table++;
    return SUCCESS;
}
int open_db(char *tname1, int rec_size1, char *tname2, int rec_size2){
    if(db_info.db_status==DB_OPEN) return FAILURE;  
    open_table(tname1, rec_size1); 
    open_table(tname2, rec_size2);
    db_info.db_status=DB_OPEN;

    return SUCCESS;
}



int store_table(int key, void* c, char* tname){
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t;
    int found=0;
    for(int i=0;i<db_info.num_table;i++){
        t=&db_info.table[i];
        if(strcmp(t->tname, fname)==0){
            found=1;
            break;
        }
    }
    if(!found){
        return REC_NOT_FOUND;
    }
    
    fseek(t->tfile, 0L, SEEK_END);
    fwrite(c, t->rec_size, 1, t->tfile);

    struct RecNdx temp;
    temp.key=key;
    temp.old_key=key;
    temp.loc=ftell(t->tfile)-t->rec_size;
    t->ndxarray[t->rec_count]=temp;
    t->ndxarray[t->rec_count].is_deleted=0;
    t->rec_count++;

    return SUCCESS;

}

int get_table(int key, void* c, char* tname){
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t;
    int found=0;
    for(int i=0;i<db_info.num_table;i++){
        t=&db_info.table[i];
        if(strcmp(t->tname, fname)==0){
            found=1;
            break;
        }
    }
    if(!found){
        return REC_NOT_FOUND;
    }
    int found_record=0;
    int loc=0;
    for(int i=0;i<t->rec_count;i++){
        if(t->ndxarray[i].is_deleted) continue;
        if(t->ndxarray[i].key==key){
            found_record=1;
            loc=t->ndxarray[i].loc;
            break;
        }
    }

    if(!found_record){
        return REC_NOT_FOUND;
    }
    fseek(t->tfile, loc, SEEK_SET);

    fread(c, t->rec_size, 1, t->tfile);
    return SUCCESS;
}

int update_table(int key, void* c, char* tname){
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t;
    int found=0;
    for(int i=0;i<db_info.num_table;i++){
        t=&db_info.table[i];
        if(strcmp(t->tname, fname)==0){
            found=1;
            break;
        }
    }
    if(!found){
        return REC_NOT_FOUND;
    }
    int found_record=0;
    for(int i=0;i<t->rec_count;i++){
        struct RecNdx temp;
        temp=t->ndxarray[i];
        if(temp.is_deleted){
            continue;
        }
        if(temp.key==key){
            found_record=1;
            fseek(t->tfile, temp.loc, SEEK_SET);
            fwrite(c, t->rec_size, 1, t->tfile);
            t->ndxarray[i].key=key;
            break;
        }
    }
    if(!found_record) return REC_NOT_FOUND;

    return SUCCESS;
}

int delete_table(int key, char* tname){
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    struct TableInfo* t;
    int found=0;
    for(int i=0;i<db_info.num_table;i++){
        t=&db_info.table[i];
        if(strcmp(t->tname, fname)==0){
            found=1;
            break;
        }
    }
    if(!found){
        return REC_NOT_FOUND;
    }

    int found_record=0;
    for(int i=0;i<t->rec_count;i++){
        struct RecNdx temp;
        temp=t->ndxarray[i];
        if(temp.key==key){
            found_record=1;
            t->ndxarray[i].old_key=key;
            t->ndxarray[i].key=-1;
            t->ndxarray[i].is_deleted=1;
            break;
        }
    }
    if(!found_record) return REC_NOT_FOUND;

    return SUCCESS;
}
int undelete_table(int key, char* tname){
    char fname[100];
    strcpy(fname, tname);
    strcat(fname, ".dat");

    int found=0;
    struct TableInfo* t;
    for(int i=0;i<db_info.num_table;i++){
        t=&db_info.table[i];
        if(strcmp(fname, t->tname)==0){
            found=1;
            break;
        }
    }
    if(!found) return REC_NOT_FOUND;

    int found_record=0;
    for(int i=0;i<t->rec_count;i++){
        struct RecNdx temp;
        temp=t->ndxarray[i];
        if(temp.old_key==key && temp.is_deleted){
            found_record=1;
            t->ndxarray[i].key=key;
            t->ndxarray[i].is_deleted=0;
            break;
        }
    }
    if(!found_record) return REC_NOT_FOUND;

    return SUCCESS;
}

int close_table(struct TableInfo* t){
    fseek(t->ndxfile, 0L, SEEK_SET);
    fwrite(&t->rec_count, sizeof(int), 1, t->ndxfile);
    fwrite(t->ndxarray, sizeof(struct RecNdx), t->rec_count, t->ndxfile);
    fclose(t->tfile);
    fclose(t->ndxfile);
    t->ndxfile=NULL;
    t->tfile=NULL;
}
int close_db(){
    for(int i=0;i<db_info.num_table;i++){
        close_table(&db_info.table[i]);
    }
    db_info.num_table=0;
    db_info.db_status=DB_CLOSE;
    return SUCCESS;
}
