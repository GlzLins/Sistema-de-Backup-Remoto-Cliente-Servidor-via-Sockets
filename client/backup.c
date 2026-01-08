// Envio de arquivos (backup)
#include "backup.h"
#include <stdio.h>
#include <string.h>
#include <openssl/ssl.h>

#define BUFFER_SIZE 4096
#define FILENAME_MAX_LEN 256

static int ssl_write_exact(SSL *ssl, const void *buf, size_t n) {
    size_t total = 0;
    const unsigned char *p = buf;

    while (total < n) {
        int w = SSL_write(ssl, p + total, n - total);
        if (w <= 0) return -1;
        total += w;
    }
    return 0;
}

static int ssl_read_exact(SSL *ssl, void *buf, size_t n) {
    size_t total = 0;
    unsigned char *p = buf;

    while (total < n) {
        int r = SSL_read(ssl, p + total, n - total);
        if (r <= 0) return -1;
        total += r;
    }
    return 0;
}

int backup_file(SSL *ssl, const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    const char *filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;

    char name[FILENAME_MAX_LEN] = {0};
    strncpy(name, filename, FILENAME_MAX_LEN - 1);

    if (ssl_write_exact(ssl, name, FILENAME_MAX_LEN) < 0) return 0;
    if (ssl_write_exact(ssl, &filesize, sizeof(filesize)) < 0) return 0;

    char response[16] = {0};
    if (SSL_read(ssl, response, sizeof(response) - 1) <= 0) return 0;

    if (strcmp(response, "READY") != 0) {
        printf("Servidor recusou o backup (%s)\n", response);
        fclose(file);
        return 0;
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

    SSL_read(ssl, response, sizeof(response) - 1);

    if (strcmp(response, "BACKUP_OK") == 0) {
        printf("Backup enviado com sucesso!\n");
        return 1;
    }

    printf("Erro no backup\n");
    return 0;
}
