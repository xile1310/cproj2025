
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>          
#define _stricmp strcasecmp   
#include "cms.h"


static void printDeclaration(void) {
    printf("Declaration\n");
    printf("SIT's policy on copying does not allow students to copy source code or assessment solutions from another person, AI, or other places.\n");
    printf("We hereby declare that we understand and agree to the policy and did not copy or share our code.\n");
    printf("We agree that our project will receive zero if plagiarism is detected.\n");
    printf("We agree that we did not copy any code directly from AI-generated sources.\n\n");
}

static void printHelp(void) {
    printf("\nCommands:\n");
    printf("  OPEN <filename>\n");
    printf("  SHOW ALL\n");
    printf("  SHOW ALL SORT BY ID ASC|DESC\n");
    printf("  SHOW ALL SORT BY MARK ASC|DESC\n");
    printf("  INSERT\n");
    printf("  QUERY ID=<id>\n");
    printf("  UPDATE ID=<id>\n");
    printf("  DELETE ID=<id>\n");
    printf("  SAVE\n");
    printf("  SHOW SUMMARY\n");
    printf("  GRADE ID=<id>\n");
    printf("  TOPPERCENT <percent>\n");
    printf("  EXPORT CSV <filename>\n");
    printf("  HELP\n");
    printf("  EXIT\n\n");
}

static int parseIdToken(const char* line, int* outId) {
    const char* p = strstr(line, "ID=");
    if (!p) p = strstr(line, "id=");
    if (!p) return 0;
    p += 3;
    while (*p == ' ') p++;
    int id = 0;
    if (sscanf(p, "%d", &id) == 1) {
        *outId = id;
        return 1;
    }
    return 0;
}

int main(void) {
    printDeclaration();
    printf("Welcome to CMS. Type HELP to see commands.\n\n");

    char line[512];
    for (;;) {
        printf("CMS> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        char cmd[64] = { 0 }, arg1[64] = { 0 }, arg2[64] = { 0 }, arg3[64] = { 0 };
        sscanf(line, "%63s %63s %63s %63s", cmd, arg1, arg2, arg3);
        for (char* p = cmd; *p; ++p) *p = (char)toupper((unsigned char)*p);

        if (strcmp(cmd, "OPEN") == 0) {
            char* fn = line + 4;
            while (*fn && isspace((unsigned char)*fn)) fn++;
            if (*fn == '\0') printf("CMS: Usage: OPEN <filename>\n");
            else cms_open(fn);
        }
        else if (strcmp(cmd, "SHOW") == 0) {
            if (_stricmp(arg1, "ALL") == 0) {
                if (_stricmp(arg2, "SORT") == 0) {
                    char field[16] = { 0 }, ord[16] = { 0 };
                    sscanf(line, "%*s %*s %*s %*s %15s %15s", field, ord);
                    cms_show_all_sorted(field, (strlen(ord) ? ord : "ASC"));
                }
                else cms_show_all();
            }
            else if (_stricmp(arg1, "SUMMARY") == 0) cms_summary();
            else printf("CMS: Unknown SHOW option.\n");
        }
        else if (strcmp(cmd, "INSERT") == 0) cms_insert();
        else if (strcmp(cmd, "QUERY") == 0) {
            int id; if (parseIdToken(line, &id)) cms_query(id);
            else printf("CMS: Usage: QUERY ID=<id>\n");
        }
        else if (strcmp(cmd, "UPDATE") == 0) {
            int id; if (parseIdToken(line, &id)) cms_update(id);
            else printf("CMS: Usage: UPDATE ID=<id>\n");
        }
        else if (strcmp(cmd, "DELETE") == 0) {
            int id; if (parseIdToken(line, &id)) cms_delete(id);
            else printf("CMS: Usage: DELETE ID=<id>\n");
        }
        else if (strcmp(cmd, "SAVE") == 0) cms_save();
        else if (strcmp(cmd, "EXPORT") == 0 && _stricmp(arg1, "CSV") == 0) {
            char* fn = strstr(line, "CSV"); if (fn) { fn += 3; while (*fn && isspace((unsigned char)*fn)) fn++; }
            if (!fn || *fn == '\0') printf("CMS: Usage: EXPORT CSV <filename>\n");
            else cms_export_csv(fn);
        }
        else if (strcmp(cmd, "GRADE") == 0) {
            int id; if (parseIdToken(line, &id)) cms_grade(id);
            else printf("CMS: Usage: GRADE ID=<id>\n");
        }
        else if (strcmp(cmd, "TOPPERCENT") == 0) {
            float percent; 
            if (sscanf(line, "%*s %f", &percent) == 1)
                cms_toppercent(percent);
            else 
                printf("CMS: Usage: TOPPERCENT <percent>\n");
        }
        else if (strcmp(cmd, "HELP") == 0) printHelp();
        else if (strcmp(cmd, "EXIT") == 0 || strcmp(cmd, "QUIT") == 0) break;
        else printf("CMS: Unknown command. Type HELP.\n");
    }

    printf("Goodbye.\n");
    return 0;

}
