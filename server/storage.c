#include "storage.h"
#include "quota.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>


#include <openssl/ssl.h>

#define BUFFER_SIZE 4096
#define FILENAME_MAX_LEN 256
#define DATA_DIR "../data"
#define OP_MAX 16



void handle_list(SSL *ssl, const char *user) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", DATA_DIR, user);

    long quota = get_user_quota(user);
    long usage = get_user_usage(user);

    /* envia cota e uso */
    SSL_write(ssl, &quota, sizeof(quota));
    SSL_write(ssl, &usage, sizeof(usage));

    DIR *dir = opendir(path);
    if (!dir) {
        int zero = 0;
        SSL_write(ssl, &zero, sizeof(zero));
        return;
    }

    struct dirent *entry;
    struct stat st;
    int count = 0;

    /* conta arquivos */
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG)
            count++;
    }

    rewinddir(dir);
    SSL_write(ssl, &count, sizeof(count));

    char filepath[PATH_MAX];

    /* envia nome + tamanho */
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_REG)
            continue;

        int n = snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        if (n < 0 || n >= (int)sizeof(filepath))
            continue;

        if (stat(filepath, &st) == 0) {
            char name[FILENAME_MAX_LEN] = {0};
            strncpy(name, entry->d_name, FILENAME_MAX_LEN - 1);

            SSL_write(ssl, name, FILENAME_MAX_LEN);
            SSL_write(ssl, &st.st_size, sizeof(st.st_size));
        }
    }

    closedir(dir);
}


    
void ensure_user_dir(const char *user) {
    struct stat st;
    char path[512];

    /* cria ../data se não existir */
    if (stat("../data", &st) == -1) {
        if (mkdir("../data", 0755) == -1) {
            perror("mkdir ../data");
            return;
        }
    }

    /* cria ../data/<user> se não existir */
    snprintf(path, sizeof(path), "%s/%s", DATA_DIR, user);

    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("mkdir user dir");
            return;
        }
    }
}




static int ssl_read_exact(SSL *ssl, void *buf, size_t n) {
    size_t total = 0;
    unsigned char *p = (unsigned char *)buf;

    while (total < n) {
        int r = SSL_read(ssl, p + total, (int)(n - total));
        if (r <= 0) return -1;
        total += (size_t)r;
    }
    return 0;
}

//backup

void handle_backup(SSL *ssl, const char *user) {
    char filename[FILENAME_MAX_LEN] = {0};
    long filesize = 0;

    
    if (ssl_read_exact(ssl, filename, FILENAME_MAX_LEN) < 0) return;
    if (ssl_read_exact(ssl, &filesize, sizeof(filesize)) < 0) return;

    filename[FILENAME_MAX_LEN - 1] = '\0';

    long quota = get_user_quota(user);
    long used  = get_user_usage(user);

    if (quota < 0) {
    SSL_write(ssl, "NO_QUOTA", 8);
    printf("Usuário %s sem cota definida\n", user);
    return;
}

    if (quota > 0 && used + filesize > quota) {
    SSL_write(ssl, "QUOTA_EXCEEDED", 14);
    printf("Quota excedida: %s (%ld/%ld)\n", user, used, quota);
    return;
}

    ensure_user_dir(user);

    char path[512];
    snprintf(path, sizeof(path), "%s/%s/%s", DATA_DIR, user, filename);

    FILE *file = fopen(path, "wb");
    if (!file) {
        perror("fopen backup");
        SSL_write(ssl, "FAIL", 4);
        return;
    }

    SSL_write(ssl, "READY", 5);

    char buffer[BUFFER_SIZE];
    long received = 0;

    while (received < filesize) {
        int bytes = SSL_read(ssl, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;
        fwrite(buffer, 1, bytes, file);
        received += bytes;
    }

    fclose(file);

    if (received == filesize) {
        SSL_write(ssl, "BACKUP_OK", 9);
        printf("Backup salvo: %s (%ld bytes)\n", path, filesize);
    } else {
        SSL_write(ssl, "FAIL", 4);
        printf("Backup incompleto (%ld/%ld)\n", received, filesize);
    }

    fflush(stdout);
}

//restore

void handle_restore(SSL *ssl, const char *user) {
    char filename[FILENAME_MAX_LEN] = {0};

    
    if (ssl_read_exact(ssl, filename, FILENAME_MAX_LEN) < 0) return;

    filename[FILENAME_MAX_LEN - 1] = '\0';

    char path[512];
    snprintf(path, sizeof(path), "%s/%s/%s", DATA_DIR, user, filename);

    FILE *file = fopen(path, "rb");
    if (!file) {
        SSL_write(ssl, "NOT_FOUND", 9);
        printf("Restore NOT_FOUND: %s\n", path);
        fflush(stdout);
        return;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    SSL_write(ssl, "FOUND", 5);
    SSL_write(ssl, &filesize, sizeof(filesize));

    char response[16] = {0};
    if (SSL_read(ssl, response, sizeof(response) - 1) <= 0 || strcmp(response, "READY") != 0) {
        fclose(file);
        return;
    }

    char buffer[BUFFER_SIZE];
    long sent = 0;

    while (sent < filesize) {
        int bytes = fread(buffer, 1, BUFFER_SIZE, file);
        if (bytes <= 0) break;
        SSL_write(ssl, buffer, bytes);
        sent += bytes;
    }

    fclose(file);

    printf("Restore enviado: %s (%ld bytes)\n", path, filesize);
    fflush(stdout);
}
