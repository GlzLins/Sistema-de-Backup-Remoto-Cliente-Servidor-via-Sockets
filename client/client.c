#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "auth.h"
#include "backup.h"
#include "restore.h"
#include "udp_client.h"
#include "list.h"

#define OP_MAX 16

static void send_operation(SSL *ssl, const char *op) {
    char buf[OP_MAX] = {0};
    strncpy(buf, op, OP_MAX - 1);
    SSL_write(ssl, buf, OP_MAX);
}


int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Uso:\n");
        printf("  %s backup <arquivo>\n", argv[0]);
        printf("  %s restore <arquivo>\n", argv[0]);
        printf("  %s list\n", argv[0]);
        return 1;
    }

    const char *operation = argv[1];
    const char *filename  = NULL;

    
    if (strcmp(operation, "backup") == 0 || strcmp(operation, "restore") == 0) {
        if (argc < 3) {
            printf("Erro: operação '%s' exige um arquivo\n", operation);
            return 1;
        }
        filename = argv[2];
    }

   
      // Descoberta via UDP
      
    char server_ip[INET_ADDRSTRLEN];
    int server_port;

    if (!discover_server(server_ip, &server_port)) {
        printf("Erro: não foi possível localizar o servidor via UDP.\n");
        return 1;
    }

    printf("Servidor encontrado em %s:%d\n", server_ip, server_port);

    
       //Inicialização TLS
      
    SSL_CTX *ctx;
    SSL *ssl;

    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        SSL_CTX_free(ctx);
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        SSL_CTX_free(ctx);
        return 1;
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    printf("Conexão TLS estabelecida com sucesso!\n");

    
       //AUTH
      
    char user[128];
    char *pass;

    printf("Usuário: ");
    fflush(stdout);
    fgets(user, sizeof(user), stdin);
    user[strcspn(user, "\n")] = 0;

    pass = getpass("Senha: ");

    if (!auth_client(ssl, user, pass)) {
        goto cleanup;
    }


       //OPERAÇÃO
       
    if (strcmp(operation, "backup") == 0) {
        send_operation(ssl, "BACKUP");
        backup_file(ssl, filename);
    }
    else if (strcmp(operation, "restore") == 0) {
        send_operation(ssl, "RESTORE");
        restore_file(ssl, filename);
    }
    else if (strcmp(operation, "list") == 0) {
        send_operation(ssl, "LIST");
        list_files(ssl);
    }
    else {
        printf("Operação inválida: %s\n", operation);
    }

cleanup:
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    EVP_cleanup();

    return 0;
}
