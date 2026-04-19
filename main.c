#define _XOPEN_SOURCE 500

#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

const int max_dir_holds = 114;

typedef struct{
    char *fpath;
    struct stat sb;
} file_t;

typedef struct{
   size_t capacity;
   size_t p;
   file_t *file;
} file_array;

file_array f_arr;

void init_file_array(file_array* farr){
    farr->capacity = 1;
    farr->p = 0;
    farr->file = malloc(sizeof(file_t));
    if(farr->file == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

int cmp_file_size(const void *p1, const void *p2){
    const file_t *f1 = p1;
    const file_t *f2 = p2;

    return (f2->sb.st_size > f1->sb.st_size) - (f2->sb.st_size < f1->sb.st_size);
}

void f_append(file_array *files , file_t file){
    if(files->p == files->capacity){
        size_t new_cap = 2 * files->capacity;
        file_t *new_files = realloc(files->file, new_cap * sizeof(file_t));
        if(new_files == NULL){
            perror("realloc");
            free(files->file);
            exit(EXIT_FAILURE);
        }
        files->file = new_files;
        files->capacity = new_cap;
    }
    files->file[files->p++] = file;
}

void print_info(const char* fpath, const struct stat* sb){
    printf("%s: %ld bytes\n", fpath, (long)sb->st_size);
}

void format_size(off_t size, char *buffer, size_t buffer_size){
    static const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double scaled_size = (double)size;
    size_t unit_index = 0;

    while(scaled_size >= 1024.0 && unit_index < (sizeof(units) / sizeof(units[0])) - 1){
        scaled_size /= 1024.0;
        unit_index++;
    }

    if(unit_index == 0){
        snprintf(buffer, buffer_size, "%lld %s", (long long)size, units[unit_index]);
    }else{
        snprintf(buffer, buffer_size, "%.1f %s", scaled_size, units[unit_index]);
    }
}

void print_chart(const file_array *files){
    const int path_width = 30;
    const int max_bar_width = 31;
    off_t max_size = 1;

    if(files->p == 0){
        return;
    }

    if(files->file[0].sb.st_size > 0){
        max_size = files->file[0].sb.st_size;
    }

    for(size_t i = 0; i < files->p; ++i){
        char size_buffer[32];
        int bar_len = 0;

        format_size(files->file[i].sb.st_size, size_buffer, sizeof(size_buffer));
        if(files->file[i].sb.st_size > 0){
            bar_len = (int)((files->file[i].sb.st_size * max_bar_width) / max_size);
            if(bar_len == 0){
                bar_len = 1;
            }
        }

        printf("%-*s %8s  ", path_width, files->file[i].fpath, size_buffer);
        for(int j = 0; j < bar_len; ++j){
            putchar('#');
        }
        putchar('\n');
    }
}

int ftw_callback(const char *fpath, const struct stat *sb, int typeflag) {
    (void)typeflag;
    //print_info(fpath, sb);
    if(strcmp(fpath, ".") == 0){
        return 0;
    }
    file_t file = {strdup(fpath), *sb};
    if(file.fpath == NULL){
        perror("strdup");
        return 1;
    }
    if(S_ISREG(sb->st_mode)){
        f_append(&f_arr, file);
    }else{
        free(file.fpath);
    }
    return 0;
}

int main(int argc, char **argv) {
    if(argc != 2){
        printf("usage: %s [path]\n", argv[0]);
        return 0;
    }
    const char *path = argv[1];
    init_file_array(&f_arr);
    if(ftw(path, ftw_callback, max_dir_holds) != 0){
        perror("ftw");
        free(f_arr.file);
        return EXIT_FAILURE;
    }
    qsort(f_arr.file, f_arr.p, sizeof(file_t), cmp_file_size);

    print_chart(&f_arr);

    for(size_t i = 0; i < f_arr.p; ++i){
        free(f_arr.file[i].fpath);
    }
    free(f_arr.file);
    return 0;
}
