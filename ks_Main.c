// main.c - For managing console user interaction
//        - Collects and filters user console input command and data
//        - Returns corresponding responses back to console terminal

#include <stdio.h>
#include <string.h>
#include "cms.h"   // Defined in ks_cms.h

int main(void) {
    char line[512];
    char cmd[64], arg1[64], arg2[64], arg3[64];

    cms_printDeclaration();
    printf("Welcome to CMS. Type HELP to see commands.\n\n");

    for (;;) {
        printf("CMS> ");
        if (!fgets(line, sizeof(line), stdin)) {
            break;  // EOF
        }

        // trim newline
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;

        // parse first few words
        cmd[0] = arg1[0] = arg2[0] = arg3[0] = '\0';
        sscanf(line, "%63s %63s %63s %63s", cmd, arg1, arg2, arg3);

        // normalise to upper case for command comparison
        for (char* p = cmd; *p; ++p) *p = (char)toupper(*p);

        if (strcmp(cmd, "OPEN") == 0) {
            cms_openDatabase(line + 4);            // pass the rest of the line
        }
        else if (strcmp(cmd, "SHOW") == 0) {
            cms_handleShowCommand(line);           // SHOW ALL, SHOW SUMMARY, SORT, etc.
        }
        else if (strcmp(cmd, "INSERT") == 0) {
            cms_insertRecord();
        }
        else if (strcmp(cmd, "QUERY") == 0) {
            cms_queryCommand(line);                // QUERY ID=...
        }
        else if (strcmp(cmd, "UPDATE") == 0) {
            cms_updateCommand(line);               // UPDATE ID=...
        }
        else if (strcmp(cmd, "DELETE") == 0) {
            cms_deleteCommand(line);               // DELETE ID=...
        }
        else if (strcmp(cmd, "EXPORT") == 0) {
            cms_exportCommand(line);               // EXPORT CSV ...
        }
        else if (strcmp(cmd, "GRADE") == 0) {
            cms_gradeCommand(line);                // GRADE ID=...
        }
        else if (strcmp(cmd, "SAVE") == 0) {
            cms_saveDatabase();
        }
        else if (strcmp(cmd, "HELP") == 0) {
            cms_printHelp();
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
