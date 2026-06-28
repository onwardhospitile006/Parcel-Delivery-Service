#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/file_db.h"
#include "../include/sync.h"

int delete_customer_account(int cid) {

    Parcel p[100];
    int n = read_parcels(p);

    for (int i = 0; i < n; i++) {
        if (p[i].customer_id == cid &&
            strcmp(p[i].status, "Delivered") != 0) {

            printf("You have parcels pending delivery. Cannot delete account.\n");
            return 0;
        }
    }

    lock_user_file();
    lock_file();

    Parcel p2[100];
    int n2 = read_parcels(p2);

    for (int i = 0; i < n2; i++) {
        if (p2[i].customer_id == cid &&
            strcmp(p2[i].status, "Delivered") != 0) {

            printf("You have parcels pending delivery. Cannot delete account.\n");
            unlock_file();
            unlock_user_file();
            return 0;
        }
    }

    FILE *fp = fopen("data/users.txt", "r");
    FILE *temp = fopen("data/temp.txt", "w");

    User u;
    int found = 0;

    char id_str[20];
    while (fscanf(fp, "%19s %49s %49s %d",
        id_str, u.username, u.password, &u.role) == 4) {

        if (id_str[0] == 'A' || id_str[0] == 'C') {
            u.id = atoi(id_str + 1);
        } else {
            u.id = atoi(id_str);
        }

        if (u.id == cid && u.role == 2) {
            found = 1;
            continue;
        }

        fprintf(temp, "%c%d %s %s %d\n",
            (u.role == 1 ? 'A' : 'C'),
            u.id, u.username, u.password, u.role);
    }

    fclose(fp);
    fclose(temp);

    if (!found) {
        printf("Account not found.\n");
        remove("data/temp.txt");
        unlock_file();
        unlock_user_file();
        return 0;
    }

    remove("data/users.txt");
    rename("data/temp.txt", "data/users.txt");

    printf("Account deleted successfully.\n");

    unlock_file();
    unlock_user_file();

    return 1;
}

void customer_flow(int cid) {
    while (1) {
        printf("\n--- CUSTOMER ---\n");
        printf("1.View My Parcels\n");
        printf("2.Track Parcel by ID\n");
        printf("3.View Delivered Parcels\n");
        printf("4.Mark Parcel as Delivered\n");
        printf("5.Delete Account\n");
        printf("6.Back\n");

        printf("Enter your choice: ");
        int ch;
        scanf("%d", &ch);


        if (ch == 6) break;

        if (ch == 1) {
            Parcel p[100];
            int n = read_parcels(p);

            int found = 0;
            for (int i = 0; i < n; i++) {
                if (p[i].customer_id == cid) {
                    printf("ID:P%d Name:%s Status:%s Location:%s\n",
                        p[i].parcel_id,
                        p[i].name,
                        p[i].status,
                        p[i].location);
                    found = 1;
                }
            }

            if (!found) printf("No parcels are there for you.\n");
        }

        else if (ch == 2) {
            Parcel p[100];
            int n = read_parcels(p);

            int has_parcels = 0;
            printf("\n--- YOUR PARCELS ---\n");
            for (int i = 0; i < n; i++) {
                if (p[i].customer_id == cid) {
                    printf("ID:P%d Name:%s\n", p[i].parcel_id, p[i].name);
                    has_parcels = 1;
                }
            }

            if (!has_parcels) {
                printf("no parcels are there to change status\n");
                continue;
            }

            int id;
            printf("Enter parcel ID: ");
            id = read_id_input();

            int found = 0;
            for (int i = 0; i < n; i++) {
                if (p[i].parcel_id == id &&
                    p[i].customer_id == cid) {
                    printf("ID:P%d Name:%s Status:%s Location:%s\n",
                        p[i].parcel_id,
                        p[i].name,
                        p[i].status,
                        p[i].location);
                    found = 1;
                }
            }

            if (!found) printf("Parcel not found.\n");
        }

        else if (ch == 3) {
            Parcel p[100];
            int n = read_parcels(p);

            int found = 0;
            for (int i = 0; i < n; i++) {
                if (p[i].customer_id == cid &&
                    strcmp(p[i].status, "Delivered") == 0) {
                    printf("ID:P%d Name:%s Location:%s\n",
                        p[i].parcel_id,
                        p[i].name,
                        p[i].location);
                    found = 1;
                }
            }

            if (!found) printf("There are no delivered parcels.\n");
        }

        else if (ch == 4) {

            Parcel p[100];
            int n = read_parcels(p);

            int found = 0;

            for (int i = 0; i < n; i++) {
                if (p[i].customer_id == cid &&
                    p[i].location_index == 4 &&
                    strcmp(p[i].status, "InTransit") == 0) {

                    printf("ID:P%d Name:%s Ready for Delivery Confirmation\n",
                        p[i].parcel_id,
                        p[i].name);
                    found = 1;
                }
            }

            if (!found) {
                printf("No parcels ready to mark as delivered.\n");
                continue;
            }

            int pid;
            printf("Enter parcel ID: ");
            pid = read_id_input();

            lock_file();

            Parcel p2[100];
            int n2 = read_parcels(p2);

            int index = -1;

            for (int i = 0; i < n2; i++) {
                if (p2[i].parcel_id == pid &&
                    p2[i].customer_id == cid) {
                    index = i;
                    break;
                }
            }

            if (index == -1) {
                printf("Invalid parcel ID.\n");
                unlock_file();
                continue;
            }

            if (!(p2[index].location_index == 4 &&
                  strcmp(p2[index].status, "InTransit") == 0)) {
                printf("Parcel not ready for delivery confirmation.\n");
                unlock_file();
                continue;
            }

            strcpy(p2[index].status, "Delivered");

            write_parcels(p2, n2);

            printf("Parcel marked as Delivered.\n");

            unlock_file();
        }

        else if (ch == 5) {
            char confirm;
            printf("Are you sure? (y/n): ");
            scanf(" %c", &confirm);

            if (confirm == 'y') {
                int res = delete_customer_account(cid);
                if (res == 1) break;
            }
        }
    }
}                                                