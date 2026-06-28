#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "../include/network.h"
#include "../include/structures.h"
#include "../include/auth.h"
#include "../include/file_db.h"
#include "../include/ipc.h"
#include "../include/sync.h"

int server_fd = -1;

void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void sigint_handler(int sig) {
    (void)sig;
    printf("\n[SERVER] Shutting down...\n");
    if (server_fd != -1) close(server_fd);
    exit(0);
}

void handle_login(int cfd, char *args, int *role, int *uid) {
    char uname[50], pass[50];
    if (sscanf(args, "%49s %49s", uname, pass) != 2) {
        send_msg(cfd, "ERROR Invalid arguments\n");
        return;
    }
    User u;
    if (login_with_creds(&u, uname, pass)) {
        *role = u.role;
        *uid = u.id;
        char resp[BUFFER_SIZE];
        snprintf(resp, sizeof(resp), "OK %d %d\n", u.role, u.id);
        send_msg(cfd, resp);
    } else {
        send_msg(cfd, "ERROR Invalid credentials\n");
    }
}

void handle_register_customer(int cfd, char *args) {
    char uname[50], pass[50];
    if (sscanf(args, "%49s %49s", uname, pass) != 2) {
        send_msg(cfd, "ERROR Invalid arguments\n");
        return;
    }
    int new_id = register_customer_with_creds(uname, pass);
    if (new_id == -1) {
        send_msg(cfd, "ERROR Username already exists.\n");
        return;
    }
    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp), "OK Customer registered with ID: C%d\n", new_id);
    send_msg(cfd, resp);
}

void handle_register_agent(int cfd, char *args) {
    char uname[50], pass[50];
    if (sscanf(args, "%49s %49s", uname, pass) != 2) {
        send_msg(cfd, "ERROR Invalid arguments\n");
        return;
    }
    int new_id = register_agent_with_creds(uname, pass);
    if (new_id == -1) {
        send_msg(cfd, "ERROR Username already exists.\n");
        return;
    }
    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp), "OK Agent registered with ID: A%d\n", new_id);
    send_msg(cfd, resp);
}

void handle_list_agents(int cfd) {
    User users[100];
    int n = read_users(users);
    char resp[BUFFER_SIZE] = "";
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (users[i].role == 1) {
            char line[128];
            snprintf(line, sizeof(line), "ID: A%d Username: %s\n", users[i].id, users[i].username);
            strcat(resp, line);
            found = 1;
        }
    }
    if (!found) strcat(resp, "No agents found.\n");
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_list_customers(int cfd) {
    User users[100];
    int n = read_users(users);
    char resp[BUFFER_SIZE] = "";
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (users[i].role == 2) {
            char line[128];
            snprintf(line, sizeof(line), "ID: C%d Username: %s\n", users[i].id, users[i].username);
            strcat(resp, line);
            found = 1;
        }
    }
    if (!found) strcat(resp, "No customers found.\n");
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_remove_agent(int cfd, char *args) {
    int id;
    if (sscanf(args, "%d", &id) != 1) {
        send_msg(cfd, "ERROR Invalid agent ID\n");
        return;
    }

    lock_user_file();
    lock_file();

    Parcel p[100];
    int pn = read_parcels(p);
    for (int i = 0; i < pn; i++) {
        if (p[i].assigned_agent == id && strcmp(p[i].status, "Delivered") != 0) {
            send_msg(cfd, "ERROR Agent has assigned parcels. Cannot remove.\n");
            unlock_file();
            unlock_user_file();
            return;
        }
    }

    FILE *fp = fopen("data/users.txt", "r");
    FILE *temp = fopen("data/temp.txt", "w");
    User u;
    int found = 0;
    char id_str[20];

    while (fscanf(fp, "%19s %49s %49s %d", id_str, u.username, u.password, &u.role) == 4) {
        if (id_str[0] == 'A' || id_str[0] == 'C')
            u.id = atoi(id_str + 1);
        else
            u.id = atoi(id_str);

        if (u.id == id && u.role == 1) { found = 1; continue; }
        fprintf(temp, "%c%d %s %s %d\n", (u.role == 1 ? 'A' : 'C'), u.id, u.username, u.password, u.role);
    }
    fclose(fp);
    fclose(temp);

    if (!found) {
        remove("data/temp.txt");
        send_msg(cfd, "ERROR Agent ID not found.\n");
    } else {
        remove("data/users.txt");
        rename("data/temp.txt", "data/users.txt");
        send_msg(cfd, "OK Agent removed successfully.\n");
    }
    unlock_file();
    unlock_user_file();
}

void handle_add_parcel(int cfd, char *args) {
    char pname[50];
    int cid;
    if (sscanf(args, "%49s %d", pname, &cid) != 2) {
        send_msg(cfd, "ERROR Invalid arguments\n");
        return;
    }

    User users[100];
    int un = read_users(users);
    int valid = 0;
    for (int i = 0; i < un; i++) {
        if (users[i].id == cid && users[i].role == 2) { valid = 1; break; }
    }
    if (!valid) {
        send_msg(cfd, "ERROR Invalid customer ID.\n");
        return;
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
    unlock_file();

    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp), "OK Parcel added with ID: P%d\n", new_id);
    send_msg(cfd, resp);
}

void handle_assign_parcel(int cfd, char *args) {
    int pid, aid;
    if (sscanf(args, "%d %d", &pid, &aid) != 2) {
        send_msg(cfd, "ERROR Invalid arguments\n");
        return;
    }

    User users[100];
    int un = read_users(users);
    int agent_valid = 0;
    for (int i = 0; i < un; i++) {
        if (users[i].id == aid && users[i].role == 1) { agent_valid = 1; break; }
    }
    if (!agent_valid) {
        send_msg(cfd, "ERROR Invalid Agent ID.\n");
        return;
    }

    lock_file();
    Parcel p[100];
    int n = read_parcels(p);
    int idx = -1;
    for (int i = 0; i < n; i++) {
        if (p[i].parcel_id == pid) { idx = i; break; }
    }
    if (idx == -1) {
        send_msg(cfd, "ERROR Parcel not found.\n");
        unlock_file();
        return;
    }
    if (p[idx].assigned_agent != -1) {
        send_msg(cfd, "ERROR Parcel already assigned.\n");
        unlock_file();
        return;
    }
    p[idx].assigned_agent = aid;
    strcpy(p[idx].status, "Assigned");
    write_parcels(p, n);
    unlock_file();

    send_msg(cfd, "OK Parcel assigned successfully.\n");
}

void handle_list_unassigned(int cfd) {
    Parcel p[100];
    int n = read_parcels(p);
    char resp[BUFFER_SIZE] = "";
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (p[i].assigned_agent == -1) {
            char line[256];
            snprintf(line, sizeof(line), "ID: P%d Name: %s\n", p[i].parcel_id, p[i].name);
            strcat(resp, line);
            found = 1;
        }
    }
    if (!found) strcat(resp, "No unassigned parcels found.\n");
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_view_parcels(int cfd) {
    Parcel p[100];
    int n = read_parcels(p);
    char resp[BUFFER_SIZE] = "";
    if (n == 0) {
        strcat(resp, "No parcels found.\n");
    } else {
        for (int i = 0; i < n; i++) {
            char line[256];
            if (p[i].assigned_agent == -1)
                snprintf(line, sizeof(line), "P%d A-1 C%d %s %s %s\n", p[i].parcel_id, p[i].customer_id, p[i].name, p[i].status, p[i].location);
            else
                snprintf(line, sizeof(line), "P%d A%d C%d %s %s %s\n", p[i].parcel_id, p[i].assigned_agent, p[i].customer_id, p[i].name, p[i].status, p[i].location);
            strcat(resp, line);
        }
    }
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_delete_parcel(int cfd, char *args) {
    int id;
    if (sscanf(args, "%d", &id) != 1) {
        send_msg(cfd, "ERROR Invalid parcel ID\n");
        return;
    }

    lock_file();
    Parcel p[100];
    int n = read_parcels(p);
    Parcel newList[100];
    int j = 0, deleted = 0;

    for (int i = 0; i < n; i++) {
        if (p[i].parcel_id == id && p[i].assigned_agent == -1) {
            deleted = 1;
            continue;
        }
        newList[j++] = p[i];
    }

    if (!deleted) {
        send_msg(cfd, "ERROR Parcel not found or already assigned.\n");
        unlock_file();
        return;
    }
    write_parcels(newList, j);
    unlock_file();
    send_msg(cfd, "OK Parcel deleted successfully.\n");
}

void handle_agent_count(int cfd) {
    User users[100];
    int un = read_users(users);
    Parcel p[100];
    int pn = read_parcels(p);
    char resp[BUFFER_SIZE] = "";
    int found = 0;

    for (int i = 0; i < un; i++) {
        if (users[i].role == 1) {
            int count = 0;
            for (int j = 0; j < pn; j++) {
                if (p[j].assigned_agent == users[i].id) count++;
            }
            char line[128];
            snprintf(line, sizeof(line), "Agent ID: A%d Username: %s -> Parcels: %d\n", users[i].id, users[i].username, count);
            strcat(resp, line);
            found = 1;
        }
    }
    if (!found) strcat(resp, "No agents available.\n");
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_view_my_parcels(int cfd, int role, int uid) {
    Parcel p[100];
    int n = read_parcels(p);
    char resp[BUFFER_SIZE] = "";
    int found = 0;

    for (int i = 0; i < n; i++) {
        int match = (role == 1) ? (p[i].assigned_agent == uid) : (p[i].customer_id == uid);
        if (match) {
            char line[256];
            snprintf(line, sizeof(line), "ID:P%d Name:%s Status:%s Location:%s\n", p[i].parcel_id, p[i].name, p[i].status, p[i].location);
            strcat(resp, line);
            found = 1;
        }
    }
    if (!found) strcat(resp, "No parcels found.\n");
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_view_my_parcels_brief(int cfd, int role, int uid) {
    Parcel p[100];
    int n = read_parcels(p);
    char resp[BUFFER_SIZE] = "";
    int found = 0;

    for (int i = 0; i < n; i++) {
        int match = (role == 1) ? (p[i].assigned_agent == uid) : (p[i].customer_id == uid);
        if (match) {
            char line[128];
            snprintf(line, sizeof(line), "ID:P%d Name:%s\n", p[i].parcel_id, p[i].name);
            strcat(resp, line);
            found = 1;
        }
    }
    if (!found) strcat(resp, "No parcels found.\n");
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_update_status(int cfd, char *args, int uid) {
    int pid;
    if (sscanf(args, "%d", &pid) != 1) {
        send_msg(cfd, "ERROR Invalid parcel ID\n");
        return;
    }

    lock_file();
    Parcel p[100];
    int n = read_parcels(p);
    int index = -1;
    for (int i = 0; i < n; i++) {
        if (p[i].parcel_id == pid && p[i].assigned_agent == uid) { index = i; break; }
    }
    if (index == -1) {
        send_msg(cfd, "ERROR Invalid parcel ID.\n");
        unlock_file();
        return;
    }
    if (strcmp(p[index].status, "Assigned") != 0) {
        send_msg(cfd, "ERROR Status can only move from Assigned to InTransit.\n");
        unlock_file();
        return;
    }
    strcpy(p[index].status, "InTransit");
    write_parcels(p, n);
    unlock_file();
    send_msg(cfd, "OK Status updated to InTransit.\n");
}

void handle_update_location(int cfd, char *args, int uid) {
    int pid, loc;
    if (sscanf(args, "%d %d", &pid, &loc) != 2) {
        send_msg(cfd, "ERROR Invalid arguments\n");
        return;
    }
    if (loc < 1 || loc > 4) {
        send_msg(cfd, "ERROR Invalid location choice.\n");
        return;
    }

    lock_file();
    Parcel p[100];
    int n = read_parcels(p);
    int index = -1;
    for (int i = 0; i < n; i++) {
        if (p[i].parcel_id == pid && p[i].assigned_agent == uid) { index = i; break; }
    }
    if (index == -1) {
        send_msg(cfd, "ERROR Invalid parcel ID.\n");
        unlock_file();
        return;
    }
    if (strcmp(p[index].status, "InTransit") != 0) {
        send_msg(cfd, "ERROR Set status to InTransit first.\n");
        unlock_file();
        return;
    }
    if (loc != p[index].location_index + 1) {
        send_msg(cfd, "ERROR Update location step by step.\n");
        unlock_file();
        return;
    }
    p[index].location_index = loc;
    strcpy(p[index].location, locations[loc]);
    write_parcels(p, n);
    unlock_file();

    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp), "OK Location updated to %s\n", locations[loc]);
    send_msg(cfd, resp);
}

void handle_pending_deliveries(int cfd, int uid) {
    Parcel p[100];
    int n = read_parcels(p);
    char resp[BUFFER_SIZE] = "";
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (p[i].assigned_agent == uid && strcmp(p[i].status, "Delivered") != 0) {
            char line[256];
            snprintf(line, sizeof(line), "ID:P%d Name:%s Status:%s\n", p[i].parcel_id, p[i].name, p[i].status);
            strcat(resp, line);
            found = 1;
        }
    }
    if (!found) strcat(resp, "No pending deliveries.\n");
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_track_parcel(int cfd, char *args, int uid) {
    int pid;
    if (sscanf(args, "%d", &pid) != 1) {
        send_msg(cfd, "ERROR Invalid parcel ID\n");
        return;
    }
    Parcel p[100];
    int n = read_parcels(p);
    for (int i = 0; i < n; i++) {
        if (p[i].parcel_id == pid && p[i].customer_id == uid) {
            char resp[256];
            snprintf(resp, sizeof(resp), "OK ID:P%d Name:%s Status:%s Location:%s\n", p[i].parcel_id, p[i].name, p[i].status, p[i].location);
            send_msg(cfd, resp);
            return;
        }
    }
    send_msg(cfd, "ERROR Parcel not found.\n");
}

void handle_view_ready_delivery(int cfd, int uid) {
    Parcel p[100];
    int n = read_parcels(p);
    char resp[BUFFER_SIZE] = "";
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (p[i].customer_id == uid && 
            p[i].location_index == 4 && 
            strcmp(p[i].status, "InTransit") == 0) {
            char line[128];
            snprintf(line, sizeof(line), "ID:P%d Name:%s\n", p[i].parcel_id, p[i].name);
            strcat(resp, line);
            found = 1;
        }
    }
    if (!found) strcat(resp, "No parcels ready for delivery confirmation.\n");
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_view_delivered(int cfd, int uid) {
    Parcel p[100];
    int n = read_parcels(p);
    char resp[BUFFER_SIZE] = "";
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (p[i].customer_id == uid && strcmp(p[i].status, "Delivered") == 0) {
            char line[256];
            snprintf(line, sizeof(line), "ID:P%d Name:%s Location:%s\n", p[i].parcel_id, p[i].name, p[i].location);
            strcat(resp, line);
            found = 1;
        }
    }
    if (!found) strcat(resp, "No delivered parcels.\n");
    send_msg(cfd, resp);
    send_msg(cfd, END_MARKER);
}

void handle_mark_delivered(int cfd, char *args, int uid) {
    int pid;
    if (sscanf(args, "%d", &pid) != 1) {
        send_msg(cfd, "ERROR Invalid parcel ID\n");
        return;
    }

    lock_file();
    Parcel p[100];
    int n = read_parcels(p);
    int index = -1;
    for (int i = 0; i < n; i++) {
        if (p[i].parcel_id == pid && p[i].customer_id == uid) { index = i; break; }
    }
    if (index == -1) {
        send_msg(cfd, "ERROR Invalid parcel ID.\n");
        unlock_file();
        return;
    }
    if (!(p[index].location_index == 4 && strcmp(p[index].status, "InTransit") == 0)) {
        send_msg(cfd, "ERROR Parcel not ready for delivery confirmation.\n");
        unlock_file();
        return;
    }
    strcpy(p[index].status, "Delivered");
    write_parcels(p, n);
    unlock_file();
    send_msg(cfd, "OK Parcel marked as Delivered.\n");
}

void handle_delete_account(int cfd, int uid) {
    Parcel p[100];
    int n = read_parcels(p);
    for (int i = 0; i < n; i++) {
        if (p[i].customer_id == uid && strcmp(p[i].status, "Delivered") != 0) {
            send_msg(cfd, "ERROR You have parcels pending delivery. Cannot delete.\n");
            return;
        }
    }

    lock_user_file();
    lock_file();

    Parcel p2[100];
    int n2 = read_parcels(p2);
    for (int i = 0; i < n2; i++) {
        if (p2[i].customer_id == uid && strcmp(p2[i].status, "Delivered") != 0) {
            send_msg(cfd, "ERROR You have parcels pending delivery. Cannot delete.\n");
            unlock_file();
            unlock_user_file();
            return;
        }
    }

    FILE *fp = fopen("data/users.txt", "r");
    FILE *temp = fopen("data/temp.txt", "w");
    User u;
    int found = 0;
    char id_str[20];

    while (fscanf(fp, "%19s %49s %49s %d", id_str, u.username, u.password, &u.role) == 4) {
        if (id_str[0] == 'A' || id_str[0] == 'C')
            u.id = atoi(id_str + 1);
        else
            u.id = atoi(id_str);

        if (u.id == uid && u.role == 2) { found = 1; continue; }
        fprintf(temp, "%c%d %s %s %d\n", (u.role == 1 ? 'A' : 'C'), u.id, u.username, u.password, u.role);
    }
    fclose(fp);
    fclose(temp);

    if (!found) {
        remove("data/temp.txt");
        send_msg(cfd, "ERROR Account not found.\n");
    } else {
        remove("data/users.txt");
        rename("data/temp.txt", "data/users.txt");
        send_msg(cfd, "OK Account deleted successfully.\n");
    }
    unlock_file();
    unlock_user_file();
}

void handle_client(int cfd) {
    char buf[BUFFER_SIZE];
    int role = -1;  // -1=not logged in, 0=admin, 1=agent, 2=customer
    int uid = -1;

    while (1) {
        int n = recv_msg(cfd, buf, sizeof(buf));
        if (n <= 0) break;

        // Strip trailing newline
        buf[strcspn(buf, "\r\n")] = '\0';

        if (strlen(buf) == 0) continue;

        printf("[SERVER] Received: %s\n", buf);

        char cmd[64] = "";
        char args[BUFFER_SIZE] = "";

        // Split into command and args
        char *space = strchr(buf, ' ');
        if (space) {
            int cmd_len = space - buf;
            strncpy(cmd, buf, cmd_len);
            cmd[cmd_len] = '\0';
            strcpy(args, space + 1);
        } else {
            strcpy(cmd, buf);
        }

        if (strcmp(cmd, "LOGIN") == 0) {
            handle_login(cfd, args, &role, &uid);
        } else if (strcmp(cmd, "REGISTER_CUSTOMER") == 0) {
            handle_register_customer(cfd, args);
        } else if (strcmp(cmd, "LOGOUT") == 0) {
            role = -1;
            uid = -1;
            send_msg(cfd, "OK Logged out.\n");
        } else if (strcmp(cmd, "QUIT") == 0) {
            send_msg(cfd, "OK Goodbye.\n");
            break;
        }
        // Admin commands
        else if (strcmp(cmd, "REGISTER_AGENT") == 0 && role == 0) {
            handle_register_agent(cfd, args);
        } else if (strcmp(cmd, "LIST_AGENTS") == 0 && role == 0) {
            handle_list_agents(cfd);
        } else if (strcmp(cmd, "LIST_CUSTOMERS") == 0 && role == 0) {
            handle_list_customers(cfd);
        } else if (strcmp(cmd, "LIST_UNASSIGNED") == 0 && role == 0) {
            handle_list_unassigned(cfd);
        } else if (strcmp(cmd, "REMOVE_AGENT") == 0 && role == 0) {
            handle_remove_agent(cfd, args);
        } else if (strcmp(cmd, "ADD_PARCEL") == 0 && role == 0) {
            handle_add_parcel(cfd, args);
        } else if (strcmp(cmd, "ASSIGN_PARCEL") == 0 && role == 0) {
            handle_assign_parcel(cfd, args);
        } else if (strcmp(cmd, "VIEW_PARCELS") == 0 && role == 0) {
            handle_view_parcels(cfd);
        } else if (strcmp(cmd, "DELETE_PARCEL") == 0 && role == 0) {
            handle_delete_parcel(cfd, args);
        } else if (strcmp(cmd, "AGENT_COUNT") == 0 && role == 0) {
            handle_agent_count(cfd);
        }
        // Agent commands
        else if (strcmp(cmd, "VIEW_MY_PARCELS") == 0 && (role == 1 || role == 2)) {
            handle_view_my_parcels(cfd, role, uid);
        } else if (strcmp(cmd, "VIEW_MY_PARCELS_BRIEF") == 0 && (role == 1 || role == 2)) {
            handle_view_my_parcels_brief(cfd, role, uid);
        } else if (strcmp(cmd, "UPDATE_STATUS") == 0 && role == 1) {
            handle_update_status(cfd, args, uid);
        } else if (strcmp(cmd, "UPDATE_LOCATION") == 0 && role == 1) {
            handle_update_location(cfd, args, uid);
        } else if (strcmp(cmd, "PENDING_DELIVERIES") == 0 && role == 1) {
            handle_pending_deliveries(cfd, uid);
        }
        // Customer commands
        else if (strcmp(cmd, "TRACK_PARCEL") == 0 && role == 2) {
            handle_track_parcel(cfd, args, uid);
        } else if (strcmp(cmd, "VIEW_READY_DELIVERY") == 0 && role == 2) {
            handle_view_ready_delivery(cfd, uid);
        } else if (strcmp(cmd, "VIEW_DELIVERED") == 0 && role == 2) {
            handle_view_delivered(cfd, uid);
        } else if (strcmp(cmd, "MARK_DELIVERED") == 0 && role == 2) {
            handle_mark_delivered(cfd, args, uid);
        } else if (strcmp(cmd, "DELETE_ACCOUNT") == 0 && role == 2) {
            handle_delete_account(cfd, uid);
        } else {
            send_msg(cfd, "ERROR Unknown command or insufficient permissions.\n");
        }
    }

    close(cfd);
    printf("[SERVER] Client disconnected.\n");
}

int main() {
    init_files();
    init_parcel_shm();
    atexit(cleanup_parcel_shm);

    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(1);
    }

    printf("====================================\n");
    printf("  PARCEL TRACKING SYSTEM SERVER\n");
    printf("====================================\n");
    printf("[SERVER] Listening on port %d...\n", SERVER_PORT);
    printf("[SERVER] Waiting for client connections...\n\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int cfd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (cfd < 0) {
            perror("accept");
            continue;
        }

        char *client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);
        printf("[SERVER] New connection from %s:%d\n", client_ip, client_port);

        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            printf("[SERVER] Client connected (PID %d)\n", getpid());
            close(server_fd);
            handle_client(cfd);
            exit(0);
        } else if (pid > 0) {
            close(cfd);
        } else {
            perror("fork");
            close(cfd);
        }
    }

    close(server_fd);
    return 0;
}
