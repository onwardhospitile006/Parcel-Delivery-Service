#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include "../include/ipc.h"
#include "../include/file_db.h"

int parcel_shmid = -1;
int parcel_semid = -1;
SharedParcelData *shared_parcel_data = NULL;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void lock_parcel_shm() {
    if (parcel_semid == -1) return;
    struct sembuf sb = {0, -1, 0};
    semop(parcel_semid, &sb, 1);
}

void unlock_parcel_shm() {
    if (parcel_semid == -1) return;
    struct sembuf sb = {0, 1, 0};
    semop(parcel_semid, &sb, 1);
}

void init_parcel_shm() {
    key_t key_shm = 12345;
    key_t key_sem = 12346;

    // Create or get the semaphore
    int is_new_sem = 0;
    parcel_semid = semget(key_sem, 1, 0666 | IPC_CREAT | IPC_EXCL);
    if (parcel_semid != -1) {
        is_new_sem = 1;
        union semun u;
        u.val = 1; // Initialize to 1 (unlocked)
        semctl(parcel_semid, 0, SETVAL, u);
    } else {
        parcel_semid = semget(key_sem, 1, 0666);
    }

    lock_parcel_shm();

    // Create or get shared memory
    int is_new_shm = 0;
    parcel_shmid = shmget(key_shm, sizeof(SharedParcelData), 0666 | IPC_CREAT | IPC_EXCL);
    if (parcel_shmid != -1) {
        is_new_shm = 1;
    } else {
        parcel_shmid = shmget(key_shm, sizeof(SharedParcelData), 0666);
    }

    shared_parcel_data = (SharedParcelData*) shmat(parcel_shmid, NULL, 0);

    struct shmid_ds shm_info;
    if (shmctl(parcel_shmid, IPC_STAT, &shm_info) != -1) {
        if (is_new_shm || shm_info.shm_nattch == 1 || shared_parcel_data->ref_count == 0) {
            shared_parcel_data->parcel_count = read_parcels_from_file(shared_parcel_data->parcels);
            shared_parcel_data->ref_count = 0;
        }
    } else {
        // Fallback
        if (is_new_shm || shared_parcel_data->ref_count == 0) {
            shared_parcel_data->parcel_count = read_parcels_from_file(shared_parcel_data->parcels);
            shared_parcel_data->ref_count = 0;
        }
    }

    shared_parcel_data->ref_count++;

    unlock_parcel_shm();
}

void cleanup_parcel_shm() {
    if (shared_parcel_data == NULL) return;

    lock_parcel_shm();

    // Always save parcels to file before exiting
    write_parcels_to_file(shared_parcel_data->parcels, shared_parcel_data->parcel_count);

    shared_parcel_data->ref_count--;

    shmdt(shared_parcel_data);
    unlock_parcel_shm();
}