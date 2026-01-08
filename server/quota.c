//Controle de cota por usuário
#include "quota.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>


long get_user_quota(const char *user) {
    FILE *file = fopen("../server/quotas.txt", "r");
    if (!file) {
        return -1;
    }

    char line[256];
    char f_user[128];
    long quota;

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%127[^:]:%ld", f_user, &quota) == 2) {
            if (strcmp(user, f_user) == 0) {
                fclose(file);
                return quota;
            }
        }
    }

    fclose(file);
    return -1;
}


long get_user_usage(const char *user) {
    char path[256];
    snprintf(path, sizeof(path), "../data/%s", user);

    DIR *dir = opendir(path);
    if (!dir) {
        return 0;
    }

    struct dirent *entry;
    struct stat st;
    long total = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

            if (stat(filepath, &st) == 0) {
                total += st.st_size;
            }
        }
    }

    closedir(dir);
    return total;
}


