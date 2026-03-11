#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pds3.h"

typedef struct Hospital {
    int  hospital_id;
    char name[100];
    char address[200];
    char email[50];
} Hospital;

#define TREPORT(a1, a2) \
    do { printf("Status: %s - %s\n\n", a1, a2); fflush(stdout); } while(0)

void process_line(char *test_case);

int main(int argc, char *argv[])
{
    FILE *cfptr;
    char test_case[500];

    if(argc != 2){
        fprintf(stderr, "Usage: %s testcasefile\n", argv[0]);
        exit(1);
    }

    cfptr = fopen(argv[1], "r");
    if(!cfptr){
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        exit(1);
    }

    while(fgets(test_case, sizeof(test_case) - 1, cfptr)){
        if(!strcmp(test_case, "\n") || !strcmp(test_case, ""))
            continue;
        process_line(test_case);
    }

    fclose(cfptr);
    return 0;
}

void process_line(char *test_case)
{
    char    command[20], tname1[50], tname2[50], tname3[50], info[1024];
    char    name[100], address[200], email[50];
    int     key1, key2, expected_status, status;
    Hospital testHospital;

    sscanf(test_case, "%s", command);
    printf("Test case: %s", test_case); fflush(stdout);

    /* ----------------------------------------------------------------
       INIT
       Format: INIT <num_tables> <expected_status>
    ---------------------------------------------------------------- */
    if(!strcmp(command, "INIT")){
        int num_tables;
        sscanf(test_case, "%*s %d %d", &num_tables, &expected_status);
        init(num_tables);
        if(expected_status == SUCCESS) TREPORT("PASS", "");
        else TREPORT("FAIL", "init failed to execute");
    }

    /* ----------------------------------------------------------------
       CREATE DB
       Format: CREATE <tname1> <tname2> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "CREATE")){
        sscanf(test_case, "%*s %s %s %d", tname1, tname2, &expected_status);
        status = create_db(tname1, tname2);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "create_db returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       OPEN DB
       Format: OPEN <tname1> <tname2> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "OPEN")){
        sscanf(test_case, "%*s %s %s %d", tname1, tname2, &expected_status);
        status = open_db(tname1, sizeof(Hospital), tname2, sizeof(Hospital));
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "open_db returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       STORE RECORD
       Format: STORE <tname> <key> <expected_status> <name> <address> <email>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "STORE")){
        sscanf(test_case, "%*s %s %d %d %s %s %s",
               tname1, &key1, &expected_status, name, address, email);

        testHospital.hospital_id = key1;
        strcpy(testHospital.name,    name);
        strcpy(testHospital.address, address);
        strcpy(testHospital.email,   email);

        status = store_table(key1, &testHospital, tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "store_table returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       SEARCH TABLE
       Format: SEARCH <tname> <key> <expected_status> [<name> <address> <email>]
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "SEARCH")){
        sscanf(test_case, "%*s %s %d %d", tname1, &key1, &expected_status);
        status = get_table(key1, &testHospital, tname1);

        if(status != expected_status){
            sprintf(info, "get_table returned %d", status);
            TREPORT("FAIL", info);
        }
        else if(status == SUCCESS){
            sscanf(test_case, "%*s %*s %*d %*d %s %s %s", name, address, email);
            if(testHospital.hospital_id == key1 && !strcmp(testHospital.name, name))
                TREPORT("PASS", "");
            else
                TREPORT("FAIL", "Data mismatch");
        }
        else TREPORT("PASS", "");
    }

    /* ----------------------------------------------------------------
       UPDATE TABLE RECORD
       Format: UPDATE <tname> <key> <expected_status> <name> <address> <email>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "UPDATE")){
        sscanf(test_case, "%*s %s %d %d %s %s %s",
               tname1, &key1, &expected_status, name, address, email);

        testHospital.hospital_id = key1;
        strcpy(testHospital.name,    name);
        strcpy(testHospital.address, address);
        strcpy(testHospital.email,   email);

        status = update_table(key1, &testHospital, tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "update_table returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       DELETE TABLE RECORD
       Format: DELETE <tname> <key> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "DELETE")){
        sscanf(test_case, "%*s %s %d %d", tname1, &key1, &expected_status);
        status = delete_table(key1, tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "delete_table returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       UNDELETE TABLE RECORD
       Format: UNDELETE <tname> <key> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "UNDELETE")){
        sscanf(test_case, "%*s %s %d %d", tname1, &key1, &expected_status);
        status = undelete_table(key1, tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "undelete_table returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       CREATE RELATION
       Format: REL_CREATE <relname> <child> <parent> <expected_status>
       PDS call: create_relation(parent, child, relname)
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_CREATE")){
        sscanf(test_case, "%*s %s %s %s %d",
               tname1, tname2, tname3, &expected_status);
        status = create_relation(
            tname3,   /* parent table  */
            tname2,   /* child table   */
            tname1    /* relation name */
        );
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "create_relation returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       OPEN RELATION
       Format: REL_OPEN <relname> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_OPEN")){
        sscanf(test_case, "%*s %s %d", tname1, &expected_status);
        status = open_relation(tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "open_relation returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       STORE RELATION
       Format: REL_STORE <pkey> <fkey> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_STORE")){
        sscanf(test_case, "%*s %d %d %d", &key1, &key2, &expected_status);
        status = store_relation(key1, key2);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "store_relation returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       SEARCH RELATION (by parent key)
       Format: REL_SEARCH <pkey> <expected_status> [<name> <address> <email>]
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_SEARCH")){
        sscanf(test_case, "%*s %d %d", &key1, &expected_status);
        status = get_relation(&testHospital, key1);

        if(status != expected_status){
            sprintf(info, "get_relation returned %d", status);
            TREPORT("FAIL", info);
        }
        else if(status == SUCCESS){
            sscanf(test_case, "%*s %*d %*d %s %s %s", name, address, email);
            if(!strcmp(testHospital.name, name))
                TREPORT("PASS", "");
            else
                TREPORT("FAIL", "Relational data mismatch");
        }
        else TREPORT("PASS", "");
    }

    /* ----------------------------------------------------------------
       DELETE RELATION
       Format: REL_DELETE <tname> <key> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_DELETE")){
        sscanf(test_case, "%*s %s %d %d", tname1, &key1, &expected_status);
        status = delete_relation(key1, tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "delete_relation returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       UNDELETE RELATION
       Format: REL_UNDELETE <tname> <key> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_UNDELETE")){
        sscanf(test_case, "%*s %s %d %d", tname1, &key1, &expected_status);
        status = undelete_relation(key1, tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "undelete_relation returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       CLOSE RELATION (standalone)
       Format: REL_CLOSE <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_CLOSE")){
        sscanf(test_case, "%*s %d", &expected_status);
        status = close_relation();
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "close_relation returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       CLOSE DB (closes tables + relation)
       Format: CLOSE <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "CLOSE")){
        sscanf(test_case, "%*s %d", &expected_status);
        status = close_db();
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "close_db returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       UNKNOWN COMMAND
    ---------------------------------------------------------------- */
    else{
        sprintf(info, "unknown command '%s'", command);
        TREPORT("FAIL", info);
    }
}