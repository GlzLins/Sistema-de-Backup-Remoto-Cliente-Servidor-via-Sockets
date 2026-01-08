#include "auth.h"
#include <stdio.h>
#include <string.h>

#define USERS_FILE "../server/users.txt"

int authenticate_user(const char *user, const char *pass) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        perror("users.txt");
        return 0;
    }

    char line[256];
    char f_user[128];
    char f_pass[128];

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%127[^:]:%127s", f_user, f_pass) == 2) {
            if (strcmp(user, f_user) == 0 &&
                strcmp(pass, f_pass) == 0) {
                fclose(file);
                return 1;
            }
        }
    }

    fclose(file);
    return 0;
}
