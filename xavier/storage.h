#ifndef STORAGE_H
#define STORAGE_H

#define MAX_STUDENTS 2000
#define MAX_NAME 96
#define MAX_PROG 96

typedef struct {
    int id;
    char name[MAX_NAME];
    char programme[MAX_PROG];
    float mark;
} Student;

extern Student db[];
extern int dbCount;
extern char openedFile[];

int loadDatabase(const char* filename);
int saveDatabase(void);
int exportCSV(const char* filename);
int findIndexById(int id);

int cmpIdAsc(const void* a, const void* b);
int cmpIdDesc(const void* a, const void* b);
int cmpMarkAsc(const void* a, const void* b);
int cmpMarkDesc(const void* a, const void* b);

#endif
