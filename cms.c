#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_RECORDS 1000
#define MAX_LINE 256
#define DATABASE_FILE "P13-8-CMS.txt"
#define CSV_FILE "P13-8-CMS.csv"

// Structure definition for student records
struct Student {
    int id;
    char name[50];
    char programme[50];
    float mark;
};

// Global variables for storing records
struct Student currentMemory[MAX_RECORDS];  // For INSERT operations
struct Student databaseMemory[MAX_RECORDS]; // For DATABASE operations
int currentCount = 0;    // Count of records in current memory
int databaseCount = 0;   // Count of records in database memory

// Function prototypes
void trim(char *str);
int readDatabase(const char *filename);
int saveDatabase(const char *filename);
void exportCSV(const char *filename);
void showAll(void);
void showDatabase(void);
int findRecord(int id, struct Student *records, int count);
void insertRecord(void);
void queryRecord(void);
void updateRecord(void);
void deleteRecord(void);
int validateMark(float mark);
void clearInputBuffer(void);

// Utility function to trim whitespace
void trim(char *str) {
    char *start = str;
    char *end;
    
    while(isspace((unsigned char)*start)) start++;
    
    if(*start == 0) {
        *str = 0;
        return;
    }
    
    end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = 0;
    memmove(str, start, (end - start) + 2);
}

// Read database from file into database memory
int readDatabase(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("CMS: Error opening database file %s\n", filename);
        return 0;
    }

    char line[MAX_LINE];
    databaseCount = 0;  // Reset database count

    while (fgets(line, sizeof(line), file) && databaseCount < MAX_RECORDS) {
        trim(line);
        if (strlen(line) == 0) continue;

        char *token = strtok(line, ",");
        if (token != NULL) {
            databaseMemory[databaseCount].id = atoi(token);
            
            token = strtok(NULL, ",");
            if (token != NULL) {
                trim(token);
                strncpy(databaseMemory[databaseCount].name, token, sizeof(databaseMemory[databaseCount].name) - 1);
                
                token = strtok(NULL, ",");
                if (token != NULL) {
                    trim(token);
                    strncpy(databaseMemory[databaseCount].programme, token, sizeof(databaseMemory[databaseCount].programme) - 1);
                    
                    token = strtok(NULL, ",");
                    if (token != NULL) {
                        databaseMemory[databaseCount].mark = atof(token);
                        databaseCount++;
                    }
                }
            }
        }
    }

    fclose(file);
    printf("\nRecords from database:\n");
    printf("%-10s %-30s %-20s %-10s\n", "ID", "Name", "Programme", "Mark");
    printf("------------------------------------------------------------\n");
    
    if (databaseCount == 0) {
        printf("No records in database\n");
    } else {
        for (int i = 0; i < databaseCount; i++) {
            printf("%-10d %-30s %-20s %-10.2f\n",
                   databaseMemory[i].id,
                   databaseMemory[i].name,
                   databaseMemory[i].programme,
                   databaseMemory[i].mark);
        }
        printf("\nTotal records in database: %d\n", databaseCount);
    }
    return 1;
}

// Save current memory records to database file
int saveDatabase(const char *filename) {
    // First, read existing records from database
    databaseCount = 0;  // Reset database count before reading
    FILE *readFile = fopen(filename, "r");
    if (readFile != NULL) {
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), readFile) && databaseCount < MAX_RECORDS) {
            trim(line);
            if (strlen(line) == 0) continue;

            char *token = strtok(line, ",");
            if (token != NULL) {
                int id = atoi(token);
                // Skip if ID exists in current memory
                if (findRecord(id, currentMemory, currentCount) != -1) continue;
                
                databaseMemory[databaseCount].id = id;
                
                token = strtok(NULL, ",");
                if (token != NULL) {
                    trim(token);
                    strncpy(databaseMemory[databaseCount].name, token, sizeof(databaseMemory[databaseCount].name) - 1);
                    
                    token = strtok(NULL, ",");
                    if (token != NULL) {
                        trim(token);
                        strncpy(databaseMemory[databaseCount].programme, token, sizeof(databaseMemory[databaseCount].programme) - 1);
                        
                        token = strtok(NULL, ",");
                        if (token != NULL) {
                            databaseMemory[databaseCount].mark = atof(token);
                            databaseCount++;
                        }
                    }
                }
            }
        }
        fclose(readFile);
    }

    // Add current memory records to database memory
    for (int i = 0; i < currentCount; i++) {
        databaseMemory[databaseCount++] = currentMemory[i];
    }

    // Save all records back to file
    FILE *writeFile = fopen(filename, "w");
    if (writeFile == NULL) {
        printf("CMS: Error opening file for writing: %s\n", filename);
        return 0;
    }

    for (int i = 0; i < databaseCount; i++) {
        fprintf(writeFile, "%d,%s,%s,%.2f\n", 
                databaseMemory[i].id,
                databaseMemory[i].name,
                databaseMemory[i].programme,
                databaseMemory[i].mark);
    }

    fclose(writeFile);

    printf("CMS: Successfully saved %d records to database\n", currentCount);
    currentCount = 0;  // Clear current memory after saving
    printf("CMS: Current memory cleared\n");
    return 1;
}

// Export database records to CSV file
void exportCSV(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("CMS: Error opening CSV file for writing\n");
        return;
    }

    fprintf(file, "ID,Name,Programme,Mark\n");

    for (int i = 0; i < databaseCount; i++) {
        fprintf(file, "%d,%s,%s,%.2f\n", 
                databaseMemory[i].id,
                databaseMemory[i].name,
                databaseMemory[i].programme,
                databaseMemory[i].mark);
    }

    fclose(file);
    printf("CMS: Successfully exported %d records to CSV\n", databaseCount);
}

// Display all records in current memory
void showAll(void) {
    if (currentCount == 0) {
        printf("CMS: No records in current memory\n");
        return;
    }

    printf("\nRecords in current memory:\n");
    printf("%-10s %-30s %-20s %-10s\n", "ID", "Name", "Programme", "Mark");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < currentCount; i++) {
        printf("%-10d %-30s %-20s %-10.2f\n",
               currentMemory[i].id,
               currentMemory[i].name,
               currentMemory[i].programme,
               currentMemory[i].mark);
    }
    printf("\nTotal records in current memory: %d\n", currentCount);
}

// Find record by ID in specified memory
int findRecord(int id, struct Student *records, int count) {
    for (int i = 0; i < count; i++) {
        if (records[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Clear input buffer
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Validate mark range
int validateMark(float mark) {
    return mark >= 0 && mark <= 100;
}

// Insert new record into current memory
void insertRecord(void) {
    if (currentCount >= MAX_RECORDS) {
        printf("CMS: Error - Maximum number of records reached in current memory\n");
        return;
    }

    struct Student newStudent;
    printf("Enter student ID: ");
    scanf("%d", &newStudent.id);
    clearInputBuffer();

    // Check for unique ID in current memory
    if (findRecord(newStudent.id, currentMemory, currentCount) != -1) {
        printf("CMS: Error - Student ID %d already exists in current memory\n", newStudent.id);
        return;
    }

    printf("Enter student name: ");
    fgets(newStudent.name, sizeof(newStudent.name), stdin);
    trim(newStudent.name);

    printf("Enter programme: ");
    fgets(newStudent.programme, sizeof(newStudent.programme), stdin);
    trim(newStudent.programme);

    printf("Enter mark (0-100): ");
    scanf("%f", &newStudent.mark);
    clearInputBuffer();

    if (!validateMark(newStudent.mark)) {
        printf("CMS: Error - Mark must be between 0 and 100\n");
        return;
    }

    currentMemory[currentCount++] = newStudent;
    printf("CMS: Record with ID=%d successfully inserted into current memory\n", newStudent.id);
}

// Query record by ID from database
void queryRecord(void) {
    int id;
    printf("Enter student ID to query: ");
    scanf("%d", &id);
    clearInputBuffer();

    int index = findRecord(id, databaseMemory, databaseCount);
    if (index == -1) {
        printf("CMS: No record found with ID=%d in database\n", id);
        return;
    }

    printf("\n%-10s %-30s %-20s %-10s\n", "ID", "Name", "Programme", "Mark");
    printf("------------------------------------------------------------\n");
    printf("%-10d %-30s %-20s %-10.2f\n",
           databaseMemory[index].id,
           databaseMemory[index].name,
           databaseMemory[index].programme,
           databaseMemory[index].mark);
}

// Update record in database
void updateRecord(void) {
    int id;
    printf("Enter student ID to update: ");
    scanf("%d", &id);
    clearInputBuffer();

    int index = findRecord(id, databaseMemory, databaseCount);
    if (index == -1) {
        printf("CMS: No record found with ID=%d in database\n", id);
        return;
    }

    printf("Current record:\n");
    printf("%-10s %-30s %-20s %-10s\n", "ID", "Name", "Programme", "Mark");
    printf("------------------------------------------------------------\n");
    printf("%-10d %-30s %-20s %-10.2f\n",
           databaseMemory[index].id,
           databaseMemory[index].name,
           databaseMemory[index].programme,
           databaseMemory[index].mark);

    printf("\nEnter new name (or press Enter to keep current): ");
    char input[50];
    fgets(input, sizeof(input), stdin);
    trim(input);
    if (strlen(input) > 0) {
        strncpy(databaseMemory[index].name, input, sizeof(databaseMemory[index].name) - 1);
    }

    printf("Enter new programme (or press Enter to keep current): ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    if (strlen(input) > 0) {
        strncpy(databaseMemory[index].programme, input, sizeof(databaseMemory[index].programme) - 1);
    }

    printf("Enter new mark (0-100, or -1 to keep current): ");
    float newMark;
    scanf("%f", &newMark);
    clearInputBuffer();

    if (newMark != -1) {
        if (!validateMark(newMark)) {
            printf("CMS: Error - Mark must be between 0 and 100\n");
            return;
        }
        databaseMemory[index].mark = newMark;
    }

    // Save changes back to file
    FILE *file = fopen(DATABASE_FILE, "w");
    if (file == NULL) {
        printf("CMS: Error opening database file for writing\n");
        return;
    }

    for (int i = 0; i < databaseCount; i++) {
        fprintf(file, "%d,%s,%s,%.2f\n",
                databaseMemory[i].id,
                databaseMemory[i].name,
                databaseMemory[i].programme,
                databaseMemory[i].mark);
    }
    fclose(file);

    printf("CMS: Record with ID=%d successfully updated in database\n", id);
}

// Delete record from database
void deleteRecord(void) {
    int id;
    printf("Enter student ID to delete: ");
    scanf("%d", &id);
    clearInputBuffer();

    int index = findRecord(id, databaseMemory, databaseCount);
    if (index == -1) {
        printf("CMS: No record found with ID=%d in database\n", id);
        return;
    }

    printf("Record to delete:\n");
    printf("%-10s %-30s %-20s %-10s\n", "ID", "Name", "Programme", "Mark");
    printf("------------------------------------------------------------\n");
    printf("%-10d %-30s %-20s %-10.2f\n",
           databaseMemory[index].id,
           databaseMemory[index].name,
           databaseMemory[index].programme,
           databaseMemory[index].mark);

    printf("\nAre you sure you want to delete this record? (y/n): ");
    char confirm;
    scanf(" %c", &confirm);
    clearInputBuffer();

    if (tolower(confirm) == 'y') {
        // Shift remaining records
        for (int i = index; i < databaseCount - 1; i++) {
            databaseMemory[i] = databaseMemory[i + 1];
        }
        databaseCount--;

        // Save changes back to file
        FILE *file = fopen(DATABASE_FILE, "w");
        if (file == NULL) {
            printf("CMS: Error opening database file for writing\n");
            return;
        }

        for (int i = 0; i < databaseCount; i++) {
            fprintf(file, "%d,%s,%s,%.2f\n",
                    databaseMemory[i].id,
                    databaseMemory[i].name,
                    databaseMemory[i].programme,
                    databaseMemory[i].mark);
        }
        fclose(file);

        printf("CMS: Record with ID=%d successfully deleted from database\n", id);
    } else {
        printf("CMS: Deletion cancelled\n");
    }
}

int main() {
    char command[20];
    int running = 1;

    printf("Class Management System (CMS)\n");
    printf("Available commands: OPEN, SHOW ALL, INSERT, QUERY, UPDATE, DELETE, SAVE, EXPORT CSV, EXIT\n");

    while (running) {
        printf("\nCMS> ");
        fgets(command, sizeof(command), stdin);
        trim(command);

        // Convert command to uppercase for case-insensitive comparison
        for (int i = 0; command[i]; i++) {
            command[i] = toupper(command[i]);
        }

        if (strcmp(command, "OPEN") == 0) {
            readDatabase(DATABASE_FILE);
        }
        else if (strcmp(command, "SHOW ALL") == 0) {
            showAll();
        }
        else if (strcmp(command, "INSERT") == 0) {
            insertRecord();
        }
        else if (strcmp(command, "QUERY") == 0) {
            queryRecord();
        }
        else if (strcmp(command, "UPDATE") == 0) {
            updateRecord();
        }
        else if (strcmp(command, "DELETE") == 0) {
            deleteRecord();
        }
        else if (strcmp(command, "SAVE") == 0) {
            saveDatabase(DATABASE_FILE);
        }
        else if (strcmp(command, "EXPORT CSV") == 0) {
            exportCSV(CSV_FILE);
        }
        else if (strcmp(command, "EXIT") == 0) {
            printf("CMS: Exiting program\n");
            running = 0;
        }
        else {
            printf("CMS: Unknown command. Available commands:\n");
            printf("OPEN, SHOW ALL, INSERT, QUERY, UPDATE, DELETE, SAVE, EXPORT CSV, EXIT\n");
        }
    }

    return 0;
}