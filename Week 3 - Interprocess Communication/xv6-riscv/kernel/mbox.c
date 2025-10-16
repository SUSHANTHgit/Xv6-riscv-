#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "mbox.h"
#define NULL ((void*)0)


#define size 10
#define max_mboxes 1000
struct mbox array_of_mb[max_mboxes];
int enqueue(int value,struct mbox *buffer){

    if(buffer->length==size){
        return -1;
    }
    if(buffer->front==-1)buffer->front =0;
    buffer->rear = (buffer->rear+1)%size;
    buffer->queue[buffer->rear]=value;
    buffer->length++;
    return 0;
}
int dequeue(struct mbox* buffer){
    int val;
    if(buffer->length==0)return -1;
    if(buffer->front==buffer->rear){
        val=buffer->queue[buffer->front];
        buffer->rear=-1;
        buffer->front=-1;
    }
    else{
        val=buffer->queue[buffer->front];
        buffer->front=(buffer->front+1)%size;
    }
    buffer->length--;
    return val;
}

void mbox_init(){
    for(int i=0;i<max_mboxes;i++){
        array_of_mb[i].front=-1;
        array_of_mb[i].rear=-1;
        array_of_mb[i].length=0;
        array_of_mb[i].valid=0;
        initlock(&array_of_mb[i].lock,"buffer_lock");
    }
}

int mbox_create(int key){
    if(array_of_mb[key].valid)return -1;
    else array_of_mb[key].valid=1;
    return 0;
}

int mbox_send(int mbox_id,int msg){
    acquire(&array_of_mb[mbox_id].lock);
    while(enqueue(msg,&array_of_mb[mbox_id])==-1){
        wakeup(&array_of_mb[mbox_id]);
        sleep(&array_of_mb[mbox_id],&array_of_mb[mbox_id].lock);
    }
    printf("sent %d in mailbox with id: %d\n",msg,mbox_id);
    wakeup(&array_of_mb[mbox_id]);
    release(&array_of_mb[mbox_id].lock);
    return 0;
}

int mbox_recv(int mbox_id,int *msg){
    acquire(&array_of_mb[mbox_id].lock);
    int temp;
    while((temp=dequeue(&array_of_mb[mbox_id]))==-1){
        wakeup(&array_of_mb[mbox_id]);
        sleep(&array_of_mb[mbox_id],&array_of_mb[mbox_id].lock);
    }
    *msg=temp;
    printf("receieved %d in mailbox with id: %d\n",*msg,mbox_id);
    wakeup(&array_of_mb[mbox_id]);  
    release(&array_of_mb[mbox_id].lock);
    return 0;
}