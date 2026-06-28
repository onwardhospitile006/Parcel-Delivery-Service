#ifndef IPC_H
#define IPC_H

#include "structures.h"

void init_parcel_shm();
void cleanup_parcel_shm();
void lock_parcel_shm();
void unlock_parcel_shm();

#endif