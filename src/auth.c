#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "../include/auth.h"
#include "../include/file_db.h"

#include "../include/sync.h"

int login(User *u) {
    char uname[50], pass[50];

    printf("Username: ");
    scanf("%s", uname);

    printf("Password: ");
    scanf("%s", pass);

    if (strcmp(uname, "admin") == 0 && strcmp(pass, "admin") == 0) {
        u->id = 1;
        strcpy(u->username, "admin");
        strcpy(u->password, "admin");
        u->role = 0;
        return 1;
    }

    User users[100];
    
    int n = read_users(users);

    for (int i = 0; i < n; i++) {
        if (!strcmp(users[i].username, uname) &&
            !strcmp(users[i].password, pass)) {

            *u = users[i];
            return 1;
        }
    }

    return 0;
}

void register_customer() {
    User users[100];
    
    User u;

    printf("Enter username: ");
    scanf("%s", u.username);

    printf("Enter password: ");
    scanf("%s", u.password);

    lock_user_file();
    int n = read_users(users);

    for (int i = 0; i < n; i++) {
        if (strcmp(users[i].username, u.username) == 0) {
            printf("Username already exists. Registration failed.\n");
            unlock_user_file();
            return;
        }
    }

    FILE *fp = fopen("data/users.txt", "a");
    int new_id = generate_user_id(users, n, 2);

    u.id = new_id;
    u.role = 2;

    fprintf(fp, "C%d %s %s %d\n",
        u.id, u.username, u.password, u.role);
    
    fclose(fp);
    unlock_user_file();

    printf("Customer registered with ID: C%d\n", new_id);
}

void register_agent() {
    User users[100];
    
    User u;

    printf("Enter agent username: ");
    scanf("%s", u.username);

    printf("Enter agent password: ");
    scanf("%s", u.password);

    lock_user_file();
    int n = read_users(users);

    for (int i = 0; i < n; i++) {
        if (strcmp(users[i].username, u.username) == 0) {
            printf("Username already exists. Registration failed.\n");
            unlock_user_file();
            return;
        }
    }

    FILE *fp = fopen("data/users.txt", "a");
    int new_id = generate_user_id(users, n, 1);
    
    u.id = new_id;
    u.role = 1;
    
    fprintf(fp, "A%d %s %s %d\n",
        u.id, u.username, u.password, u.role);

    fclose(fp);
    unlock_user_file();
    
    printf("Agent registered with ID: A%d\n", new_id);
}

int login_with_creds(User *u, const char *uname, const char *pass) {
    if (strcmp(uname, "admin") == 0 && strcmp(pass, "admin") == 0) {
        u->id = 1;
        strcpy(u->username, "admin");
        strcpy(u->password, "admin");
        u->role = 0;
        return 1;
    }

    User users[100];
    int n = read_users(users);

    for (int i = 0; i < n; i++) {
        if (!strcmp(users[i].username, uname) &&
            !strcmp(users[i].password, pass)) {
            *u = users[i];
            return 1;
        }
    }

    return 0;
}

int register_customer_with_creds(const char *uname, const char *pass) {
    User users[100];

    User u;
    strcpy(u.username, uname);
    strcpy(u.password, pass);

    lock_user_file();
    int n = read_users(users);

    for (int i = 0; i < n; i++) {
        if (strcmp(users[i].username, uname) == 0) {
            unlock_user_file();
            return -1;
        }
    }

    FILE *fp = fopen("data/users.txt", "a");
    int new_id = generate_user_id(users, n, 2);

    u.id = new_id;
    u.role = 2;

    fprintf(fp, "C%d %s %s %d\n",
        u.id, u.username, u.password, u.role);

    fclose(fp);
    unlock_user_file();

    return new_id;
}

int register_agent_with_creds(const char *uname, const char *pass) {
    User users[100];

    User u;
    strcpy(u.username, uname);
    strcpy(u.password, pass);

    lock_user_file();
    int n = read_users(users);

    for (int i = 0; i < n; i++) {
        if (strcmp(users[i].username, uname) == 0) {
            unlock_user_file();
            return -1;
        }
    }

    FILE *fp = fopen("data/users.txt", "a");
    int new_id = generate_user_id(users, n, 1);

    u.id = new_id;
    u.role = 1;

    fprintf(fp, "A%d %s %s %d\n",
        u.id, u.username, u.password, u.role);

    fclose(fp);
    unlock_user_file();

    return new_id;
}