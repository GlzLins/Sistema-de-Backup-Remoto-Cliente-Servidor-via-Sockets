#ifndef CLIENT_BACKUP_H
#define CLIENT_BACKUP_H

#include <openssl/ssl.h>

int backup_file(SSL *ssl, const char *filepath);

#endif
