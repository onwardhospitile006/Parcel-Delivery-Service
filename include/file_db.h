#ifndef FILE_DB_H
#define FILE_DB_H

#include "structures.h"

int read_parcels(Parcel parcels[]);
void write_parcels(Parcel parcels[], int n);
int read_parcels_from_file(Parcel parcels[]);
void write_parcels_to_file(Parcel parcels[], int n);
int generate_parcel_id(Parcel p[], int n);
int read_users(User users[]);
int generate_user_id(User u[], int n, int role);
int read_id_input();
void init_files();

#endif