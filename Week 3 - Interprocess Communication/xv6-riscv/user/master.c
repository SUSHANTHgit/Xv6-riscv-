#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main(){
    int *ptr;
    ptr=(int*)shm_get(1);
    if(ptr==0)printf("shm_get failed\n");
    // hardcoding interwined path
    ptr[0]=5; 
    ptr[1]=6;
    ptr[6]=10;
    ptr[5]=11;
    ptr[11]=20;
    ptr[10]=21;
    if(mbox_create(1)<0)printf("mbox already exists with this key");
    if(mbox_create(2)<0)printf("mbox already exists with this key");
       //forking twice for two child processes one for process A and another for process B
       int pid=fork();
       if(pid==0){
          char * args[]={"process","0","21",0};
          exec("process",args);
          exit(0);
       }
       int pid2=fork();
       if(pid2==0){
          char * args[]={"process","1","20",0};
          exec("process",args);
          exit(0);
       }
     wait(0);
     shm_close(1);
    return 0;
}