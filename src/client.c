#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/network.h"

int sockfd;

// Helper to strip prefixes like 'A', 'C', 'P' from IDs
int read_id_input() {
    char str[20];
    scanf("%s", str);
    int i = 0;
    while (str[i] != '\0' && (str[i] < '0' || str[i] > '9')) {
        i++;
    }
    return atoi(&str[i]);
}

void send_and_print(const char *cmd) {
    send_msg(sockfd, cmd);

    char buf[BUFFER_SIZE];
    int n = recv_msg(sockfd, buf, sizeof(buf));
    if (n > 0) {
        buf[strcspn(buf, "\r\n")] = '\0';
        if (strncmp(buf, "ERROR ", 6) == 0) {
            printf("%s\n", buf + 6);
        } else if (strncmp(buf, "OK ", 3) == 0) {
            printf("%s\n", buf + 3);
        } else {
            printf("%s\n", buf);
        }
    }
}

int send_and_print_multi(const char *cmd) {
    send_msg(sockfd, cmd);

    char buf[BUFFER_SIZE];
    recv_multiline(sockfd, buf, sizeof(buf));
    printf("%s", buf);
    
    // Return 0 if the list is empty (contains "No ")
    if (strstr(buf, "No ") != NULL) return 0;
    return 1;
}

void admin_panel() {
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

        if (ch == 8) {
            send_msg(sockfd, "LOGOUT\n");
            char buf[BUFFER_SIZE];
            recv_msg(sockfd, buf, sizeof(buf));
            break;
        }

        if (ch == 1) {
            char uname[50], pass[50];
            printf("Enter agent username: ");
            scanf("%s", uname);
            printf("Enter agent password: ");
            scanf("%s", pass);

            char cmd[BUFFER_SIZE];
            snprintf(cmd, sizeof(cmd), "REGISTER_AGENT %s %s\n", uname, pass);
            send_and_print(cmd);
        }

        else if (ch == 2) {
            printf("\n--- AGENT LIST ---\n");
            if (send_and_print_multi("LIST_AGENTS\n") == 0) {
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

            char cmd[BUFFER_SIZE];
            snprintf(cmd, sizeof(cmd), "REMOVE_AGENT %d\n", id);
            send_and_print(cmd);
        }

        else if (ch == 3) {
            printf("\n--- CUSTOMER LIST ---\n");
            if (send_and_print_multi("LIST_CUSTOMERS\n") == 0) {
                continue;
            }

            char pname[50];
            printf("\nEnter parcel name: ");
            scanf("%s", pname);

            int cid;
            printf("Enter customer ID: ");
            cid = read_id_input();

            char cmd[BUFFER_SIZE];
            snprintf(cmd, sizeof(cmd), "ADD_PARCEL %s %d\n", pname, cid);
            send_and_print(cmd);
        }

        else if (ch == 4) {
            printf("\n--- UNASSIGNED PARCELS ---\n");
            if (send_and_print_multi("LIST_UNASSIGNED\n") == 0) {
                continue;
            }

            int pid;
            printf("Enter parcel ID: ");
            pid = read_id_input();

            printf("\n--- AGENT LIST ---\n");
            if (send_and_print_multi("LIST_AGENTS\n") == 0) {
                continue;
            }

            int aid;
            printf("Enter agent ID: ");
            aid = read_id_input();

            char cmd[BUFFER_SIZE];
            snprintf(cmd, sizeof(cmd), "ASSIGN_PARCEL %d %d\n", pid, aid);
            send_and_print(cmd);
        }

        else if (ch == 5) {
            printf("\n--- ALL PARCELS ---\n");
            send_and_print_multi("VIEW_PARCELS\n");
        }

        else if (ch == 6) {
            printf("\n--- UNASSIGNED PARCELS ---\n");
            if (send_and_print_multi("LIST_UNASSIGNED\n") == 0) {
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

            char cmd[BUFFER_SIZE];
            snprintf(cmd, sizeof(cmd), "DELETE_PARCEL %d\n", id);
            send_and_print(cmd);
        }
 PER AGENT ---\n");
            send_and_print_multi("AGENT_COUNT\n");
        }
    }
}

void agent_panel() {
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

        if (ch == 5) {
            send_msg(sockfd, "LOGOUT\n");
            char buf[BUFFER_SIZE];
            recv_msg(sockfd, buf, sizeof(buf));
            break;
        }

        if (ch == 1) {
            printf("\n--- MY PARCELS ---\n");
            send_and_print_multi("VIEW_MY_PARCELS\n");
        }

        else if (ch == 2) {
            printf("\n--- MY PARCELS ---\n");
            if (send_and_print_multi("VIEW_MY_PARCELS\n") == 0) {
                continue;
            }

            int pid;
            printf("Enter parcel ID: ");
            pid = read_id_input();

            char cmd[BUFFER_SIZE];
            snprintf(cmd, sizeof(cmd), "UPDATE_STATUS %d\n", pid);
            send_and_print(cmd);
        }

        else if (ch == 3) {
            printf("\n--- MY PARCELS ---\n");
            if (send_and_print_multi("VIEW_MY_PARCELS\n") == 0) {
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

            int loc;
            printf("Enter your choice: ");
            scanf("%d", &loc);

            char cmd[BUFFER_SIZE];
            snprintf(cmd, sizeof(cmd), "UPDATE_LOCATION %d %d\n", pid, loc);
            send_and_print(cmd);
        }

        else if (ch == 4) {
            printf("\n--- PENDING DELIVERIES ---\n");
            send_and_print_multi("PENDING_DELIVERIES\n");
        }
    }
}

void customer_panel() {
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

        if (ch == 6) {
            send_msg(sockfd, "LOGOUT\n");
            char buf[BUFFER_SIZE];
            recv_msg(sockfd, buf, sizeof(buf));
            break;
        }

        if (ch == 1) {
            printf("\n--- MY PARCELS ---\n");
            send_and_print_multi("VIEW_MY_PARCELS\n");
        }

        else if (ch == 2) {
            printf("\n--- MY PARCELS ---\n");
            if (send_and_print_multi("VIEW_MY_PARCELS_BRIEF\n") == 0) {
                continue;
            }

            int pid;
            printf("Enter parcel ID: ");
            pid = read_id_input();

            char cmd[BUFFER_SIZE];
            snprintf(cmd, sizeof(cmd), "TRACK_PARCEL %d\n", pid);
            send_and_print(cmd);
        }

        else if (ch == 3) {
            printf("\n--- DELIVERED PARCELS ---\n");
            send_and_print_multi("VIEW_DELIVERED\n");
        }

        else if (ch == 4) {
            printf("\n--- READY FOR DELIVERY ---\n");
            if (send_and_print_multi("VIEW_READY_DELIVERY\n") == 0) {
                continue;
            }

            int pid;
            printf("Enter parcel ID: ");
            pid = read_id_input();

            // Show details first
            char track_cmd[BUFFER_SIZE];
            snprintf(track_cmd, sizeof(track_cmd), "TRACK_PARCEL %d\n", pid);
            send_and_print(track_cmd);

            // Then mark as delivered
            char cmd[BUFFER_SIZE];
            snprintf(cmd, sizeof(cmd), "MARK_DELIVERED %d\n", pid);
            send_and_print(cmd);
        }

        else if (ch == 5) {
            char confirm;
            printf("Are you sure? (y/n): ");
            scanf(" %c", &confirm);

            if (confirm == 'y') {
                send_msg(sockfd, "DELETE_ACCOUNT\n");
                char buf[BUFFER_SIZE];
                int n = recv_msg(sockfd, buf, sizeof(buf));
                if (n > 0) {
                    buf[strcspn(buf, "\r\n")] = '\0';
                    if (strncmp(buf, "ERROR ", 6) == 0) {
                        printf("%s\n", buf + 6);
                    } else if (strncmp(buf, "OK ", 3) == 0) {
                        printf("%s\n", buf + 3);
                        break;
                    } else {
                        printf("%s\n", buf);
                    }
                }
            }
        }
    }
}

int do_login(int expected_role) {
    char uname[50], pass[50];
    printf("Username: ");
    scanf("%s", uname);
    printf("Password: ");
    scanf("%s", pass);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "LOGIN %s %s\n", uname, pass);
    send_msg(sockfd, cmd);

    char buf[BUFFER_SIZE];
    int n = recv_msg(sockfd, buf, sizeof(buf));
    if (n <= 0) return 0;

    buf[strcspn(buf, "\r\n")] = '\0';

    int role, uid;
    if (sscanf(buf, "OK %d %d", &role, &uid) == 2) {
        if (role == expected_role) return 1;
        // Wrong role, logout
        send_msg(sockfd, "LOGOUT\n");
        recv_msg(sockfd, buf, sizeof(buf));
        return 0;
    }
    return 0;
}

int main() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        printf("Could not connect to server. Is the server running?\n");
        exit(1);
    }

    printf("Connected to server.\n");

    while (1) {
        printf("\n--- MAIN PAGE ---\n");
        printf("1.Admin\n2.Agent\n3.Customer\n4.Exit\n");

        printf("Enter your choice: ");
        int ch;
        scanf("%d", &ch);

        if (ch == 4) {
            send_msg(sockfd, "QUIT\n");
            char buf[BUFFER_SIZE];
            recv_msg(sockfd, buf, sizeof(buf));
            break;
        }

        if (ch == 1) {
            if (do_login(0))
                admin_panel();
            else
                printf("Invalid Admin Login\n");
        }

        else if (ch == 2) {
            if (do_login(1))
                agent_panel();
            else
                printf("Invalid Agent Login\n");
        }

        else if (ch == 3) {
            while (1) {
                printf("\n--- SIGNING OPTION ---\n");
                printf("1.Login\n2.Register\n3.Back\n");

                printf("Enter your choice: ");
                int c;
                scanf("%d", &c);

                if (c == 3) break;

                if (c == 2) {
                    char uname[50], pass[50];
                    printf("Enter username: ");
                    scanf("%s", uname);
                    printf("Enter password: ");
                    scanf("%s", pass);

                    char cmd[BUFFER_SIZE];
                    snprintf(cmd, sizeof(cmd), "REGISTER_CUSTOMER %s %s\n", uname, pass);
                    send_and_print(cmd);

                } else if (c == 1) {
                    if (do_login(2))
                        customer_panel();
                    else
                        printf("Invalid Customer Login\n");
                }
            }
        }
    }

    close(sockfd);
    printf("Disconnected.\n");
    return 0;
}
