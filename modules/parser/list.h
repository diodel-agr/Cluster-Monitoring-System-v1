#ifndef LIST_H_
#define LIST_H_
/* includes. */
#include <linux/slab.h>

/* defines. */
#define COMP struct Node

/* structures. */
//typedef struct List;
struct Node {
	int pid;
	char* command;
	struct List* list;
	struct Node* parent; /* reference to the parent node. */
};

typedef struct List {
	COMP* component;
	struct List* next;
} l;

/* prototypes. */
struct List* listCreate(void); /* create a new list. */
struct List* listInsert(struct List*, COMP*); /* insert an element to the head of the list. */
struct List* listAllocate(void); /* allocate and create a new List element. */
struct List* listDeallocate(struct List*); /* deallocate the entire list. */
void listPrint(struct List*);

#endif /* LIST_H_ */

