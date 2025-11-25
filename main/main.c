// main.c - For managing console user interaction
//        - Collects and filters user console input command and data
//        - Returns corresponding responses back to console terminal

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h> // toupper (converts lowercase letters to uppercase)
#ifndef _WIN32
#include <strings.h> // for linux/mac user only because windows user is able to use _stricmp from string.h      
#define _stricmp strcasecmp   // make _stricmp work like strcasecmp
#endif
#include "cms.h" //cms function declarations

// print academic integrity declaration
static void printDeclaration(void) {
    printf("Declaration\n");
    printf("SIT's policy on copying does not allow students to copy source code or assessment solutions from another person, AI, or other places.\n");
    printf("We hereby declare that we understand and agree to the policy and did not copy or share our code.\n");
    printf("We agree that our project will receive zero if plagiarism is detected.\n");
    printf("We agree that we did not copy any code directly from AI-generated sources.\n\n");
}

// print all supported console commands
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
// look for an "ID=<number>" token (not case sensitive ID/id)
// if found and is valid, store the number ID in outId and return 1
// if not found/invalid, return 0 so caller can show user message
static int parseIdToken(const char* inputline, int* outId) {
    const char* p = strstr(inputline, "ID="); //p= cursor
    if (!p) p = strstr(inputline, "id=");
    if (!p) return 0;
    p += 3; //skip past "ID="
    while (*p == ' ') p++; //skip any spaces after "="
    int id = 0;
    if (sscanf(p, "%d", &id) == 1) {
        *outId = id;
        return 1; // successful parse ID
    }
    return 0; // fail to parse ID
}

int main(void) {
    printDeclaration(); //show declaration and welcome message at the startup
    printf("Welcome to CMS. Type HELP to see commands.\n\n");

    char inputline[512]; //buffer to hold full command typed by user
    for (;;) { //main input loop means keep reading command until user exit/eof
        printf("CMS> ");
        if (!fgets(inputline, sizeof(inputline), stdin)) break;
        inputline[strcspn(inputline, "\r\n")] = 0;
        if (strlen(inputline) == 0) continue; //if user just press enter then ignore and continue

        char cmd[64] = { 0 }, arg1[64] = { 0 }, arg2[64] = { 0 }, arg3[64] = { 0 }; //cmd=command= main keyword, arg1-arg3 are extra words if any
        sscanf(inputline, "%63s %63s %63s %63s", cmd, arg1, arg2, arg3);
        for (char* p = cmd; *p; ++p) *p = (char)toupper((unsigned char)*p); //convert main command word to uppercase so input is not case sensitive

        //handle OPEN<filename>
        if (strcmp(cmd, "OPEN") == 0) {
            char* filename = inputline + 4; //move cursor/pointer to rest of the line after "OPEN"
            while (*filename && isspace((unsigned char)*filename)) filename++;
            if (*filename == '\0') printf("CMS: Usage: OPEN <filename>\n");
            else cms_open(filename);
        }
        //handle SHOW commands (like SHOW ALL, SHOW SUMMARY...)
        else if (strcmp(cmd, "SHOW") == 0) {
            if (_stricmp(arg1, "ALL") == 0) {
                // Strict parsing: only accept exact forms
                char t1[64] = { 0 }, t2[64] = { 0 }, t3[64] = { 0 }, t4[64] = { 0 }, t5[64] = { 0 }, t6[64] = { 0 };
                int n = sscanf(inputline, "%63s %63s %63s %63s %63s %63s", t1, t2, t3, t4, t5, t6);
                if (n == 2) {
                    cms_show_all();
                }
                else if (n == 6) {
                    // Expect: SHOW ALL SORT BY <ID|MARK> <ASC|DESC>
                    if (_stricmp(t3, "SORT") == 0 && _stricmp(t4, "BY") == 0) {
                        if ((_stricmp(t5, "ID") == 0 || _stricmp(t5, "MARK") == 0) &&
                            (_stricmp(t6, "ASC") == 0 || _stricmp(t6, "DESC") == 0)) {
                            cms_show_all_sorted(t5, t6);
                        }
                        else {
                            printf("CMS: Invalid SHOW command. Type HELP.\n");
                        }
                    }
                    else {
                        printf("CMS: Invalid SHOW command. Type HELP.\n");
                    }
                }
                else {
                    printf("CMS: Invalid SHOW command. Type HELP.\n");
                }
            }
            else if (_stricmp(arg1, "SUMMARY") == 0) cms_summary();
            else printf("CMS: Unknown SHOW option. Type HELP.\n");
        }
   
        else if (strcmp(cmd, "INSERT") == 0) cms_insert();  //INSERT adds new record
		
        else if (strcmp(cmd, "QUERY") == 0) { //QUERY ID=<id> shows stats of specific student by ID
            int id; if (parseIdToken(inputline, &id)) cms_query(id);
            else printf("CMS: Usage: QUERY ID=<id>\n");
        }
		else if (strcmp(cmd, "UPDATE") == 0) { //UPDATE ID=<id> modifies a existing record by ID
            int id; if (parseIdToken(inputline, &id)) cms_update(id);
            else printf("CMS: Usage: UPDATE ID=<id>\n");
        }
        else if (strcmp(cmd, "DELETE") == 0) { //DELETE ID=<id> removes a specific record by ID
            int id; if (parseIdToken(inputline, &id)) cms_delete(id);
            else printf("CMS: Usage: DELETE ID=<id>\n");
        }
        else if (strcmp(cmd, "SAVE") == 0) cms_save(); //save date
        else if (strcmp(cmd, "EXPORT") == 0 && _stricmp(arg1, "CSV") == 0) { //export student records to csv file (our unique feature)
            char* filename = strstr(inputline, "CSV"); if (filename) { filename += 3; while (*filename && isspace((unsigned char)*filename)) filename++; }
            if (!filename || *filename == '\0') printf("CMS: Usage: EXPORT CSV <filename>\n");
            else cms_export_csv(filename);
        }
        else if (strcmp(cmd, "GRADE") == 0) { //displays grade for specific student by ID (our unique feature)
            int id; if (parseIdToken(inputline, &id)) cms_grade(id);
            else printf("CMS: Usage: GRADE ID=<id>\n");
        }
        else if (strcmp(cmd, "TOPPERCENT") == 0) { //shows top percentage of students by mark (our unique feature)
            float percent; 
            if (sscanf(inputline, "%*s %f", &percent) == 1)
                cms_toppercent(percent);
            else 
                printf("CMS: Usage: TOPPERCENT <percent>\n");
        }
        else if (strcmp(cmd, "HELP") == 0) printHelp(); //shows lists of commands
        else if (strcmp(cmd, "EXIT") == 0 || strcmp(cmd, "QUIT") == 0) break;
        else printf("CMS: Unknown command. Type HELP.\n"); //anything else is unknown command
    }

    printf("Goodbye.\n");
    return 0;

}
