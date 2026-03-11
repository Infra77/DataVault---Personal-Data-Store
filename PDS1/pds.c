#include <stdio.h>
#include <string.h>
#include "pds.h"

struct Info db_info;

void Info_init(){
    db_info.ndxfile=NULL;
    db_info.dbfile=NULL;
    
    db_info.status=DB_CLOSE;
    db_info.rec_count=0;
}

int create_db(char* dbname){
    strcpy(db_info.ndxname, dbname);
    strcat(db_info.ndxname, ".ndx");

    strcpy(db_info.dbname, dbname);
    strcat(db_info.dbname, ".dat");
    db_info.ndxfile=fopen(db_info.ndxname, "wb");
    db_info.dbfile=fopen(db_info.dbname, "wb");
    if(db_info.dbfile==NULL) return FAILURE;
    fclose(db_info.dbfile);
    db_info.status=DB_CLOSE;
    return SUCCESS;
}

int open_db(char *dbname, int rec_size){
    if(db_info.status==DB_OPEN) return FAILURE;
    if(strcmp(db_info.dbname, "")==0) return FAILURE;

    db_info.rec_size=rec_size;
    db_info.dbfile=fopen(db_info.dbname, "rb+");
    db_info.ndxfile=fopen(db_info.ndxname, "rb+");
    if(db_info.dbfile==NULL || db_info.ndxfile==NULL){
        fclose(db_info.dbfile);
        fclose(db_info.ndxfile);
        return FAILURE;
    }
    struct Rec_ndx pair;
    fread(&db_info.rec_count, sizeof(int), 1, db_info.ndxfile);
    fread(db_info.ndxArray, db_info.rec_size, db_info.rec_count, db_info.ndxfile);
    db_info.status=DB_OPEN;
    fclose(db_info.ndxfile);
    return SUCCESS;
}

int store_db(int key, void* c){
    if(db_info.dbfile==NULL) return FAILURE;
    fseek(db_info.dbfile, 0L, SEEK_END);
    struct Rec_ndx temp;
    temp.key=key;
    temp.old_key=key;
    temp.loc=ftell(db_info.dbfile);
    db_info.ndxArray[db_info.rec_count]=temp;
    db_info.ndxArray[db_info.rec_count].is_deleted=0;
    db_info.rec_count++;
    fwrite(c, db_info.rec_size, 1, db_info.dbfile);

    return SUCCESS;
}

int get_db(int key, void* c){
    if(db_info.dbfile==NULL) return FAILURE;
    
    int found_record=0;
    struct Rec_ndx pair;
    int loc=0;
    for(int i=0;i<db_info.rec_count;i++){
        pair=db_info.ndxArray[i];
        if(pair.is_deleted) continue;
        if(pair.key==key){
            found_record=1;
            loc=db_info.ndxArray[i].loc;
            break;
        }
    }
    if(!found_record) return REC_NOT_FOUND;
    fseek(db_info.dbfile, loc, SEEK_SET);
    fread(c, db_info.rec_size, 1, db_info.dbfile);
    return SUCCESS;
}

int update_db(int key, void* c){
    if(db_info.dbfile==NULL) return FAILURE;
    
    int found_record=0;
    for(int i=0;i<db_info.rec_count;i++){
        struct Rec_ndx  temp;
        temp=db_info.ndxArray[i];
        if(temp.is_deleted){
            continue;
        }
        if(temp.key==key){
            found_record=1;
            fseek(db_info.dbfile, temp.loc, SEEK_SET);
            fwrite(c, db_info.rec_size, 1, db_info.dbfile);
            // db_info.ndxArray[i].key=key;
            break;
        }
    }
    if(!found_record) return REC_NOT_FOUND;

    return SUCCESS;
}

int delete_db(int key){
    
    int found_record=0;
    for(int i=0;i<db_info.rec_count;i++){
        struct Rec_ndx  temp;
        temp=db_info.ndxArray[i];
        if(temp.key==key){
            found_record=1;
            db_info.ndxArray[i].old_key=key;
            db_info.ndxArray[i].key=-1;
            db_info.ndxArray[i].is_deleted=1;
            break;
        }
    }
    if(!found_record) return REC_NOT_FOUND;

    return SUCCESS;
}
int undelete_db(int key){
    
    int found_record=0;
    for(int i=0;i<db_info.rec_count;i++){
        struct Rec_ndx  temp;
        temp=db_info.ndxArray[i];
        if(temp.old_key==key && temp.is_deleted){
            found_record=1;
            db_info.ndxArray[i].key=key;
            db_info.ndxArray[i].is_deleted=0;
            break;
        }
    }
    if(!found_record) return REC_NOT_FOUND;

    return SUCCESS;
}
int close_db(){
    // fseek(db_info.ndxfile, 0L, SEEK_SET);
    db_info.ndxfile=fopen(db_info.ndxname, "rb+");
    fwrite(&db_info.rec_count, sizeof(int), 1, db_info.ndxfile);
    fwrite(db_info.ndxArray, sizeof(struct Rec_ndx), db_info.rec_count, db_info.ndxfile);
    fclose(db_info.dbfile);
    db_info.dbfile=NULL;
    db_info.status=DB_CLOSE;
    db_info.dbfile=NULL;
    db_info.ndxfile=NULL;
    return SUCCESS;
}