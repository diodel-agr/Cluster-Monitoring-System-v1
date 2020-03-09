/* includes. */
#include "queue.h"

/**
 * queue_isEmpty.
 *
 * Tests if the head of the queue has a valid reference.
 * Returns 1 if the queue is empty, 0 otherwise.
 */
int queue_isEmpty(void) {
	return (head == NULL);
}

/**
 * queue_getFront.
 *
 * Tests if the queue has any elements and returns the head
 * of the queue.
 */
struct Queue* queue_getFront(void) {
	if (queue_isEmpty()) {
		return NULL;
	} else {
		return head;
	}
}

/**
 * queue_dequeue.
 *
 * Extracts an element from the queue.
 */
struct Queue* queue_dequeue(void) {
	if (queue_isEmpty()) {
		return NULL;
	} else {
		struct Queue* node = head;
		head = head->next;
		return node;
	}
}

/**
 * queue_enqueue.
 *
 * @str: the Elem value to be inserted.
 *
 * Inserts an element into the queue.
 */
void queue_enqueue(int ppid, int pid, char* cmd) {
	struct Queue* newNode = (struct Queue*)kmalloc(sizeof(struct Queue), GFP_KERNEL);
	newNode->elem = (struct Elem*)kmalloc(sizeof(struct Elem), GFP_KERNEL);
    newNode->elem->ppid = ppid;
    newNode->elem->pid = pid;
    newNode->elem->cmd = (char*)kmalloc((1 + strlen(cmd)) * sizeof(char), GFP_KERNEL);
	strcpy(newNode->elem->cmd, cmd);
	newNode->next = NULL;
    /* insert. */
	if (queue_isEmpty()) {
		/* initialize the head and the queue (first element). */
		head = newNode;
	} else if (tail == NULL) {
		/* initialize the tail of the queue (second element). */
		tail = newNode;
		head->next = tail;
	} else {
        /* insert at the tail (n-th element). */
		tail->next = newNode;
		tail = newNode;
	}
}

/**
 * queue_clear.
 *
 * Deletes all the elements from the queue.
 * Usually this kind of function has O(n) complexity. However, in our case,
 * this function should have better performance. The while loop should run
 * at most once.
 */
void queue_clear(void) {
	struct Queue* node;
	if (!queue_isEmpty()) {
		while (head != NULL) {
			node = head;
			head = head->next;
			kfree(node->elem);
			kfree(node);
		}
	}
	head = NULL;
    tail = NULL;
}
