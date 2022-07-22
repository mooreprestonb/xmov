
#include <stdlib.h>
#include "llist.h"

NODE * addnode(int data,NODE *head)
{
  NODE *temp;
  NODE *newnode;
  temp=head;
  
  newnode=(NODE *)malloc(sizeof(NODE));
  newnode->next=NULL; 
  newnode->found=data;
  if(head==NULL){ 
    head=newnode; 
  } else {
    for(temp=head;temp->next!=NULL;temp=temp->next);
    temp->next=newnode;
  }
  return head;
}

NODE * removenode(NODE *head)
{
  NODE *delnode;
  
  delnode=head;
  head=head->next;
  free(delnode);
  return head;
}

 
