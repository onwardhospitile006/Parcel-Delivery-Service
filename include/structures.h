#ifndef STRUCTURES_H
#define STRUCTURES_H

#define MAX 100

typedef struct {
    int id;
    char username[50];
    char password[50];
    int role;
} User;

typedef struct {
    int parcel_id;
    int assigned_agent;
    int customer_id;
    char name[50];
    char status[50];
    char location[50];
    int location_index;
} Parcel;

extern char *locations[];

typedef struct {
    Parcel parcels[100];
    int parcel_count;
    int ref_count;
} SharedParcelData;

#endif