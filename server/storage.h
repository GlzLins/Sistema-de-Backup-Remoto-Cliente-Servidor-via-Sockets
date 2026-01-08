#ifndef STORAGE_H
#define STORAGE_H

#include <openssl/ssl.h>
void ensure_user_dir(const char *user);
void handle_backup(SSL *ssl, const char *user);
void handle_restore(SSL *ssl, const char *user);
void handle_list(SSL *ssl, const char *user);

#endif
