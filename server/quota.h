#ifndef QUOTA_H
#define QUOTA_H


long get_user_quota(const char *user);


long get_user_usage(const char *user);


void ensure_user_dir(const char *user);

#endif
