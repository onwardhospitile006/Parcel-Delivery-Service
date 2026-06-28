#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "../include/auth.h"
#include "../include/flows.h"
#include "../include/ipc.h"
#include "../include/file_db.h"

void handle_sigint(int sig) {
    printf("\nCaught signal %d (Ctrl+C). Exiting gracefully...\n", sig);
    exit(0);
}

int main() {
    init_files();
    init_parcel_shm();
    atexit(cleanup_parcel_shm);
    signal(SIGINT, handle_sigint);
    
    while (1) {
        printf("\n--- MAIN PAGE ---\n");
        printf("1.Admin\n2.Agent\n3.Customer\n4.Exit\n");

        printf("Enter your choice: ");
        int ch;
        scanf("%d", &ch);

        if (ch == 4) break;

        User u;

        // ---------------- ADMIN ----------------
        if (ch == 1) {
            if (login(&u) && u.role == 0)
                dispatcher_flow();
            else
                printf("Invalid Admin Login\n");
        }

        // ---------------- AGENT ----------------
        else if (ch == 2) {
            if (login(&u) && u.role == 1)
                agent_flow(u.id);
            else
                printf("Invalid Agent Login\n");
        }

        // ---------------- CUSTOMER ----------------
        else if (ch == 3) {
            while (1) {
                printf("\n--- SIGNING OPTION ---\n");
                printf("1.Login\n2.Register\n3.Back\n");

                printf("Enter your choice: ");
                int c;
                scanf("%d", &c);

                if (c == 3) break;

                if (c == 2) {
                    register_customer();
                } else if (c == 1) {
                    if (login(&u) && u.role == 2)
                        customer_flow(u.id);
                    else
                        printf("Invalid Customer Login\n");
                }
            }
        }
    }
    return 0;
}