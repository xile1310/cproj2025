#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cms.h"
#include "storage.h"

int stricmp(const char* a, const char* b) {
    while (*a && *b) {
        char ca = tolower((unsigned char)*a);
        char cb = tolower((unsigned char)*b);
        if (ca != cb)
            return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}
static void printHeader(void) {
    printf("%-10s %-30s %-24s %-6s\n", "ID", "Name", "Programme", "Mark");
    printf("%-10s %-30s %-24s %-6s\n", "----------", "------------------------------", "------------------------", "------");
}

static void printOne(const Student* s) {
    printf("%-10d %-30s %-24s %-6.2f\n", s->id, s->name, s->programme, s->mark);
}

static const char* gradeBand(float m) {
    if (m >= 85) return "A"; if (m >= 80) return "A-"; if (m >= 75) return "B+"; if (m >= 70) return "B";
    if (m >= 65) return "B-"; if (m >= 60) return "C+"; if (m >= 55) return "C"; if (m >= 50) return "D"; return "F";
}

void cms_open(const char* filename) { loadDatabase(filename); }
void cms_save(void) { saveDatabase(); }
void cms_export_csv(const char* filename) { exportCSV(filename); }

void cms_show_all(void) {
    if (dbCount == 0) { printf("CMS: No records in memory.\n"); return; }
    printHeader();
    for (int i = 0; i < dbCount; i++) printOne(&db[i]);
}

void cms_show_all_sorted(const char* field, const char* order) {
    if (dbCount == 0) { printf("CMS: No records in memory.\n"); return; }
    Student* tmp = malloc(sizeof(Student) * dbCount);
    if (!tmp) return;
    memcpy(tmp, db, sizeof(Student) * dbCount);

    int asc = (_stricmp(order, "DESC") != 0);
    if (_stricmp(field, "ID") == 0)
        qsort(tmp, dbCount, sizeof(Student), asc ? cmpIdAsc : cmpIdDesc);
    else if (_stricmp(field, "MARK") == 0)
        qsort(tmp, dbCount, sizeof(Student), asc ? cmpMarkAsc : cmpMarkDesc);

    printHeader();
    for (int i = 0; i < dbCount; i++) printOne(&tmp[i]);
    free(tmp);
}

void cms_insert(void) {
    if (dbCount >= MAX_STUDENTS) { printf("CMS: Database full.\n"); return; }
    Student s = { 0 };
    printf("Enter ID (integer): "); if (scanf("%d", &s.id) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');
    if (findIndexById(s.id) != -1) { printf("CMS: ID already exists.\n"); return; }
    printf("Enter Name: "); fgets(s.name, sizeof(s.name), stdin); s.name[strcspn(s.name, "\r\n")] = 0;
    printf("Enter Programme: "); fgets(s.programme, sizeof(s.programme), stdin); s.programme[strcspn(s.programme, "\r\n")] = 0;
    printf("Enter Mark (0-100): "); scanf("%f", &s.mark); while (getchar() != '\n');
    if (s.mark < 0 || s.mark > 100) { printf("CMS: Invalid mark.\n"); return; }
    db[dbCount++] = s;
    printf("CMS: Record inserted.\n");
}

void cms_query(int id) {
    int idx = findIndexById(id);
    if (idx == -1) { printf("CMS: ID=%d not found.\n", id); return; }
    printHeader();
    printOne(&db[idx]);
}

void cms_update(int id) {
    int idx = findIndexById(id);
    if (idx == -1) { printf("CMS: ID=%d not found.\n", id); return; }
    Student* s = &db[idx];
    char buf[128];
    printf("Current -> "); printOne(s);
    printf("New Name (blank to keep): "); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\r\n")] = 0;
    if (strlen(buf)) strncpy(s->name, buf, MAX_NAME);
    printf("New Programme (blank to keep): "); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\r\n")] = 0;
    if (strlen(buf)) strncpy(s->programme, buf, MAX_PROG);
    printf("New Mark (blank to keep): "); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\r\n")] = 0;
    if (strlen(buf)) {
        float m; if (sscanf(buf, "%f", &m) == 1 && m >= 0 && m <= 100) s->mark = m;
    }
    printf("CMS: Record updated.\n");
}

void cms_delete(int id) {
    int idx = findIndexById(id);
    if (idx == -1) { printf("CMS: ID=%d not found.\n", id); return; }
    printf("Confirm delete ID=%d (Y/N)? ", id);
    int c = getchar(); while (getchar() != '\n');
    if (c == 'Y' || c == 'y') {
        for (int i = idx; i < dbCount - 1; i++) db[i] = db[i + 1];
        dbCount--;
        printf("CMS: Record deleted.\n");
    }
    else printf("CMS: Deletion cancelled.\n");
}

void cms_summary(void) {
    if (dbCount == 0) { printf("CMS: No records.\n"); return; }
    int minI = 0, maxI = 0; double sum = 0.0;
    for (int i = 0; i < dbCount; i++) {
        sum += db[i].mark;
        if (db[i].mark < db[minI].mark) minI = i;
        if (db[i].mark > db[maxI].mark) maxI = i;
    }
    printf("Total: %d\nAverage: %.2f\nHighest: %.2f (%s)\nLowest: %.2f (%s)\n",
        dbCount, (float)(sum / dbCount), db[maxI].mark, db[maxI].name,
        db[minI].mark, db[minI].name);
}

void cms_grade(int id) {
    int idx = findIndexById(id);
    if (idx == -1) { printf("CMS: ID=%d not found.\n", id); return; }
    printf("CMS: ID=%d, Name=%s, Mark=%.2f, Grade=%s\n",
        db[idx].id, db[idx].name, db[idx].mark, gradeBand(db[idx].mark));

}


void cms_toppercent(float percent) {
    if (dbCount == 0) {
        printf("CMS: No records.\n");
        return;
    }
    if (percent <= 0 || percent > 100) {
        printf("CMS: Invalid percent value.\n");
        return;
    }

    printf("Choose mode:\n");
    printf("1. Whole database\n");
    printf("2. Specific programme\n");
    printf("Enter choice: ");

    int choice;
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        printf("CMS: Invalid input.\n");
        return;
    }
    while (getchar() != '\n');

	Student* list = NULL;
	int listcount = 0;
    int count = 0;
    if (choice == 2) {
        char programme[MAX_PROG];
        printf("Enter programme name: ");
        fgets(programme, sizeof(programme), stdin);
        programme[strcspn(programme, "\r\n")] = 0;

        for (int i = 0; i < dbCount; i++) {
            if (_stricmp(db[i].programme, programme) == 0)
                listcount++;
        }
        if (listcount == 0) {
            printf("CMS: No records found for programme '%s'.\n", programme);
            return;
        }
        list = malloc(sizeof(Student) * listcount);
        if (!list) {
            printf("CMS: No records in memory .\n");
            return;
        }

        int pos = 0;
        for (int i = 0; i < dbCount; i++) {
            if (_stricmp(db[i].programme, programme) == 0)
                list[pos++] = db[i];
        }

        int count = (int)(listcount * (percent / 100.0f));
        if (count < 1) count = 1;
        printf("Top %.2f%% (%d students):\n", percent, count);
        printHeader();
        for (int i = 0; i < count; i++) {
            printOne(&list[i]);
        }

    }
    else {
        listcount = dbCount;
        list = malloc(sizeof(Student) * dbCount);
        if (!list) {
            printf("CMS: No records in memory .\n");
            return;
        }
        memcpy(list, db, sizeof(Student) * dbCount);
        qsort(list, dbCount, sizeof(Student), cmpMarkDesc);
        count = (int)(dbCount * (percent / 100.0f));
        if (count < 1) count = 1;
        printf("Top %.2f%% (%d students):\n", percent, count);
        printHeader();
        for (int i = 0; i < count; i++)
            printOne(&list[i]);
        }
    free(list);
}

