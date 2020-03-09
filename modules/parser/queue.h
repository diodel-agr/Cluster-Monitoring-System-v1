#ifndef _queue_h_
#define _queue_h_

/* includes. */
#include <linux/slab.h>

/* Elem structure. Managed by the queue. */
struct Elem {
    int ppid;
    int pid;
    char* cmd;
};

/**
 * Queue structure. It holds a Elem pointer which could be any type
 * set up at compile time.
 */
struct Queue {
	struct Elem* elem;
	struct Queue* next;
};

/**
 * The @head and @tail pointers are used to manage the whole queue functionality.
 */
static struct Queue* head = NULL;
static struct Queue* tail = NULL;

/* prototypes. */
int queue_isEmpty(void);
struct Queue* queue_getFront(void);
struct Queue* queue_dequeue(void);
void queue_enqueue(int ppid, int pid, char* cmd);
void queue_clear(void);

#endif
