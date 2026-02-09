#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

typedef enum {
    NODE_FILE,
    NODE_DIR
} NodeType;

typedef struct Node {
    char *path;
    char *name;
    NodeType type;
    struct Node **children;
    size_t no_of_childs;
} Node;

typedef struct Result {
    unsigned long no_of_folders;
    unsigned long no_of_files;
    unsigned long total_lines;
} Result;

#define EXT_TABLE_SIZE 1024

typedef struct ExtStat {
    char *ext;
    size_t file_count;
    size_t line_count;
    struct ExtStat *next;
} ExtStat;

static ExtStat *ext_table[EXT_TABLE_SIZE];

static const char *IGNORE_DIRS[] = {
    ".",
    "..",
    ".git",
    ".gitignore",
    ".gitmodules",
    ".gitlab",
    ".github",
    ".svn",
    ".hg",
    ".vscode",
    ".idea",
    ".fleet",
    ".settings",
    ".cursor",
    ".claude",
    ".zed",
    ".next",
    ".nuxt",
    ".svelte-kit",
    ".angular",
    ".turbo",
    ".vercel",
    ".netlify",
    ".expo",
    "node_modules",
    "bower_components",
    "jspm_packages",
    "__pycache__",
    ".pytest_cache",
    ".mypy_cache",
    ".ruff_cache",
    ".tox",
    ".nox",
    "venv",
    ".venv",
    "env",
    ".env",
    "target",
    ".gradle",
    ".mvn",
    "build",
    "dist",
    "out",
    "bin",
    "obj",
    "vendor",
    "Pods",
    "Carthage",
    ".cache",
    ".parcel-cache",
    ".turbo-cache",
    ".DS_Store",
    "Thumbs.db",
    ".coverage",
    "coverage",
    "lcov-report",
    ".terraform",
    ".terragrunt-cache",
    ".cargo",
    "debug",
    "release",
    ".next-cache",
    ".svelte-cache",
    ".turbo"
};

static const size_t IGNORE_COUNT = sizeof(IGNORE_DIRS) / sizeof(IGNORE_DIRS[0]);

int should_ignore(const char *name) {
    if (!name) return 0;
    for (size_t i = 0; i < IGNORE_COUNT; i++) {
        if (strcmp(IGNORE_DIRS[i], name) == 0) return 1;
    }
    return 0;
}

Node *create_node(const char *path, const char *name, NodeType type) {
    Node *n = malloc(sizeof(Node));
    if (!n) return NULL;
    n->path = strdup(path);
    n->name = strdup(name);
    n->type = type;
    n->children = NULL;
    n->no_of_childs = 0;
    return n;
}

void add_child(Node *parent, Node *child) {
    Node **tmp = realloc(parent->children, sizeof(Node *) * (parent->no_of_childs + 1));
    if (!tmp) return;
    parent->children = tmp;
    parent->children[parent->no_of_childs++] = child;
}

unsigned long hash_ext(const char *s) {
    unsigned long h = 5381;
    int c;
    while ((c = *s++))
        h = ((h << 5) + h) + (unsigned char)c;
    return h % EXT_TABLE_SIZE;
}

ExtStat *get_ext_stat(const char *ext) {
    if (!ext) ext = "noext";
    unsigned long idx = hash_ext(ext);
    ExtStat *e = ext_table[idx];
    while (e) {
        if (strcmp(e->ext, ext) == 0) return e;
        e = e->next;
    }
    e = malloc(sizeof(ExtStat));
    if (!e) return NULL;
    e->ext = strdup(ext);
    e->file_count = 0;
    e->line_count = 0;
    e->next = ext_table[idx];
    ext_table[idx] = e;
    return e;
}

const char *get_extension(const char *name) {
    if (!name) return "noext";
    const char *dot = strrchr(name, '.');
    if (!dot || dot == name) return "noext";
    return dot + 1;
}

size_t count_lines(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    size_t lines = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') lines++;
    }
    fclose(f);
    return lines;
}

Node *build_tree(const char *input_path) {
    struct stat st;
    if (stat(input_path, &st) != 0) return NULL;
    char path[4096];
    strncpy(path, input_path, sizeof(path));
    path[sizeof(path) - 1] = '\0';
    size_t len = strlen(path);
    while (len > 1 && path[len - 1] == '/') {
        path[len - 1] = '\0';
        len--;
    }
    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;
    NodeType type = S_ISDIR(st.st_mode) ? NODE_DIR : NODE_FILE;
    if (type == NODE_DIR && should_ignore(name)) return NULL;
    Node *node = create_node(path, name, type);
    if (!node) return NULL;
    if (type == NODE_DIR) {
        DIR *dir = opendir(path);
        if (!dir) return node;
        struct dirent *e;
        while ((e = readdir(dir)) != NULL) {
            if (should_ignore(e->d_name)) continue;
            char child_path[4096];
            snprintf(child_path, sizeof(child_path), "%s/%s", path, e->d_name);
            Node *child = build_tree(child_path);
            if (child) add_child(node, child);
        }
        closedir(dir);
    }
    return node;
}

void free_tree(Node *n) {
    if (!n) return;
    for (size_t i = 0; i < n->no_of_childs; i++) free_tree(n->children[i]);
    free(n->children);
    free(n->path);
    free(n->name);
    free(n);
}

void accumulate_stats(Node *n, Result *res) {
    if (!n || !res) return;
    if (n->type == NODE_DIR) {
        res->no_of_folders++;
        for (size_t i = 0; i < n->no_of_childs; i++) accumulate_stats(n->children[i], res);
    } else {
        res->no_of_files++;
        const char *ext = get_extension(n->name);
        ExtStat *es = get_ext_stat(ext);
        if (es) {
            es->file_count++;
            size_t lines = count_lines(n->path);
            es->line_count += lines;
            res->total_lines += lines;
        }
    }
}

size_t collect_ext_stats(ExtStat ***out) {
    size_t count = 0;
    for (size_t i = 0; i < EXT_TABLE_SIZE; i++) {
        ExtStat *e = ext_table[i];
        while (e) {
            count++;
            e = e->next;
        }
    }
    if (count == 0) {
        *out = NULL;
        return 0;
    }
    ExtStat **arr = malloc(sizeof(ExtStat *) * count);
    size_t idx = 0;
    for (size_t i = 0; i < EXT_TABLE_SIZE; i++) {
        ExtStat *e = ext_table[i];
        while (e) {
            arr[idx++] = e;
            e = e->next;
        }
    }
    *out = arr;
    return count;
}

int cmp_lines_desc(const void *a, const void *b) {
    ExtStat *ea = *(ExtStat **)a;
    ExtStat *eb = *(ExtStat **)b;
    if (ea->line_count < eb->line_count) return 1;
    if (ea->line_count > eb->line_count) return -1;
    return 0;
}

void print_report(Result *res) {
    printf("Summary\n");
    printf("-------\n");
    printf("Folders: %lu\n", res->no_of_folders);
    printf("Files:   %lu\n", res->no_of_files);
    printf("Lines:   %lu\n\n", res->total_lines);
    ExtStat **arr;
    size_t n = collect_ext_stats(&arr);
    if (n == 0) {
        printf("No files found\n");
        return;
    }
    qsort(arr, n, sizeof(ExtStat *), cmp_lines_desc);
    printf("%-12s %12s %12s %8s\n", "Extension", "Files", "Lines", "PctL");
    printf("----------------------------------------------------\n");
    for (size_t i = 0; i < n; i++) {
        double pct = res->total_lines ? (100.0 * arr[i]->line_count / res->total_lines) : 0.0;
        printf("%-12s %12zu %12zu %7.2f%%\n", arr[i]->ext, arr[i]->file_count, arr[i]->line_count, pct);
    }
    free(arr);
}

void free_ext_stats(void) {
    for (size_t i = 0; i < EXT_TABLE_SIZE; i++) {
        ExtStat *e = ext_table[i];
        while (e) {
            ExtStat *next = e->next;
            free(e->ext);
            free(e);
            e = next;
        }
        ext_table[i] = NULL;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }
    Node *tree = build_tree(argv[1]);
    if (!tree) return 1;
    Result res = {0, 0, 0};
    accumulate_stats(tree, &res);
    print_report(&res);
    free_tree(tree);
    free_ext_stats();
    return 0;
}
