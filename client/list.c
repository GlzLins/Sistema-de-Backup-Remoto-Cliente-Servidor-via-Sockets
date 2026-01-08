#include <stdio.h>
#include <string.h>
#include <openssl/ssl.h>

#define BUFFER_SIZE 256

int list_files(SSL *ssl) {
    long quota = 0, usage = 0;
    int count = 0;

    SSL_read(ssl, &quota, sizeof(quota));
    SSL_read(ssl, &usage, sizeof(usage));

    printf("\nUso: %ld / %ld bytes\n\n", usage, quota);
    printf("Arquivos disponíveis:\n");

    SSL_read(ssl, &count, sizeof(count));

    for (int i = 0; i < count; i++) {
      char name[BUFFER_SIZE] = {0};
        long size = 0;

        SSL_read(ssl, name, BUFFER_SIZE);
        SSL_read(ssl, &size, sizeof(size));

        name[BUFFER_SIZE- 1] = '\0';

        printf(" - %s (%ld bytes)\n", name, size);

    }

    return 1;
}