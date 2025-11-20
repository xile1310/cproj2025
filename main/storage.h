#ifndef STORAGE_H
#define STORAGE_H

#define MAX_STUDENTS 1000
#define MAX_NAME     64
#define MAX_PROG     64

typedef struct {
    int   id;
    char  name[MAX_NAME];
    char  programme[MAX_PROG];
    float mark;
} Student;

/* Global database in RAM */
extern Student db[MAX_STUDENTS];
extern int     dbCount;

/* Sorting comparators for qsort */
int cmpIdAsc   (const void *a, const void *b);
int cmpIdDesc  (const void *a, const void *b);
int cmpMarkAsc (const void *a, const void *b);
int cmpMarkDesc(const void *a, const void *b);

/* Storage / file functions */
int  loadDatabase (const char *filename);
int  saveDatabase (void);
int  exportCSV    (const char *filename);
int  findIndexById(int id);

#endif
