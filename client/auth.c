#include "auth.h"
#include <string.h>
#include <stdio.h>

#define CMD_MAX   16
#define USER_MAX  128
#define PASS_MAX  128

static int ssl_write_exact(SSL *ssl, const void *buf, size_t n) {
    size_t total = 0;
    const unsigned char *p = (const unsigned char*)buf;
    while (total < n) {
        int w = SSL_write(ssl, p + total, (int)(n - total));
        if (w <= 0) return -1;
        total += (size_t)w;
    }
    return 0;
}

int auth_client(SSL *ssl, const char *user, const char *pass) {
    char cmd[CMD_MAX] = {0};
    char u[USER_MAX]  = {0};
    char p[PASS_MAX]  = {0};
    char response[32] = {0};

    strncpy(cmd, "AUTH", CMD_MAX - 1);
    strncpy(u, user, USER_MAX - 1);
    strncpy(p, pass, PASS_MAX - 1);

    if (ssl_write_exact(ssl, cmd, CMD_MAX) < 0) return 0;
    if (ssl_write_exact(ssl, u, USER_MAX) < 0) return 0;
    if (ssl_write_exact(ssl, p, PASS_MAX) < 0) return 0;

    int r = SSL_read(ssl, response, sizeof(response) - 1);
    if (r <= 0) return 0;
    response[r] = '\0';

    if (strcmp(response, "AUTH_OK") == 0) {
        printf("Autenticação bem-sucedida!\n");
        return 1;
    }

    printf("Falha na autenticação\n");
    return 0;
}
