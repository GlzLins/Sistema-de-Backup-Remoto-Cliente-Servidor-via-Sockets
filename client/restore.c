// Restauração de arquivos
#include "restore.h"
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 4096
#define FILENAME_MAX_LEN 256

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

static int ssl_write_exact(SSL *ssl, const void *buf, size_t n) {
    size_t total = 0;
    const unsigned char *p = (const unsigned char *)buf;

    while (total < n) {
        int w = SSL_write(ssl, p + total, (int)(n - total));
        if (w <= 0) return -1;
        total += (size_t)w;
    }
    return 0;
}

int restore_file(SSL *ssl, const char *filename) {

  
       //ENVIO DO FILENAME 

    char name[FILENAME_MAX_LEN] = {0};
    strncpy(name, filename, FILENAME_MAX_LEN - 1);

    if (ssl_write_exact(ssl, name, FILENAME_MAX_LEN) < 0)
        return 0;


    char response[16] = {0};
    if (SSL_read(ssl, response, sizeof(response) - 1) <= 0)
        return 0;

    if (strcmp(response, "FOUND") != 0) {
        printf("Arquivo não encontrado no servidor\n");
        return 0;
    }

       //TAMANHO DO ARQUIVO
     

    long filesize = 0;
    if (ssl_read_exact(ssl, &filesize, sizeof(filesize)) < 0)
        return 0;

    /* confirma pronto */
    SSL_write(ssl, "READY", 5);

   
       // RECEBIMENTO DO ARQUIVO
       
    FILE *out = fopen(filename, "wb");
    if (!out) {
        perror("Erro ao criar arquivo");
        return 0;
    }

    char buffer[BUFFER_SIZE];
    long received = 0;

    while (received < filesize) {
        int bytes = SSL_read(ssl, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;

        fwrite(buffer, 1, bytes, out);
        received += bytes;
    }

    fclose(out);

    if (received == filesize) {
        printf("Restauração concluído com sucesso! (%ld bytes)\n", filesize);
        return 1;
    }

    printf("Erro durante a restauração \n");
    return 0;
}
