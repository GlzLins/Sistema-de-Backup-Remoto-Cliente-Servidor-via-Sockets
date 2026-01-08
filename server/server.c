#include "auth.h"
#include "storage.h"
#include "quota.h"
#include "udp_discovery.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT     4443
#define CMD_MAX  16
#define USER_MAX 128
#define PASS_MAX 128
#define OP_MAX   16


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


  // OpenSSL / TLS
   

static void init_openssl(void) {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

static void cleanup_openssl(void) {
    EVP_cleanup();
}

static SSL_CTX *create_context(void) {
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);

    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

static void configure_context(SSL_CTX *ctx) {

    if (SSL_CTX_use_certificate_file(ctx, "../server/server.crt", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "../server/server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    int server_fd;
    struct sockaddr_in addr;

    init_openssl();
    SSL_CTX *ctx = create_context();
    configure_context(ctx);

    /* UDP discovery em background */
    start_udp_discovery();

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        close(server_fd);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return 1;
    }

    printf("Servidor TLS aguardando conexões na porta %d...\n", PORT);
    fflush(stdout);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t clen = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &clen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        SSL *ssl = SSL_new(ctx);
        if (!ssl) {
            close(client_fd);
            continue;
        }
        SSL_set_fd(ssl, client_fd);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(client_fd);
            continue;
        }

        printf("Conexão TLS estabelecida com sucesso!\n");
        fflush(stdout);

        //auth

        char cmd[CMD_MAX]   = {0};
        char user[USER_MAX] = {0};
        char pass[PASS_MAX] = {0};

        if (ssl_read_exact(ssl, cmd,  CMD_MAX)  < 0) goto cleanup_client;
        if (ssl_read_exact(ssl, user, USER_MAX) < 0) goto cleanup_client;
        if (ssl_read_exact(ssl, pass, PASS_MAX) < 0) goto cleanup_client;

        cmd[CMD_MAX - 1]   = '\0';
        user[USER_MAX - 1] = '\0';
        pass[PASS_MAX - 1] = '\0';

        if (strncmp(cmd, "AUTH", 4) != 0) {
            SSL_write(ssl, "AUTH_FAIL", 9);
            fprintf(stderr, "AUTH inválido: cmd='%s'\n", cmd);
            goto cleanup_client;
        }

        if (!authenticate_user(user, pass)) {
            SSL_write(ssl, "AUTH_FAIL", 9);
            fprintf(stderr, "Falha de autenticação: user='%s'\n", user);
            goto cleanup_client;
        }

        SSL_write(ssl, "AUTH_OK", 7);
        printf("Usuário autenticado: %s\n", user);
        fflush(stdout);

     

        char operation[OP_MAX] = {0};
        if (ssl_read_exact(ssl, operation, OP_MAX) < 0) goto cleanup_client;
        operation[OP_MAX - 1] = '\0';

        if (strncmp(operation, "BACKUP", 6) == 0) {
            handle_backup(ssl, user);
        }
        else if (strncmp(operation, "RESTORE", 7) == 0) {
            handle_restore(ssl, user);
        }
        else if (strncmp(operation, "LIST", 4) == 0) {
        handle_list(ssl, user);
        }
        else {
            SSL_write(ssl, "FAIL", 4);
            fprintf(stderr, "Operação inválida: '%s'\n", operation);
        }

cleanup_client:
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client_fd);
    }

   
    close(server_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 0;
}
