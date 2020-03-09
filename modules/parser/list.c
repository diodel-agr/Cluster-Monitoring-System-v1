/* includes. */
#include "list.h"

/* listCreate - creates a new list.
 * 
 * @return: the new list.
 */
struct List* listCreate() {
	struct List* head = NULL;
	return head;
}

/* listInsert - inserts a Comp* to the list.
 * 
 * @head: pointer to the head of the list.
 * @comp: the object to insert.
 */
struct List* listInsert(struct List* head, COMP* comp) {
	struct List* oldHead = head;
	head = listAllocate();
	head->component = comp;
	head->next = oldHead;
	return head;
}

/* listAllocate - allocate and create a new List element.
 * 
 * @return: the new element.
 */
struct List* listAllocate() { // allocate and create a new List element.
	struct List* newElem = (struct List*)kmalloc(sizeof(struct List), GFP_KERNEL);
	if (newElem == NULL) {
		return NULL;
	} else {
		newElem->component = NULL; //(COMP*)kmalloc(sizeof(COMP), GFP_KERNEL);
		return newElem;
	}
}

/* listDeallocate - deallocate an element from the list.
 * 
 * @head: the head of the list.
 * @returns: the new head.
 */
struct List* listDeallocate(struct List* head) { // deallocate the entire list.
	while (head != NULL) {
		struct List* oldHead = head;
		head = head->next;
		kfree(oldHead->component);
		kfree(oldHead);
	}
	return head;
}

/* listPrint - used for debug only.
 * 
 * @head: the head of the list.
 */
void listPrint(struct List* head) {
    printk(KERN_INFO "*KL __PARSER list print.\n");
	while (head != NULL) {
		printk(KERN_INFO "%d, ", head->component->pid);
		head = head->next;
	}
	printk(KERN_INFO "\n");
}
