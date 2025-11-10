// INF1002 Programming Fundamentals � Class Management System (CMS)
// Reference implementation for learning/comparison only. Do not submit verbatim.
// 
// Features implemented (maps to brief):
//  - OPEN <filename>                    : load database from a text file (tab-separated)
//  - SHOW ALL                           : list all records currently in memory
//  - SHOW ALL SORT BY ID ASC|DESC       : sorted by ID
//  - SHOW ALL SORT BY MARK ASC|DESC     : sorted by Mark
//  - INSERT                             : interactive prompts to add a record (duplicate-ID guarded)
//  - QUERY ID=<id>                      : show one record by student ID
//  - UPDATE ID=<id>                     : interactively update that record
//  - DELETE ID=<id>                     : confirm and delete
//  - SAVE                               : write all in-memory records back to the opened file
//  - SHOW SUMMARY                       : total, average, highest/lowest (with names)
//  - EXPORT CSV <filename>              : (unique feature) export to CSV
//  - GRADE ID=<id>                      : (unique feature) compute grade band for a student
//  - EXIT                               : quit program
//
// File format expected when reading (robust to header lines):
//   - Tab-separated columns with header line:  ID\tName\tProgramme\tMark
//   - Name/Programme may contain spaces, but columns must be separated by TABs (\t).
//   - Any leading lines (e.g., "Database Name:", "Table Name:") are safely skipped.
//   - Example lines:
//       2301234\tJoshua Chen\tSoftware Engineering\t70.5
//
// Build (Windows/MSVC):   cl /W3 /EHsc cms.c
// Build (gcc/clang):      gcc -std=c11 -Wall -Wextra -O2 -o cms cms.c
// Run:                    ./cms
//
// Note: This code is intentionally straightforward (single-file, arrays). In your report, 
//       explain data-structure choices, testing, and team contributions per the brief.

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _MSC_VER
#include <string.h>
#define strcasecmp   _stricmp
#define strncasecmp  _strnicmp
#endif


#define MAX_STUDENTS 2000
#define MAX_NAME 96
#define MAX_PROG 96

typedef struct {
    int   id;
    char  name[MAX_NAME];
    char  programme[MAX_PROG];
    float mark;
} Student;

static Student db[MAX_STUDENTS];
static int dbCount = 0;
static char openedFile[256] = "";  // current database filename

// ----------------------------- Utility helpers -----------------------------
static char* lstrip(char* s) { while (*s && isspace((unsigned char)*s)) s++; return s; }
static void  rstrip(char* s) { size_t n = strlen(s); while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0'; }
static void  trim(char* s) { rstrip(s); char* p = lstrip(s); if (p != s) memmove(s, p, strlen(p) + 1); }

static int cmpIdAsc(const void* a, const void* b) { const Student* x = a, * y = b; return (x->id - y->id); }
static int cmpIdDesc(const void* a, const void* b) { const Student* x = a, * y = b; return (y->id - x->id); }
static int cmpMarkAsc(const void* a, const void* b) { const Student* x = a, * y = b; return (x->mark > y->mark) - (x->mark < y->mark); }
static int cmpMarkDesc(const void* a, const void* b) { const Student* x = a, * y = b; return (y->mark > x->mark) - (y->mark < x->mark); }

static int findIndexById(int id) {
    for (int i = 0;i < dbCount;i++) if (db[i].id == id) return i;
    return -1;
}

static void printOne(const Student* s) {
    printf("%-10d %-30s %-24s %-6.2f\n", s->id, s->name, s->programme, s->mark);
}

static void printHeader(void) {
    printf("%-10s %-30s %-24s %-6s\n", "ID", "Name", "Programme", "Mark");
    printf("%-10s %-30s %-24s %-6s\n", "----------", "------------------------------", "------------------------", "------");
}

// ----------------------------- File I/O -----------------------------
// Expect lines: ID\tName\tProgramme\tMark  (skip non-data lines)
static int parseDataLine(const char* line, Student* out) {
    // duplicate line to tokenize safely
    char buf[512]; strncpy(buf, line, sizeof(buf)); buf[sizeof(buf) - 1] = '\0';

    // quick reject: if line doesn't start with digit, skip
    const char* p = lstrip(buf);
    if (!isdigit((unsigned char)*p)) return 0;

    // split by tabs exactly 4 columns
    char* tok1 = strtok(buf, "\t");
    char* tok2 = strtok(NULL, "\t");
    char* tok3 = strtok(NULL, "\t");
    char* tok4 = strtok(NULL, "\t\r\n");
    if (!tok1 || !tok2 || !tok3 || !tok4) return 0;

    trim(tok1); trim(tok2); trim(tok3); trim(tok4);

    // id
    char* endp = NULL; long lid = strtol(tok1, &endp, 10);
    if (endp == tok1 || *endp != '\0') return 0;

    // mark
    char* endp2 = NULL; float m = strtof(tok4, &endp2);
    if (endp2 == tok4) return 0;

    out->id = (int)lid;
    strncpy(out->name, tok2, MAX_NAME - 1); out->name[MAX_NAME - 1] = '\0';
    strncpy(out->programme, tok3, MAX_PROG - 1); out->programme[MAX_PROG - 1] = '\0';
    out->mark = m;
    return 1;
}

static int loadDatabase(void) {
    FILE* f = fopen("Databasefile.txt", "r");
    if (!f) { printf("CMS: Failed to open 'Databasefile.txt' for reading.\n"); return 0; }
    
    printf("\nRecords in Databasefile.txt:\n");
    printHeader();
    
    char line[512];
    int recordCount = 0;
    while (fgets(line, sizeof(line), f)) {
        Student s;
        if (parseDataLine(line, &s)) {
            printOne(&s);
            recordCount++;
        }
    }
    fclose(f);
    if (recordCount == 0) {
        printf("No records found in database.\n");
    }
    printf("\nTotal records displayed: %d\n", recordCount);
    return 1;
}

static int saveDatabase(void) {
    if (openedFile[0] == '\0') { printf("CMS: No file opened. Use OPEN <filename> first.\n"); return 0; }
    FILE* f = fopen(openedFile, "w");
    if (!f) { printf("CMS: Failed to open '%s' for writing.\n", openedFile); return 0; }
    fprintf(f, "Table Name: StudentRecords\n");
    fprintf(f, "ID\tName\tProgramme\tMark\n");
    for (int i = 0;i < dbCount;i++) {
        fprintf(f, "%d\t%s\t%s\t%.2f\n", db[i].id, db[i].name, db[i].programme, db[i].mark);
    }
    fclose(f);
    printf("CMS: The database file '%s' is successfully saved.\n", openedFile);
    return 1;
}

static int exportCSV(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) { printf("CMS: Failed to open '%s' for writing.\n", filename); return 0; }
    fprintf(f, "ID,Name,Programme,Mark\n");
    for (int i = 0;i < dbCount;i++) {
        // naive CSV (names/programmes assumed no commas). For full CSV, escape commas/quotes.
        fprintf(f, "%d,%s,%s,%.2f\n", db[i].id, db[i].name, db[i].programme, db[i].mark);
    }
    fclose(f);
    printf("CMS: Exported %d records to '%s'\n", dbCount, filename);
    return 1;
}

// ----------------------------- Commands -----------------------------
static void cmdShowAllSorted(const char* field, const char* order) {
    if (dbCount == 0) { printf("CMS: No records in memory.\n"); return; }
    Student* tmp = malloc(sizeof(Student) * dbCount);
    if (!tmp) { printf("CMS: Memory error.\n"); return; }
    memcpy(tmp, db, sizeof(Student) * dbCount);

    int byId = 0, byMark = 0; int asc = 1;
    if (field && strcasecmp(field, "ID") == 0) byId = 1; else if (field && strcasecmp(field, "MARK") == 0) byMark = 1;
    if (order && strcasecmp(order, "DESC") == 0) asc = 0; else asc = 1;

    if (byId) { qsort(tmp, dbCount, sizeof(Student), asc ? cmpIdAsc : cmpIdDesc); }
    else if (byMark) { qsort(tmp, dbCount, sizeof(Student), asc ? cmpMarkAsc : cmpMarkDesc); }

    printHeader();
    for (int i = 0;i < dbCount;i++) printOne(&tmp[i]);
    free(tmp);
}

static void cmdShowAll(void) {
    if (dbCount == 0) { printf("CMS: No records in memory.\n"); return; }
    printHeader();
    for (int i = 0;i < dbCount;i++) printOne(&db[i]);
}

static void cmdInsert(void) {
    if (dbCount >= MAX_STUDENTS) { printf("CMS: Database full.\n"); return; }
    Student s; memset(&s, 0, sizeof(s));
    printf("Enter ID (integer): "); if (scanf("%d", &s.id) != 1) { printf("CMS: Invalid ID.\n"); while (getchar() != '\n'); return; }
    while (getchar() != '\n');
    if (findIndexById(s.id) != -1) { printf("CMS: The record with ID=%d already exists.\n", s.id); return; }
    printf("Enter Name: "); fgets(s.name, sizeof(s.name), stdin); rstrip(s.name);
    printf("Enter Programme: "); fgets(s.programme, sizeof(s.programme), stdin); rstrip(s.programme);
    printf("Enter Mark (0-100): "); if (scanf("%f", &s.mark) != 1) { printf("CMS: Invalid mark.\n"); while (getchar() != '\n'); return; }
    while (getchar() != '\n');
    if (s.mark < 0 || s.mark>100) { printf("CMS: Mark out of range.\n"); return; }
    db[dbCount++] = s;
    printf("CMS: A new record with ID=%d is successfully inserted.\n", s.id);
}

static void cmdQuery(int id) {
    int idx = findIndexById(id);
    if (idx == -1) { printf("CMS: The record with ID=%d does not exist.\n", id); return; }
    printHeader();
    printOne(&db[idx]);
}

static void cmdUpdate(int id) {
    int idx = findIndexById(id);
    if (idx == -1) { printf("CMS: The record with ID=%d does not exist.\n", id); return; }
    Student* s = &db[idx];
    char buf[128];
    printf("Current -> "); printOne(s);
    printf("New Name (leave blank to keep): "); fgets(buf, sizeof(buf), stdin); rstrip(buf); if (strlen(buf) > 0) strncpy(s->name, buf, MAX_NAME - 1);
    printf("New Programme (leave blank to keep): "); fgets(buf, sizeof(buf), stdin); rstrip(buf); if (strlen(buf) > 0) strncpy(s->programme, buf, MAX_PROG - 1);
    printf("New Mark (leave blank to keep): "); fgets(buf, sizeof(buf), stdin); rstrip(buf);
    if (strlen(buf) > 0) { float m; if (sscanf(buf, "%f", &m) == 1 && m >= 0 && m <= 100) s->mark = m; else { printf("CMS: Invalid mark. No change.\n"); } }
    printf("CMS: The record with ID=%d is successfully updated.\n", id);
}

static void cmdDelete(int id) {
    int idx = findIndexById(id);
    if (idx == -1) { printf("CMS: The record with ID=%d does not exist.\n", id); return; }
    printf("CMS: Are you sure you want to delete record with ID=%d? Type 'Y' to confirm or 'N' to cancel.\n", id);
    int c = getchar(); while (getchar() != '\n');
    if (c == 'Y' || c == 'y') {
        for (int i = idx;i < dbCount - 1;i++) db[i] = db[i + 1];
        dbCount--;
        printf("CMS: The record with ID=%d is successfully deleted.\n", id);
    }
    else {
        printf("CMS: The deletion is cancelled.\n");
    }
}

static const char* gradeBand(float m) {
    if (m >= 85) return "A"; if (m >= 80) return "A-"; if (m >= 75) return "B+"; if (m >= 70) return "B"; if (m >= 65) return "B-";
    if (m >= 60) return "C+"; if (m >= 55) return "C"; if (m >= 50) return "D"; return "F";
}

static void cmdGrade(int id) {
    int idx = findIndexById(id);
    if (idx == -1) { printf("CMS: The record with ID=%d does not exist.\n", id); return; }
    printf("CMS: ID=%d, Name=%s, Mark=%.2f, Grade=%s\n", db[idx].id, db[idx].name, db[idx].mark, gradeBand(db[idx].mark));
}

static void cmdSummary(void) {
    if (dbCount == 0) { printf("CMS: No records in memory.\n"); return; }
    int minI = 0, maxI = 0; double sum = 0.0;
    for (int i = 0;i < dbCount;i++) {
        sum += db[i].mark;
        if (db[i].mark < db[minI].mark) minI = i;
        if (db[i].mark > db[maxI].mark) maxI = i;
    }
    printf("Total number of students: %d\n", dbCount);
    printf("Average mark: %.2f\n", (float)(sum / dbCount));
    printf("Highest mark: %.2f (%s)\n", db[maxI].mark, db[maxI].name);
    printf("Lowest mark : %.2f (%s)\n", db[minI].mark, db[minI].name);
}

// ----------------------------- Parser helpers -----------------------------
static int parseIdToken(const char* line, int* outId) {
    // looks for substring like ID=123456
    const char* p = strstr(line, "ID=");
    if (!p) p = strstr(line, "id=");
    if (!p) return 0;
    p += 3; // skip ID=
    while (*p == ' ') p++;
    int id = 0; if (sscanf(p, "%d", &id) == 1) { *outId = id; return 1; }
    return 0;
}

static void printHelp(void) {
    printf("\nCommands:\n");
    printf("  OPEN                                 : display records from Databasefile.txt\n");
    printf("  SHOW ALL\n");
    printf("  SHOW ALL SORT BY ID ASC|DESC\n");
    printf("  SHOW ALL SORT BY MARK ASC|DESC\n");
    printf("  INSERT\n");
    printf("  QUERY ID=<id>\n");
    printf("  UPDATE ID=<id>\n");
    printf("  DELETE ID=<id>\n");
    printf("  SHOW SUMMARY\n");
    printf("  EXPORT CSV <filename>\n");
    printf("  GRADE ID=<id>\n");
    printf("  SAVE\n");
    printf("  HELP\n");
    printf("  EXIT\n\n");
}

static void printDeclaration(void) {
    printf("Declaration\n");
    printf("SIT�s policy on copying does not allow students to copy source code or assessment solutions from another person, AI, or other places.\n");
    printf("We hereby declare that we understand and agree to the policy and did not copy or share our code.\n");
    printf("We agree that our project will receive zero if plagiarism is detected.\n");
    printf("We agree that we did not copy any code directly from AI-generated sources.\n\n");
}

int main(void) {
    printDeclaration();
    printf("Welcome to CMS. Type HELP to see commands.\n\n");

    char line[512];
    for (;;) {
        printf("CMS> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        rstrip(line);
        if (strlen(line) == 0) continue;

        // Tokenize the first few words (case-insensitive)
        char cmd[64] = { 0 }, arg1[64] = { 0 }, arg2[64] = { 0 }, arg3[64] = { 0 };
        sscanf(line, "%63s %63s %63s %63s", cmd, arg1, arg2, arg3);
        for (char* p = cmd; *p; ++p) *p = (char)toupper((unsigned char)*p);

        if (strcmp(cmd, "OPEN") == 0) {
            loadDatabase();
        }
        else if (strcmp(cmd, "SHOW") == 0) {
            if (strncasecmp(arg1, "ALL", 3) == 0) {
                if (strcasecmp(arg2, "SORT") == 0 && strcasecmp(arg3, "BY") == 0) {
                    // Expect: SHOW ALL SORT BY <ID|MARK> [ASC|DESC]
                    char field[16] = { 0 }, ord[16] = { 0 };
                    // re-scan with 6 tokens to get field and order
                    char w1[16], w2[16], w3[16], w4[16], w5[16], w6[16];
                    int n = sscanf(line, "%15s %15s %15s %15s %15s %15s", w1, w2, w3, w4, field, ord);
                    if (n >= 5) cmdShowAllSorted(field, (n >= 6 ? ord : "ASC"));
                    else { printf("CMS: Usage: SHOW ALL SORT BY <ID|MARK> [ASC|DESC]\n"); }
                }
                else {
                    cmdShowAll();
                }
            }
            else if (strcasecmp(arg1, "SUMMARY") == 0) {
                cmdSummary();
            }
            else {
                printf("CMS: Unknown SHOW option.\n");
            }
        }
        else if (strcmp(cmd, "INSERT") == 0) {
            cmdInsert();
        }
        else if (strcmp(cmd, "QUERY") == 0) {
            int id; if (parseIdToken(line, &id)) cmdQuery(id); else printf("CMS: Usage: QUERY ID=<id>\n");
        }
        else if (strcmp(cmd, "UPDATE") == 0) {
            int id; if (parseIdToken(line, &id)) cmdUpdate(id); else printf("CMS: Usage: UPDATE ID=<id>\n");
        }
        else if (strcmp(cmd, "DELETE") == 0) {
            int id; if (parseIdToken(line, &id)) cmdDelete(id); else printf("CMS: Usage: DELETE ID=<id>\n");
        }
        else if (strcmp(cmd, "SAVE") == 0) {
            saveDatabase();
        }
        else if (strcmp(cmd, "EXPORT") == 0) {
            if (strcasecmp(arg1, "CSV") == 0) {
                char* fn = strstr(line, "CSV"); if (fn) { fn += 3; fn = (char*)lstrip(fn); }
                if (!fn || *fn == '\0') { printf("CMS: Usage: EXPORT CSV <filename>\n"); }
                else exportCSV(fn);
            }
            else {
                printf("CMS: Usage: EXPORT CSV <filename>\n");
            }
        }
        else if (strcmp(cmd, "GRADE") == 0) {
            int id; if (parseIdToken(line, &id)) cmdGrade(id); else printf("CMS: Usage: GRADE ID=<id>\n");
        }
        else if (strcmp(cmd, "HELP") == 0) {
            printHelp();
        }
        else if (strcmp(cmd, "EXIT") == 0 || strcmp(cmd, "QUIT") == 0) {
            break;
        }
        else {
            printf("CMS: Unknown command. Type HELP.\n");
        }
    }
    printf("Goodbye.\n");
    return 0;
}
