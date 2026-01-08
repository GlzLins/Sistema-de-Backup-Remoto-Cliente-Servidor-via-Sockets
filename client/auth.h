#ifndef AUTH_H
#define AUTH_H

#include <openssl/ssl.h>

int auth_client(SSL *ssl, const char *user, const char *pass);

#endif
