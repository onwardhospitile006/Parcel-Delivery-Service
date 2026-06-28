#include <fcntl.h>
#include <unistd.h>
#include "../include/sync.h"

#include "../include/ipc.h"

int parcel_fd;
int user_fd;

void lock_file() {
    lock_parcel_shm();
}

void unlock_file() {
    unlock_parcel_shm();
}

void lock_user_file() {
    user_fd = open("data/.users.lock", O_RDWR | O_CREAT, 0666);

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    fcntl(user_fd, F_SETLKW, &lock);
}

void unlock_user_file() {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    fcntl(user_fd, F_SETLK, &lock);
    close(user_fd);
}