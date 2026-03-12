#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pds4.h"

typedef struct Student {
    int  student_id;
    char name[100];
    char dept[100];
    char email[50];
} Student;

typedef struct Course {
    int  course_id;
    char title[100];
    char dept[100];
    char instructor[50];
} Course;

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
    char    name[100], dept[100], email[50];
    int     key1, key2, expected_status, status;
    Student testStudent;
    Course  testCourse;

    sscanf(test_case, "%s", command);
    printf("Test case: %s", test_case); fflush(stdout);

    /* ----------------------------------------------------------------
       INIT
       Format: INIT <num_tables> <num_relations> <expected_status>
    ---------------------------------------------------------------- */
    if(!strcmp(command, "INIT")){
        int num_tables, num_relations;
        sscanf(test_case, "%*s %d %d %d", &num_tables, &num_relations, &expected_status);
        init(num_tables, num_relations);
        if(expected_status == SUCCESS) TREPORT("PASS", "");
        else TREPORT("FAIL", "init failed to execute");
    }

    /* ----------------------------------------------------------------
       CREATE DB
       Format: CREATE <dbname> <tname1> <tname2> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "CREATE")){
        char dbname[50];
        sscanf(test_case, "%*s %s %s %s %d", dbname, tname1, tname2, &expected_status);
        char* tnames[] = {tname1, tname2};
        status = create_db(2, tnames);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "create_db returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       OPEN DB
       Format: OPEN <dbname> <tname1> <tname2> <relname> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "OPEN")){
        char dbname[50], relname[50];
        sscanf(test_case, "%*s %s %s %s %s %d", dbname, tname1, tname2, relname, &expected_status);
        char* tnames[]   = {tname1, tname2};
        int   rsizes[]   = {sizeof(Student), sizeof(Course)};
        char* rnames[]   = {relname};
        status = open_db(dbname, 2, tnames, rsizes, 1, rnames);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "open_db returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       STORE STUDENT
       Format: STORE_S <tname> <key> <expected_status> <name> <dept> <email>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "STORE_S")){
        sscanf(test_case, "%*s %s %d %d %s %s %s",
               tname1, &key1, &expected_status, name, dept, email);
        testStudent.student_id = key1;
        strcpy(testStudent.name,  name);
        strcpy(testStudent.dept,  dept);
        strcpy(testStudent.email, email);
        status = store_table(key1, &testStudent, tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "store_table returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       STORE COURSE
       Format: STORE_C <tname> <key> <expected_status> <title> <dept> <instructor>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "STORE_C")){
        sscanf(test_case, "%*s %s %d %d %s %s %s",
               tname1, &key1, &expected_status, name, dept, email);
        testCourse.course_id = key1;
        strcpy(testCourse.title,      name);
        strcpy(testCourse.dept,       dept);
        strcpy(testCourse.instructor, email);
        status = store_table(key1, &testCourse, tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "store_table returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       SEARCH STUDENT
       Format: SEARCH_S <tname> <key> <expected_status> [<name> <dept> <email>]
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "SEARCH_S")){
        sscanf(test_case, "%*s %s %d %d", tname1, &key1, &expected_status);
        status = get_table(key1, &testStudent, tname1);
        if(status != expected_status){
            sprintf(info, "get_table returned %d", status);
            TREPORT("FAIL", info);
        }
        else if(status == SUCCESS){
            sscanf(test_case, "%*s %*s %*d %*d %s %s %s", name, dept, email);
            if(testStudent.student_id == key1 && !strcmp(testStudent.name, name))
                TREPORT("PASS", "");
            else
                TREPORT("FAIL", "Data mismatch");
        }
        else TREPORT("PASS", "");
    }

    /* ----------------------------------------------------------------
       SEARCH COURSE
       Format: SEARCH_C <tname> <key> <expected_status> [<title> <dept> <instructor>]
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "SEARCH_C")){
        sscanf(test_case, "%*s %s %d %d", tname1, &key1, &expected_status);
        status = get_table(key1, &testCourse, tname1);
        if(status != expected_status){
            sprintf(info, "get_table returned %d", status);
            TREPORT("FAIL", info);
        }
        else if(status == SUCCESS){
            sscanf(test_case, "%*s %*s %*d %*d %s %s %s", name, dept, email);
            if(testCourse.course_id == key1 && !strcmp(testCourse.title, name))
                TREPORT("PASS", "");
            else
                TREPORT("FAIL", "Data mismatch");
        }
        else TREPORT("PASS", "");
    }

    /* ----------------------------------------------------------------
       SEARCH BY FIELD (non-key)
       Format: SEARCH_FIELD <tname> <field_offset> <field_size> <field_val> <expected_status> [<name>]
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "SEARCH_FIELD")){
        int field_offset, field_size;
        char field_val[100];
        sscanf(test_case, "%*s %s %d %d %s %d",
               tname1, &field_offset, &field_size, field_val, &expected_status);
        status = get_table_by_field(&testStudent, field_val, field_offset, field_size, tname1);
        if(status != expected_status){
            sprintf(info, "get_table_by_field returned %d", status);
            TREPORT("FAIL", info);
        }
        else if(status == SUCCESS){
            sscanf(test_case, "%*s %*s %*d %*d %*s %*d %s", name);
            if(!strcmp(testStudent.name, name))
                TREPORT("PASS", "");
            else
                TREPORT("FAIL", "Field search data mismatch");
        }
        else TREPORT("PASS", "");
    }

    /* ----------------------------------------------------------------
       UPDATE STUDENT
       Format: UPDATE_S <tname> <key> <expected_status> <name> <dept> <email>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "UPDATE_S")){
        sscanf(test_case, "%*s %s %d %d %s %s %s",
               tname1, &key1, &expected_status, name, dept, email);
        testStudent.student_id = key1;
        strcpy(testStudent.name,  name);
        strcpy(testStudent.dept,  dept);
        strcpy(testStudent.email, email);
        status = update_table(key1, &testStudent, tname1);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "update_table returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       DELETE
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
       UNDELETE
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
       Format: REL_CREATE <relname> <ptname> <rtname> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_CREATE")){
        sscanf(test_case, "%*s %s %s %s %d", tname1, tname2, tname3, &expected_status);
        status = create_relation(tname2, tname3, tname1);
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
       Format: REL_STORE <relname> <pkey> <fkey> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_STORE")){
        sscanf(test_case, "%*s %s %d %d %d", tname1, &key1, &key2, &expected_status);
        status = store_relation(tname1, key1, key2);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "store_relation returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       SEARCH RELATION
       Format: REL_SEARCH <relname> <pkey> <expected_status> [<title>]
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "REL_SEARCH")){
        sscanf(test_case, "%*s %s %d %d", tname1, &key1, &expected_status);
        status = get_relation(&testCourse, key1, tname1);
        if(status != expected_status){
            sprintf(info, "get_relation returned %d", status);
            TREPORT("FAIL", info);
        }
        else if(status == SUCCESS){
            sscanf(test_case, "%*s %*s %*d %*d %s", name);
            if(!strcmp(testCourse.title, name))
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
       CLOSE DB
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
       SAVE SCHEMA
       Format: SAVE <dbname> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "SAVE")){
        char dbname[50];
        sscanf(test_case, "%*s %s %d", dbname, &expected_status);
        status = save_schema(dbname);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "save_schema returned %d", status);
            TREPORT("FAIL", info);
        }
    }

    /* ----------------------------------------------------------------
       LOAD SCHEMA
       Format: LOAD <dbname> <expected_status>
    ---------------------------------------------------------------- */
    else if(!strcmp(command, "LOAD")){
        char dbname[50];
        sscanf(test_case, "%*s %s %d", dbname, &expected_status);
        status = load_schema(dbname);
        if(status == expected_status) TREPORT("PASS", "");
        else{
            sprintf(info, "load_schema returned %d", status);
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