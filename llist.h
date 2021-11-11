#ifndef __LLIST__
#define __LLIST__
#include <stdlib.h>
typedef struct llist
{
    struct llist *next;
    int* data  ; 
}llist;

void insert(int *socket ); 
int* remove(); 
#endif