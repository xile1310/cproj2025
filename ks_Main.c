// main.c - For managing console user interaction
//        - Collects and filters user console input command and data
//        - Returns corresponding responses back to console terminal

#include <stdio.h>
#include <string.h>
#include "cms.h"   // Defined in ks_cms.h (Declaration of CMS functions)
#include <ctype.h> // toupper (converts lowercase letters to uppercase)

int main(void) {
    // A buffer to store entire line the user types
    // A character array with space for 512 character is typically enough for console commands
    char line[512];

    // buffers to store first few words of command
    // cmd= main command (OPEN , SHOW, INSERT, etc.)
    // arg kept just incase, though not heavily used
    char cmd[64], arg1[64], arg2[64], arg3[64];

    // print system declaration
    cms_printDeclaration();
    printf("Welcome to CMS. Type HELP to see commands.\n\n");

    // main input loop which keeps running until user EXIT/QUIT
    for (;;) {
        printf("CMS> "); //prompt

        // read one line of input from user
        // fgets() reads at most sizeof(inputLine)-1 characters and automatically adds a '\0' terminator.
        if (!fgets(line, sizeof(line), stdin)) {
            break;  // EOF
        }

        // trim newline
        line[strcspn(line, "\r\n")] = '\0';

        // if the line is now empty, user just press enter to skip this loop
        if (line[0] == '\0') continue;

        // parse first few words
        cmd[0] = arg1[0] = arg2[0] = arg3[0] = '\0';
        sscanf(line, "%63s %63s %63s %63s", cmd, arg1, arg2, arg3);

        // normalise to upper case for command comparison
        // so user can type in any case and it will still work
        // example like: "open", "Open", "OPEN" all will work
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

    // program ending (user typed either EXIT or QUIT or EOF)
    printf("Goodbye.\n");
    return 0;
}
