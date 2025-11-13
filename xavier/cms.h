#ifndef CMS_H
#define CMS_H

void cms_open(const char* filename);
void cms_save(void);
void cms_export_csv(const char* filename);

void cms_show_all(void);
void cms_show_all_sorted(const char* field, const char* order);
void cms_insert(void);
void cms_query(int id);
void cms_update(int id);
void cms_delete(int id);
void cms_summary(void);
void cms_grade(int id);

#endif