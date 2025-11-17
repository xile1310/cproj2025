
// cms.h – interface between main.c and cms.c
#ifndef CMS_H
#define CMS_H

void cms_printDeclaration(void);
void cms_printHelp(void);

void cms_openDatabase(const char* restOfLine);
void cms_handleShowCommand(const char* line);
void cms_insertRecord(void);
void cms_queryCommand(const char* line);
void cms_updateCommand(const char* line);
void cms_deleteCommand(const char* line);
void cms_saveDatabase(void);
void cms_exportCommand(const char* line);
void cms_gradeCommand(const char* line);

#endif
