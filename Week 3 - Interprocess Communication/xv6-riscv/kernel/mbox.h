#include "types.h"
#include "spinlock.h"

struct mbox
{
  int queue[10];
  int front;
  int rear;
  int length;
  int valid;
  struct spinlock lock;
};
#define max_mboxes 1000
extern struct mbox array_of_mb[max_mboxes];
int enqueue(int,struct mbox*);
int dequeue(struct mbox*);
void mbox_init(void);
int mbox_create(int);
int mbox_send(int,int);
int mbox_recv(int,int*);