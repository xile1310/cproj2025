#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "storage.h"

Student db[MAX_STUDENTS];
int     dbCount = 0;
static char openedFile[256] = "";

// small string helpers

static char *lstrip(char *s) {
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    return s;
}

static void rstrip(char *s) {
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

static void trim(char *s) {
    char *start = lstrip(s);
    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }
    rstrip(s);
}

// comparators for qsort

int cmpIdAsc(const void *a, const void *b) {
    const Student *x = (const Student *)a;
    const Student *y = (const Student *)b;
    return x->id - y->id;
}

// Descending
int cmpIdDesc(const void *a, const void *b) {
    const Student *x = (const Student *)a;
    const Student *y = (const Student *)b;
    return y->id - x->id;
}

// Ascending
int cmpMarkAsc(const void *a, const void *b) {
    const Student *x = (const Student *)a;
    const Student *y = (const Student *)b;

    if (x->mark < y->mark) return -1;
    if (x->mark > y->mark) return 1;
    return 0;
}

int cmpMarkDesc(const void *a, const void *b) {
    const Student *x = (const Student *)a;
    const Student *y = (const Student *)b;

    if (x->mark > y->mark) return -1;
    if (x->mark < y->mark) return 1;
    return 0;
}

// lookup helper 
int findIndexById(int id) {
    for (int i = 0; i < dbCount; i++) {
        if (db[i].id == id) return i;
    }
    return -1;
}

// parsing a data line
static int parseDataLine(const char *line, Student *out) {
    char buf[512];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *p = lstrip(buf);
    if (!isdigit((unsigned char)*p)) {
        return 0;  
    }

//  Expect: ID \t Name \t Programme \t Mark 
    char *tok1 = strtok(buf, "\t");
    char *tok2 = strtok(NULL, "\t");
    char *tok3 = strtok(NULL, "\t");
    char *tok4 = strtok(NULL, "\t\r\n");

    if (!tok1 || !tok2 || !tok3 || !tok4) return 0;

    trim(tok1); trim(tok2); trim(tok3); trim(tok4);

    out->id = atoi(tok1);

    strncpy(out->name, tok2, MAX_NAME - 1);
    out->name[MAX_NAME - 1] = '\0';

    strncpy(out->programme, tok3, MAX_PROG - 1);
    out->programme[MAX_PROG - 1] = '\0';

    out->mark = (float)atof(tok4);
    return 1;
}

// load / save / export 
int loadDatabase(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("CMS: Cannot open '%s'.\n", filename);
        return 0;
    }

    dbCount = 0;
    char line[512];

    while (fgets(line, sizeof(line), f)) {
        Student s;
        if (parseDataLine(line, &s)) {
            if (dbCount < MAX_STUDENTS) {
                db[dbCount++] = s;
            }
        }
    }

    fclose(f);
    strncpy(openedFile, filename, sizeof(openedFile) - 1);
    openedFile[sizeof(openedFile) - 1] = '\0';

    printf("CMS: File '%s' opened. Records loaded: %d\n", openedFile, dbCount);
    return 1;
}

int saveDatabase(void) {
    if (openedFile[0] == '\0') {   
        printf("CMS: No file opened.\n");
        return 0;
    }

    FILE *f = fopen(openedFile, "w");
    if (!f) {
        printf("CMS: Cannot write '%s'.\n", openedFile);
        return 0;
    }

    // Simple header + data rows 
    fprintf(f, "Table Name: StudentRecords\n");
    fprintf(f, "ID\tName\tProgramme\tMark\n");
    for (int i = 0; i < dbCount; i++) {
        fprintf(f, "%d\t%s\t%s\t%.2f\n",
                db[i].id, db[i].name, db[i].programme, db[i].mark);
    }

    fclose(f);
    printf("CMS: Saved %d records to '%s'.\n", dbCount, openedFile);
    return 1;
}

int exportCSV(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("CMS: Cannot write '%s'.\n", filename);
        return 0;
    }

    fprintf(f, "ID,Name,Programme,Mark\n");
    for (int i = 0; i < dbCount; i++) {

// assumes no commas in name/programme 
        fprintf(f, "%d,%s,%s,%.2f\n",
                db[i].id, db[i].name, db[i].programme, db[i].mark);
    }

    fclose(f);
    printf("CMS: Exported %d records to '%s'.\n", dbCount, filename);
    return 1;
}
