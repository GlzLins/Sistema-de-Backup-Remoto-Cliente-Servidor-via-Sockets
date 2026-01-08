#ifndef SERVER_H
#define SERVER_H

#include <openssl/ssl.h>


void init_openssl();
void cleanup_openssl();
SSL_CTX *create_context();
void configure_context(SSL_CTX *ctx);

int authenticate_user(const char *user, const char *pass);

#endif
