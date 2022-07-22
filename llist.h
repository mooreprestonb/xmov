
#ifndef _INCLUDE_LLIST_H
#define _INCLUDE_LLIST_H

typedef struct NODE {
  int found;
  NODE *next;
} NODE;

NODE * addnode(int data,NODE *head);
NODE * removenode(NODE *head);

#endif
