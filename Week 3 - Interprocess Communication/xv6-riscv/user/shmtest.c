#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    printf("shmtest: starting...\n");

    int key = 1;  // shared memory key
    char *ptr;

    // Process 1: get shared memory
    ptr = (char *) shm_get(key);
    if(ptr == 0){
        printf("shmtest: shm_get failed\n");
        exit(1);
    }
    printf("shmtest: writing to shared memory\n");
    strcpy(ptr, "hello from proc1");

    // Fork so child can also use the shared memory
    int pid = fork();
    if(pid < 0){
        printf("shmtest: fork failed\n");
        exit(1);
    }

    if(pid == 0){
        // Child process: attach same shared memory
        char *child_ptr = (char *) shm_get(key);
        if(child_ptr == 0){
            printf("child: shm_get failed\n");
            exit(1);
        }

        printf("child: read from shared memory: %s\n", child_ptr);
        strcpy(child_ptr, "hello from child");
        // close shared memory
        shm_close(key);
        exit(0);
    } else {
        // Parent waits
        wait(0);
        printf("parent: read from shared memory: %s\n", ptr);
        // parent closes shared memory
        shm_close(key);
    }

    printf("shmtest: finished.\n");
    exit(0);
}
