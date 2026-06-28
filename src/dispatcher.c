#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/file_db.h"
#include "../include/auth.h"
#include "../include/sync.h"

void dispatcher_flow() {
    while (1) {
        printf("\n--- ADMIN PANEL ---\n");
        printf("1.Register Agent\n");
        printf("2.Remove Agent\n");
        printf("3.Add Parcel\n");
        printf("4.Assign Parcel\n");
        printf("5.View Parcels\n");
        printf("6.Delete Parcels\n");
        printf("7.Show Parcel Count Per Agent\n");
        printf("8.Back\n");

        printf("Enter your choice: ");
        int ch;
        scanf("%d", &ch);

        if (ch == 8) break;

        if (ch == 1) {
            register_agent();
        }

        else if (ch == 2) {

            User users[100];
            int n = read_users(users);

            int found_agents = 0;

            printf("\n--- AGENT LIST ---\n");
            for (int i = 0; i < n; i++) {
                if (users[i].role == 1) {
                    printf("ID: A%d Username: %s\n",
                        users[i].id, users[i].username);
                    found_agents = 1;
                }
            }

            if (!found_agents) {
                printf("No agents found.\n");
                continue;
            }

            int id;
            printf("Enter agent ID to remove: ");
            id = read_id_input();

            char confirm;
            printf("Are you sure? (y/n): ");
            scanf(" %c", &confirm);

            if (confirm != 'y') {
                printf("Operation cancelled.\n");
                continue;
            }

            lock_user_file();
            lock_file();

            Parcel p[100];
            int pn = read_parcels(p);

            int has_parcels = 0;
            for (int i = 0; i < pn; i++) {
                if (p[i].assigned_agent == id && strcmp(p[i].status, "Delivered") != 0) {
                    has_parcels = 1;
                    break;
                }
            }

            if (has_parcels) {
                printf("Agent has assigned parcels. Cannot remove.\n");
                unlock_file();
                unlock_user_file();
                continue;
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

                if (u.id == id && u.role == 1) {
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
                printf("Agent ID not found.\n");
                remove("data/temp.txt");
                unlock_file();
                unlock_user_file();
                continue;
            }

            remove("data/users.txt");
            rename("data/temp.txt", "data/users.txt");

            printf("Agent removed successfully.\n");

            unlock_file();
            unlock_user_file();
        }

        else if (ch == 3) {

            User users[100];
            int un = read_users(users);

            int has_customer = 0;

            printf("\n--- CUSTOMER LIST ---\n");
            for (int i = 0; i < un; i++) {
                if (users[i].role == 2) {
                    printf("ID: C%d Username: %s\n",
                        users[i].id, users[i].username);
                    has_customer = 1;
                }
            }

            if (!has_customer) {
                printf("No customers found.\n");
                continue;
            }

            char pname[50];
            printf("\nEnter parcel name: ");
            scanf("%s", pname);

            int cid;
            printf("Enter customer ID: ");
            cid = read_id_input();

            int valid = 0;
            for (int i = 0; i < un; i++) {
                if (users[i].id == cid && users[i].role == 2) {
                    valid = 1;
                    break;
                }
            }

            if (!valid) {
                printf("Invalid customer ID.\n");
                continue;
            }

            lock_file();

            Parcel p[100];
            int n = read_parcels(p);

            int new_id = generate_parcel_id(p, n);

            strcpy(p[n].name, pname);
            p[n].parcel_id = new_id;
            p[n].assigned_agent = -1;
            p[n].customer_id = cid;
            p[n].location_index = 0;
            strcpy(p[n].location, locations[0]);
            strcpy(p[n].status, "Unassigned");
            write_parcels(p, n + 1);

            printf("Parcel added with ID: P%d\n", new_id);

            unlock_file();
        }

        else if (ch == 4) {

            Parcel p[100];
            int n = read_parcels(p);

            int found_unassigned = 0;

            printf("\n--- UNASSIGNED PARCELS ---\n");
            for (int i = 0; i < n; i++) {
                if (p[i].assigned_agent == -1) {
                    printf("ID: P%d Name: %s\n", p[i].parcel_id, p[i].name);
                    found_unassigned = 1;
                }
            }

            if (!found_unassigned) {
                printf("No unassigned parcels found.\n");
                continue;
            }

            int pid;
            printf("Enter parcel ID: ");
            pid = read_id_input();

            User users[100];
            int un = read_users(users);

            int has_agent = 0;

            printf("\n--- AGENT LIST ---\n");
            for (int i = 0; i < un; i++) {
                if (users[i].role == 1) {
                    printf("ID: A%d Username: %s\n",
                        users[i].id, users[i].username);
                    has_agent = 1;
                }
            }

            if (!has_agent) {
                printf("No agents found.\n");
                continue;
            }

            int aid;
            printf("Enter agent ID: ");
            aid = read_id_input();

            int agent_valid = 0;
            for (int i = 0; i < un; i++) {
                if (users[i].id == aid && users[i].role == 1) {
                    agent_valid = 1;
                    break;
                }
            }

            if (!agent_valid) {
                printf("Invalid Agent ID.\n");
                continue;
            }

            lock_file();

            Parcel p2[100];
            int n2 = read_parcels(p2);

            int idx = -1;
            for (int i = 0; i < n2; i++) {
                if (p2[i].parcel_id == pid) {
                    idx = i;
                    break;
                }
            }

            if (idx == -1) {
                printf("Parcel not found.\n");
                unlock_file();
                continue;
            }

            if (p2[idx].assigned_agent != -1) {
                printf("Parcel already assigned.\n");
                unlock_file();
                continue;
            }

            p2[idx].assigned_agent = aid;
            strcpy(p2[idx].status, "Assigned");

            write_parcels(p2, n2);

            printf("Parcel assigned successfully.\n");

            unlock_file();
        }

        else if (ch == 5) {
            Parcel p[100];
            int n = read_parcels(p);

            if (n == 0) {
                printf("No parcels found.\n");
            } else {
                for (int i = 0; i < n; i++) {
                    if (p[i].assigned_agent == -1) {
                        printf("P%d A-1 C%d %s %s %s\n",
                            p[i].parcel_id,
                            p[i].customer_id,
                            p[i].name,
                            p[i].status,
                            p[i].location);
                    } else {
                        printf("P%d A%d C%d %s %s %s\n",
                            p[i].parcel_id,
                            p[i].assigned_agent,
                            p[i].customer_id,
                            p[i].name,
                            p[i].status,
                            p[i].location);
                    }
                }
            }
        }

        else if (ch == 6) {

            Parcel p[100];
            int n = read_parcels(p);

            int found = 0;

            printf("\n--- UNASSIGNED PARCELS ---\n");
            for (int i = 0; i < n; i++) {
                if (p[i].assigned_agent == -1) {
                    printf("P%d %s\n", p[i].parcel_id, p[i].name);
                    found = 1;
                }
            }

            if (!found) {
                printf("No unassigned parcels found.\n");
                continue;
            }

            int id;
            printf("Enter parcel id to delete: ");
            id = read_id_input();

            char confirm;
            printf("Are you sure? (y/n): ");
            scanf(" %c", &confirm);

            if (confirm != 'y') {
                printf("Cancelled.\n");
                continue;
            }

            lock_file();

            Parcel p2[100];
            int n2 = read_parcels(p2);

            Parcel newList[100];
            int j = 0;
            int deleted = 0;

            for (int i = 0; i < n2; i++) {
                if (p2[i].parcel_id == id && p2[i].assigned_agent == -1) {
                    deleted = 1;
                    continue;
                }
                newList[j++] = p2[i];
            }

            if (!deleted) {
                printf("Parcel not found or already assigned.\n");
                unlock_file();
                continue;
            }

            write_parcels(newList, j);
            printf("Parcel deleted successfully.\n");

            unlock_file();
        }

        else if (ch == 7) {
            User users[100];
            int un = read_users(users);

            Parcel p[100];
            int pn = read_parcels(p);

            int found = 0;

            printf("\n--- PARCEL COUNT PER AGENT ---\n");

            for (int i = 0; i < un; i++) {
                if (users[i].role == 1) {
                    int count = 0;

                    for (int j = 0; j < pn; j++) {
                        if (p[j].assigned_agent == users[i].id) {
                            count++;
                        }
                    }

                    printf("Agent ID: A%d Username: %s -> Parcels: %d\n",
                        users[i].id,
                        users[i].username,
                        count);

                    found = 1;
                }
            }

            if (!found) {
                printf("No agents available.\n");
            }
        }
    }
}