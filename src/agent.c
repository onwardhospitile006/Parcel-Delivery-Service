#include <stdio.h>
#include <string.h>
#include "../include/file_db.h"
#include "../include/sync.h"

void agent_flow(int agent_id) {
    while (1) {
        printf("\n--- AGENT PANEL ---\n");
        printf("1.View My Parcels\n");
        printf("2.Update Parcel Status\n");
        printf("3.Mark Location Update\n");
        printf("4.View Only Pending Deliveries\n");
        printf("5.Back\n");

        printf("Enter your choice: ");
        int ch;
        scanf("%d", &ch);

        if (ch == 5) break;

        if (ch == 1) {
            Parcel p[100];
            int n = read_parcels(p);

            int found = 0;
            for (int i = 0; i < n; i++) {
                if (p[i].assigned_agent == agent_id) {
                    printf("ID:P%d Name:%s Status:%s Location:%s\n",
                        p[i].parcel_id,
                        p[i].name,
                        p[i].status,
                        p[i].location);
                    found = 1;
                }
            }

            if (!found) printf("There are no parcels assigned to you.\n");
        }

        else if (ch == 2) {

            Parcel p[100];
            int n = read_parcels(p);

            int found = 0;
            for (int i = 0; i < n; i++) {
                if (p[i].assigned_agent == agent_id &&
                    strcmp(p[i].status, "Assigned") == 0) {
                    printf("ID:P%d Name:%s Status:%s\n",
                        p[i].parcel_id,
                        p[i].name,
                        p[i].status);
                    found = 1;
                }
            }

            if (!found) {
                printf("No parcels to update.\n");
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
                    p2[i].assigned_agent == agent_id) {
                    index = i;
                    break;
                }
            }

            if (index == -1) {
                printf("Invalid parcel ID.\n");
                unlock_file();
                continue;
            }

            if (strcmp(p2[index].status, "Assigned") != 0) {
                printf("Status can only move from Assigned to InTransit.\n");
                unlock_file();
                continue;
            }

            strcpy(p2[index].status, "InTransit");

            write_parcels(p2, n2);

            printf("Status updated to InTransit.\n");

            unlock_file();
        }

        else if (ch == 3) {

            Parcel p[100];
            int n = read_parcels(p);

            int found = 0;
            for (int i = 0; i < n; i++) {
                if (p[i].assigned_agent == agent_id &&
                    strcmp(p[i].status, "InTransit") == 0) {
                    printf("ID:P%d Name:%s Location:%s\n",
                        p[i].parcel_id,
                        p[i].name,
                        p[i].location);
                    found = 1;
                }
            }

            if (!found) {
                printf("No parcels available for location update.\n");
                continue;
            }

            int pid;
            printf("Enter parcel ID: ");
            pid = read_id_input();

            printf("\nLocation Options:\n");
            printf("1. Shipping\n");
            printf("2. Destination Warehouse\n");
            printf("3. Out For Delivery\n");
            printf("4. Destination Reached\n");

            int choice;
            printf("Enter your choice: ");
            scanf("%d", &choice);

            if (choice < 1 || choice > 4) {
                printf("Invalid choice.\n");
                continue;
            }

            lock_file();

            Parcel p2[100];
            int n2 = read_parcels(p2);

            int index = -1;
            for (int i = 0; i < n2; i++) {
                if (p2[i].parcel_id == pid &&
                    p2[i].assigned_agent == agent_id) {
                    index = i;
                    break;
                }
            }

            if (index == -1) {
                printf("Invalid parcel ID.\n");
                unlock_file();
                continue;
            }

            if (strcmp(p2[index].status, "InTransit") != 0) {
                printf("Set status to InTransit first.\n");
                unlock_file();
                continue;
            }

            if (choice != p2[index].location_index + 1) {
                printf("Update location step by step.\n");
                unlock_file();
                continue;
            }

            p2[index].location_index = choice;
            strcpy(p2[index].location, locations[choice]);

            write_parcels(p2, n2);

            printf("Location updated to %s\n", locations[choice]);

            unlock_file();
        }

        else if (ch == 4) {
            Parcel p[100];
            int n = read_parcels(p);

            int found = 0;
            for (int i = 0; i < n; i++) {
                if (p[i].assigned_agent == agent_id &&
                    strcmp(p[i].status, "Delivered") != 0) {
                    printf("ID:P%d Name:%s Status:%s\n",
                        p[i].parcel_id,
                        p[i].name,
                        p[i].status);
                    found = 1;
                }
            }

            if (!found) printf("No pending deliveries.\n");
        }
    }
}