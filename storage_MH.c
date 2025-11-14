#include "cms.h"

StudentRecord *records = NULL; //dynamic array holding student records in RAM
size_t record_count = 0; // how many records are currently stored
size_t record_capacity = 0; // allocated capacity of the records array
char db_filename[256] = {0}; // name of the last opened DB file


//make sure records has space for at least one more record.
void capacity_check() {
    if (record_capacity == 0) {
        record_capacity = 16;
        records = malloc(record_capacity * sizeof(StudentRecord));
    } else if (record_count + 1 > record_capacity) {
        record_capacity *= 2;
        records = realloc(records, record_capacity * sizeof(StudentRecord));
    }
}

// Trim front and back spaces 
static void spaces_trim(char *s) {
    char *start = s;
    while(*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start)+1);
    /* trim trailing */
    int len = (int)strlen(s);
    while(len>0 && isspace((unsigned char)s[len-1])) { s[len-1]=0; len--; }
}

void trim_newline(char *s) {
    int l = (int)strlen(s);
    while (l>0 && (s[l-1]=='\n' || s[l-1]=='\r')) { s[l-1]=0; l--; }
}

//Parse line tokens separated by tab or multiple spaces.
   //Expected: ID  Name  Programme  Mark
static int parse_line(const char *line, StudentRecord *out) {
    char tmp[LINE_MAX];
    strncpy(tmp, line, LINE_MAX-1); tmp[LINE_MAX-1]=0;
    trim_newline(tmp);

    // Try splitting by tab first
    char *tok;
    char *saveptr;
    char *parts[4] = {0};
    int p = 0;

    tok = strtok_r(tmp, "\t", &saveptr);
    while(tok && p<4) {
        parts[p++] = tok;
        tok = strtok_r(NULL, "\t", &saveptr);
    }

    if (p < 4) {
        // fallback split by two or more spaces 
        strncpy(tmp, line, LINE_MAX-1); tmp[LINE_MAX-1]=0;
        p = 0;
        char *s = tmp;
        while (*s) {
            while(*s && !isspace((unsigned char)*s)) s++;
            // split by any sequence of spaces 
            // use strtok by spaces 
            break;
        }
        //* simpler approach: sscanf for ID at front, mark at end, middle is name & programme */
        int id;
        float mark;

        /* Find last token as mark */
        char *last_space = strrchr(tmp, ' ');
        if (last_space) {
            char mark_str[64];
            strncpy(mark_str, last_space+1, 63); mark_str[63]=0;
            if (sscanf(mark_str, "%f", &mark) == 1) {
                /* remove trailing mark from tmp */
                *last_space = '\0';
                /* now assume first token is ID */
                char idstr[64];
                if (sscanf(tmp, "%d", &id) == 1) {
                    /* remove ID from tmp: skip ID and spaces */
                    char *after_id = tmp;
                    while(*after_id && !isspace((unsigned char)*after_id)) after_id++;
                    while(*after_id && isspace((unsigned char)*after_id)) after_id++;
                    /* remaining is Name and Programme - we split roughly in middle */
                    /* Attempt to split by two spaces for name/programme */
                    char *mid = strstr(after_id, "  ");
                    char namebuf[NAME_MAX] = {0};
                    char progbuf[PROG_MAX] = {0};
                    if (mid) {
                        strncpy(namebuf, after_id, (size_t)(mid - after_id));
                        trim_spaces(namebuf);
                        strncpy(progbuf, mid+2, PROG_MAX-1);
                        trim_spaces(progbuf);
                    } else {
                        /* fallback: try to split by the last space in after_id */
                        char *last = strrchr(after_id, ' ');
                        if (last) {
                            strncpy(namebuf, after_id, (size_t)(last - after_id));
                            trim_spaces(namebuf);
                            strncpy(progbuf, last+1, PROG_MAX-1);
                            trim_spaces(progbuf);
                        } else {
                            strncpy(namebuf, after_id, NAME_MAX-1);
                            progbuf[0] = '\0';
                        }
                    }
                    out->ID = id;
                    strncpy(out->Name, namebuf, NAME_MAX-1);
                    strncpy(out->Programme, progbuf, PROG_MAX-1);
                    out->Mark = mark;
                    return 1;
                }
            }
        }
        return 0;
    }

    /* If we got 4 parts */
    trim_spaces(parts[0]); trim_spaces(parts[1]); trim_spaces(parts[2]); trim_spaces(parts[3]);
    out->ID = atoi(parts[0]);
    strncpy(out->Name, parts[1], NAME_MAX-1);
    strncpy(out->Programme, parts[2], PROG_MAX-1);
    out->Mark = (float)atof(parts[3]);
    return 1;
}

// open and load the records 
int open_db(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    record_count = 0;

    char line[LINE_MAX];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (strlen(line) == 0) continue;
        // skip header lines that include ID or Table Name
        if (strstr(line, "ID") && strstr(line, "Name") && strstr(line, "Programme")) continue;
        StudentRecord r;
        if (parse_line(line, &r)) {
            ensure_capacity();
            records[record_count++] = r;
        }
    }
    fclose(f);
    strncpy(db_filename, filename, sizeof(db_filename)-1);
    return 1;
}

// Attempt to save records to file 
int save_db(const char *filename) {
    const char *fname = filename ? filename : db_filename;
    if (!fname || strlen(fname) == 0) return 0;
    FILE *f = fopen(fname, "w");
    if (!f) return 0;

    // Write simple header 
    fprintf(f, "ID\tName\tProgramme\tMark\n");
    for (size_t i=0;i<record_count;i++) {
        fprintf(f, "%d\t%s\t%s\t%.1f\n", records[i].ID, records[i].Name, records[i].Programme, records[i].Mark);
    }
    fclose(f);
    return 1;
}
