#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int key = 1;   // mailbox id
  int pid;

  if(mbox_create(key) < 0){
    printf("mbtest: mailbox already exists\n");
    exit(1);
  }

  pid = fork();
  if(pid < 0){
    printf("mbtest: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    // child process: send some messages
    for(int i = 0; i < 5; i++){
      printf("child:sending%d\n", i);
      mbox_send(key, i);
    }
    exit(0);
  } else {
     
    // parent process: receive messages
    int msg;
    for(int i = 0; i < 5; i++){
      mbox_recv(key, &msg);
      printf("parent:received%d\n", msg);
    }
    wait(0);
  }

  exit(0);
}
