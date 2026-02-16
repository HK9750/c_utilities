#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int id;
    char *name;
    float grade;
} Student;

typedef struct {
    Student **students;
    int count;
    int capacity;
} StudentList;

static char* util_strdup(const char *src) {
    if (!src) return NULL;
    size_t len = strlen(src) + 1;
    char *dst = (char*)malloc(len);
    if (dst) {
        memcpy(dst, src, len);
    }
    return dst;
}

static Student* student_create(int id, const char *name, float grade) {
    Student *s = (Student*)malloc(sizeof(Student));
    if (!s) {
        fprintf(stderr, "Error: Failed to allocate student struct.\n");
        return NULL;
    }

    s->name = util_strdup(name);
    if (!s->name) {
        fprintf(stderr, "Error: Failed to allocate student name.\n");
        free(s);
        return NULL;
    }

    s->id = id;
    s->grade = grade;
    return s;
}

static void student_free(Student *s) {
    if (s) {
        free(s->name);   /* free the dynamically allocated name */
        free(s);         /* free the struct itself */
    }
}

static void list_init(StudentList *list) {
    list->students = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void list_free_all(StudentList *list) {
    for (int i = 0; i < list->count; i++) {
        student_free(list->students[i]);
    }
    free(list->students);
    list_init(list);   /* reset to safe state */
}

static int list_reserve(StudentList *list, int min_capacity) {
    if (min_capacity <= list->capacity) {
        return 0;   /* already enough */
    }

    /* Grow by factor 1.5 or to min_capacity, whichever is larger */
    int new_cap = list->capacity + (list->capacity / 2);
    if (new_cap < min_capacity) {
        new_cap = min_capacity;
    }
    if (new_cap < 4) new_cap = 4;   /* at least 4 slots initially */

    Student **new_array = (Student**)realloc(list->students, new_cap * sizeof(Student*));
    if (!new_array) {
        fprintf(stderr, "Error: Failed to expand student list.\n");
        return -1;
    }

    list->students = new_array;
    list->capacity = new_cap;
    return 0;
}

static int list_add(StudentList *list, int id, const char *name, float grade) {
    /* First, ensure we have space */
    if (list->count == list->capacity) {
        if (list_reserve(list, list->count + 1) != 0) {
            return -1;   /* allocation failed */
        }
    }

    /* Create the student object */
    Student *s = student_create(id, name, grade);
    if (!s) {
        return -1;
    }

    /* Insert into array */
    list->students[list->count] = s;
    list->count++;
    return 0;
}

static int list_remove_by_id(StudentList *list, int id) {
    int found_idx = -1;

    /* Find index of student with given id */
    for (int i = 0; i < list->count; i++) {
        if (list->students[i]->id == id) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1) {
        return 0;   /* not found */
    }

    /* Free the student at found_idx */
    student_free(list->students[found_idx]);

    /* Shift remaining students left */
    for (int i = found_idx; i < list->count - 1; i++) {
        list->students[i] = list->students[i + 1];
    }
    list->count--;

    /* Optional: shrink the array if it's too sparse */
    if (list->count > 0 && list->count <= list->capacity / 2) {
        int new_cap = list->count;   /* could also keep some slack */
        Student **new_array = (Student**)realloc(list->students, 
                                                 new_cap * sizeof(Student*));
        if (new_array) {
            list->students = new_array;
            list->capacity = new_cap;
        } else {
            /* Shrink failed, but we still have the original array; continue */
            fprintf(stderr, "Warning: Failed to shrink array, but removal succeeded.\n");
        }
    } else if (list->count == 0) {
        /* No students left: free the array entirely */
        free(list->students);
        list->students = NULL;
        list->capacity = 0;
    }

    return 1;   /* removed */
}

static void list_print(const StudentList *list) {
    if (list->count == 0) {
        printf("No students in the list.\n");
        return;
    }

    printf("\n%-6s %-20s %s\n", "ID", "Name", "Grade");
    printf("------ -------------------- -----\n");
    for (int i = 0; i < list->count; i++) {
        Student *s = list->students[i];
        printf("%-6d %-20s %.2f\n", s->id, s->name, s->grade);
    }
    printf("\n");
}

static char get_command(void) {
    char line[32];
    printf("\nCommands: (a)dd, (l)ist, (r)emove, (q)uit\n> ");
    if (!fgets(line, sizeof(line), stdin)) {
        return 'q';   /* EOF */
    }
    /* Return the first non-whitespace character, lowercased */
    for (char *p = line; *p; p++) {
        if (!isspace(*p)) {
            return tolower(*p);
        }
    }
    return 0;   /* empty line */
}

static int read_int(const char *prompt, int *value) {
    char buffer[64];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    if (sscanf(buffer, "%d", value) == 1) {
        return 1;
    }
    return 0;
}

static int read_float(const char *prompt, float *value) {
    char buffer[64];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    if (sscanf(buffer, "%f", value) == 1) {
        return 1;
    }
    return 0;
}

static void read_string(const char *prompt, char *dest, size_t size) {
    printf("%s", prompt);
    if (!fgets(dest, (int)size, stdin)) {
        dest[0] = '\0';
        return;
    }
    /* Remove trailing newline */
    size_t len = strlen(dest);
    if (len > 0 && dest[len-1] == '\n') {
        dest[len-1] = '\0';
    }
}

int main(void) {
    StudentList list;
    list_init(&list);

    char command;
    int running = 1;

    printf("=== Student Record System ===\n");
    printf("Manage your students with simple commands.\n");

    while (running) {
        command = get_command();

        switch (command) {
            case 'a': {   /* add */
                int id;
                char name[256];
                float grade;

                if (!read_int("Enter student ID: ", &id)) {
                    printf("Invalid ID. Please enter an integer.\n");
                    break;
                }
                read_string("Enter student name: ", name, sizeof(name));
                if (strlen(name) == 0) {
                    printf("Name cannot be empty.\n");
                    break;
                }
                if (!read_float("Enter student grade: ", &grade)) {
                    printf("Invalid grade. Please enter a number.\n");
                    break;
                }

                if (list_add(&list, id, name, grade) == 0) {
                    printf("Student added successfully.\n");
                } else {
                    printf("Failed to add student.\n");
                }
                break;
            }

            case 'l':   /* list */
                list_print(&list);
                break;

            case 'r': {   /* remove */
                int id;
                if (!read_int("Enter ID of student to remove: ", &id)) {
                    printf("Invalid ID.\n");
                    break;
                }
                if (list_remove_by_id(&list, id)) {
                    printf("Student removed.\n");
                } else {
                    printf("Student with ID %d not found.\n", id);
                }
                break;
            }

            case 'q':   /* quit */
                running = 0;
                break;

            default:
                printf("Unknown command. Use a, l, r, or q.\n");
                break;
        }
    }

    /* Clean up before exit */
    list_free_all(&list);
    printf("Goodbye!\n");
    return 0;
}