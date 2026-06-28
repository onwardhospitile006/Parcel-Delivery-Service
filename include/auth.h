#ifndef AUTH_H
#define AUTH_H

#include "structures.h"

int login(User *u);
void register_customer();
void register_agent();

// Server-side variants that accept parameters instead of reading stdin
int login_with_creds(User *u, const char *uname, const char *pass);
int register_customer_with_creds(const char *uname, const char *pass);
int register_agent_with_creds(const char *uname, const char *pass);

#endif