#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../include/file_db.h"
#include "../include/structures.h"

void init_files() {
    struct stat st = {0};
    if (stat("data", &st) == -1) {
        mkdir("data", 0700);
    }

    FILE *fp;
    
    fp = fopen("data/users.txt", "r");
    if (fp == NULL) {
        fp = fopen("data/users.txt", "w");
        fclose(fp);
    } else {
        fclose(fp);
    }

    fp = fopen("data/parcels.txt", "r");
    if (fp == NULL) {
        fp = fopen("data/parcels.txt", "w");
        fclose(fp);
    } else {
        fclose(fp);
    }
}

char *locations[] = {
    "ParcelWarehouse",
    "Shipping",
    "DestinationWarehouse",
    "OutForDelivery",
    "DestinationReached"
};

int generate_parcel_id(Parcel p[], int n) {
    int max = 0;
    for(int i = 0; i < n; i++) {
        if (p[i].parcel_id > max) {
            max = p[i].parcel_id;
        }
    }
    return max + 1;
}

int generate_user_id(User u[], int n, int role) {
    int max = 0;
    for(int i = 0; i < n; i++) {
        if (u[i].role == role && u[i].id > max) {
            max = u[i].id;
        }
    }
    return max + 1;
}

int read_id_input() {
    char str[20];
    scanf("%s", str);
    int i = 0;
    while (str[i] != '\0' && (str[i] < '0' || str[i] > '9')) {
        i++;
    }
    return atoi(&str[i]);
}

int read_users(User users[]) {
    FILE *fp = fopen("data/users.txt", "r");

    if (fp == NULL) {
        return 0;
    }

    int i = 0;
    char id_str[20];

    while (i < 100 && fscanf(fp, "%19s %49s %49s %d",
        id_str,
        users[i].username,
        users[i].password,
        &users[i].role) == 4) {
        
        if (id_str[0] == 'A' || id_str[0] == 'C') {
            users[i].id = atoi(id_str + 1);
        } else {
            users[i].id = atoi(id_str);
        }
        i++;
    }

    fclose(fp);
    return i;
}

int read_parcels_from_file(Parcel p[]) {
    FILE *fp = fopen("data/parcels.txt", "r");
    if (!fp) return 0;

    int i = 0;
    char pid_str[20], aid_str[20], cid_str[20];
    while (i < 100 && fscanf(fp, "%19s %19s %19s %49s %49s %49s",
        pid_str, aid_str, cid_str,
        p[i].name,
        p[i].status,
        p[i].location) == 6 ) {
        
        p[i].parcel_id = atoi(pid_str + (pid_str[0] == 'P' ? 1 : 0));
        
        if (aid_str[0] == 'A' && aid_str[1] == '-') {
            p[i].assigned_agent = -1;
        } else if (aid_str[0] == 'A') {
            p[i].assigned_agent = atoi(aid_str + 1);
        } else {
            p[i].assigned_agent = atoi(aid_str);
        }
        
        p[i].customer_id = atoi(cid_str + (cid_str[0] == 'C' ? 1 : 0));
        
        i++;
    }

    fclose(fp);
    return i;
}

void write_parcels_to_file(Parcel p[], int n) {
    FILE *fp = fopen("data/parcels.txt", "w");

    for (int i = 0; i < n; i++) {
        fprintf(fp, "P%d A%d C%d %s %s %s\n",
            p[i].parcel_id,
            p[i].assigned_agent,
            p[i].customer_id,
            p[i].name,
            p[i].status,
            p[i].location);
    }

    fclose(fp);
}

extern SharedParcelData *shared_parcel_data;

int read_parcels(Parcel p[]) {
    if (shared_parcel_data == NULL) return 0;
    
    int n = shared_parcel_data->parcel_count;
    for (int i = 0; i < n; i++) {
        p[i] = shared_parcel_data->parcels[i];
    }
    return n;
}

void write_parcels(Parcel p[], int n) {
    if (shared_parcel_data == NULL) return;
    
    shared_parcel_data->parcel_count = n;
    for (int i = 0; i < n; i++) {
        shared_parcel_data->parcels[i] = p[i];
    }
}