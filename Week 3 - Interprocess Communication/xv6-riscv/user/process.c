#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    int* ptr=(int*)shm_get(1);
    if(ptr==0){
        printf("shm_get failed\n");
    }
    int start_point=atoi(argv[1]);
    int end_point=atoi(argv[2]);
    int next_pos;
    if(start_point==0){
        printf("process A currently at:%d\n",start_point);
        mbox_send(1,ptr[start_point]);
        mbox_recv(2,&next_pos);
        while(next_pos!=end_point){
        printf("process A currently at:%d\n",next_pos);
        mbox_send(1,ptr[next_pos]);
        mbox_recv(2,&next_pos);
        }
    }
    else if(start_point==1){
        printf("process B currently at:%d\n",start_point);
        mbox_send(2,ptr[start_point]);
        mbox_recv(1,&next_pos);
        while(next_pos!=end_point){
        printf("process B currently at:%d\n",next_pos);
        mbox_send(2,ptr[next_pos]);
        mbox_recv(1,&next_pos);
        }
    }
    shm_close(1);
   return 0;
}