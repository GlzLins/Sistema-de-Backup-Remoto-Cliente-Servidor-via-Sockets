#ifndef CLIENT_RESTORE_H
#define CLIENT_RESTORE_H

#include <openssl/ssl.h>

int restore_file(SSL *ssl, const char *filename);

#endif
