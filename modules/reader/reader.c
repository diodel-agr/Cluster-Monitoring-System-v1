/* includes. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> 			/* working with file system structures. */
#include <linux/types.h> 		/* for dev_t types, register_chrdev. */
#include <linux/uaccess.h> 		/* for copy_from_user(). */
#include <linux/cdev.h> 		/* for cdev structure. */
#include <linux/slab.h> 		/* for kmalloc, kfree. */
#include <linux/kmod.h> 		/* for call user mode helper. */
#include <linux/unistd.h>
#include <linux/mutex.h> 		/* for locking. */
#include <linux/kthread.h> 		/* for threading. */
#include <linux/delay.h> 		/* for msleep function. */

/* defines. */
#define DEVICE_COUNT 	1
#define DEVICE_NAME 	"ps_reader"
#define SCRIPT_NAME 	"getPSOutput.sh"
#define SLEEP_DELAY 	10 	/* miliseconds. */
#define Node 			char

/* structures. */
/**
 * Queue structure. It holds a Node pointer which could be any type
 * set up at compile time.
 */
struct Queue {
	Node* elem;
	struct Queue* next;
};

/***** external variables. *****/
/* this function is exported by the parser module. */
extern void writeQueue(int ppid, int pid, char* cmd);

/***** exported symbols. *****/
/* this function is called by the analyser module. */
static void getPSOutput(void);
EXPORT_SYMBOL(getPSOutput);

/* variables set via command line arguments to insmod. */
char path[] = "/home/razvan/Documents/LICENTA/modules/keylogger/reader/";
char program_name[] = "caller.o";
char* debug = "true"; // set "true" if you want to see ps output into a debug file.
int reader_major = 0; // default major number value (0) for dynamic allocation.

/* local variables. */
int reader_minor = 0;
struct cdev reader_cdev;
dev_t reader_dev; // major and minor number 'holder'.
struct task_struct* readerConsumer = NULL;
static struct mutex lock; /* lock for synchronizing the access to the queue. */
static struct Queue* head = NULL; /* queue head. */
static struct Queue* tail = NULL; /* queue tail. */

/* prototypes. */
static ssize_t reader_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static int consume(void*);
static int atoi(char*);

/* prototypes for the queue functions. */
int queue_isEmpty(void);
struct Queue* queue_getFront(void);
struct Queue* queue_dequeue(void);
void queue_enqueue(Node* str);
void queue_clear(void);

/* setting the device driver write function. */
static struct file_operations fops = {
    .owner = THIS_MODULE,
	.write = reader_write,
};

/**
* reader_write - the device driver write function.
*
* @filp pointer to the file structure.
* @buf pointer to the user space buffer to read from.
* @count number of characters to read from the buffer.
* @f_pos offset into the file.
* @return the number of bytes successfully written. If the return value is different by @count, 
* the caller may try again to write.
* 
* This function will be executed when some process will write
* to the character device file.
*/
static ssize_t reader_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) { // define device driver write function.
    int ret, bytes;
    char* ps_output = NULL;
    ps_output = (char*)kmalloc(count * sizeof(char), GFP_KERNEL);
    if (ps_output == NULL) { /* kmalloc error. */
        printk(KERN_INFO "__READER Error allocating ps_output!\n");
        return -EFAULT;
    }
    memset(ps_output, 0, count);
    bytes = copy_from_user(ps_output, buf, (count - 1)); /* returns 0 on success. */
    if (bytes) {
        printk(KERN_INFO "__READER copy_from_user failed! copied %d.\n", bytes);
        ret = 0; /* 0 bytes successfully written. */
    } else {
        /* acquire lock. */
        mutex_lock(&lock);
        /* add string to the queue. */
        queue_enqueue(ps_output);
        /* release lock. */
        mutex_unlock(&lock);
        /* start thread if necessary. */
        if (strcmp("start", ps_output) == 0) {
            wake_up_process(readerConsumer);
        }
        /* set the number of bytes successfully written. */
        ret = count;
    }
    kfree(ps_output);
    return ret;
}

/**
 * getPSOutput - request ps output.
 * 
 * This function uses the call_usermodehelper functions to call a user space program 
 * called 'caller.o' that send the output of 'ps -axjf' to this module. The output of 
 * ps will be processed by the 'reader_write' function.
 */
static void getPSOutput(void) {
	printk(KERN_INFO "__READER 'ps -axjf' request.");
    /* set-up things. */
    char* script = (char*)kmalloc(sizeof(char) * (strlen(path) + strlen(program_name) + 1), GFP_KERNEL);
    if (script == NULL) {
        printk(KERN_INFO "__READER kmalloc failed!\n");
        return;
    }
    strcpy(script, path);
    strcat(script, program_name);
    char major[10];
    sprintf(major, "%d", reader_major);
    /* arguments. */
    char *argv[] = {
        script, /* path + program_name. */
        path,
        SCRIPT_NAME, /* getPSOutput.sh */
        DEVICE_NAME, /* ps_reader */
        major,
        "true",
        (char*)NULL
    };
    /* environment. */
	static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin",
        (char*)NULL
    };
    /* here is where the magic happens. */
    struct subprocess_info *info;
	info = call_usermodehelper_setup(script, argv, envp, GFP_KERNEL, NULL, NULL, NULL);
	if (info == NULL) {
        printk(KERN_INFO "*KL -ENOMEM.\n");
		return;
    }
    call_usermodehelper_exec(info, UMH_WAIT_PROC);
    kfree(script);
    printk(KERN_INFO "__READER GetPSOutput success.\n");
}    

/**
 * consume - process the contents of a queue containing the output from ps.
 * 
 * @param: this parameter is not used. Thist signature is demanded by kthread_create.
 * 
 * This function consumes the contents of the reader_queue, processes them and sends them 
 * to the parser_queue. The processing is: from a string like 'pid ppid command' creates a 
 * structure of type { int pid, int pid, char* command }.
 * The thread that executes this function is started by the reader_write function and runs 
 * until "stop" is read from the queue.
 */
static int consume(void* param) {
	int count = 0;
    /* consume queue elements. */
    struct Queue* node = NULL;
    while (!kthread_should_stop()) {
        /* acquire lock. */
        mutex_lock(&lock);
        /* access queue. */
        if (!queue_isEmpty()) {
            node = queue_dequeue();
            /* consume... */
            if (strcmp("start", node->elem) == 0) { /* start signal. */
				count = 0;		
                /* send data to parser. */
                printk(KERN_INFO "__READER: start.\n");
                writeQueue(-1, -1, node->elem);
            } else if (strcmp("stop", node->elem) == 0) { /* stop signal. */
                /* send string to parser and stop running. */
                printk(KERN_INFO "__READER: stop. Count = %d.\n", count);
                writeQueue(-1, -1, node->elem);
                /* clear queue. */
                queue_clear();
            } else if (strcmp("PPID PID COMMAND", node->elem) != 0) {
                /* process node contents. */
                char* output = node->elem;
                char* _ppid = strsep(&(output), " ");
                char* _pid = strsep(&(output), " ");
                char* path = strsep(&(output), " ");
                char* command, *next;
                command = strsep(&(path), "/");
                while ((next = strsep(&(path), "/")) != NULL) {
					command = next;
				}
				//printk(KERN_INFO "__before atoi: ppid: %s, pid: %s.\n", _ppid, _pid);
                int ppid = atoi(_ppid);
                int pid = atoi(_pid);
                /* print strings / write to the other queue, if no error occured. */
                if (ppid >= 0 && pid >= 0) {
					count++;
                    /* send data to parser. */
                    writeQueue(ppid, pid, command);
                } else {
					/* ppid or pid error. */
					printk(KERN_INFO "__READER atoi_error. ppid: %d, pid: %d, _ppid: %s, _pid: %s.\n", ppid, pid, _ppid, _pid);
				}
            }
			/* release lock. */
            mutex_unlock(&lock);
            /* release mem */
            kfree(node->elem);
            kfree(node);
            node = NULL;
        } else {
            /* release lock. */
            mutex_unlock(&lock);
        }
        /* sleep a little? */
        msleep(SLEEP_DELAY);
    }
    /* thread is stopping, release mem */
    queue_clear();
    return 0;
}

/**
 * atoi - Ascii TO Integer.
 * 
 * @str: the string to parse.
 * 
 * This function has the same purpose as the library function.
 * Interprets the contents of @str as numbers and returns the corresponding integer.
 * However, this version is adapted to our case, which is: we are sure that a number
 * is represented in @str and that number is positive.
 */
static int atoi(char* str) {
    if (str == NULL) {
        return -2; /* invalid argument. */
    }
    int result = 0, i = 0;
    while (str[i] != 0) {
        result = (result * 10) + (str[i] - '0');
        i++;
    }
    return result;
}

/********************* queue functions definition. *********************/

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
	return head;
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
 * @str: the Node value to be inserted.
 *
 * Inserts an element into the queue.
 */
void queue_enqueue(Node* str) {
	/* prepare new queue element. */
	struct Queue* newNode = (struct Queue*)kmalloc(sizeof(struct Queue), GFP_KERNEL);
	newNode->elem = (Node*)kmalloc((1 + strlen(str)) * sizeof(Node), GFP_KERNEL);
	strcpy(newNode->elem, str);
	newNode->next = NULL;
    /* insert. */
	if (queue_isEmpty()) {
		/* initialize the head and the queue. */
		head = newNode;
	} else if (tail == NULL) {
		/* initialize the tail of the queue. */
		tail = newNode;
		head->next = tail;
	} else {
        /* insert at the tail. */
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
	while (!queue_isEmpty()) {
        node = head;
        head = head->next;
        kfree(node->elem);
        kfree(node);
	}
	head = NULL;
    tail = NULL;
}

/****** init and exit functions. ******/
int __init reader_init(void) {
    /* check variables setted by command line arguments. */
    int result, err, device_no;
    /* set-up dev structure. */
    if (reader_major != 0) { // static allocation.
        reader_dev = MKDEV(reader_major, reader_minor);
        result = register_chrdev_region(reader_dev, DEVICE_COUNT, DEVICE_NAME);
    } else { /* dynamic allocation. */
        result = alloc_chrdev_region(&reader_dev, reader_minor, DEVICE_COUNT, DEVICE_NAME);
        reader_major = MAJOR(reader_dev);
    }
    // check result.
    if (result < 0) {
        printk(KERN_INFO "__READER can't get major number %d.\n", reader_major);
    }
    /* set-up cdev structure. */
    device_no = MKDEV(reader_major, reader_minor);
    cdev_init(&reader_cdev, &fops);
    reader_cdev.owner = THIS_MODULE;
    reader_cdev.ops = &fops;
    err = cdev_add(&reader_cdev, reader_dev, DEVICE_COUNT);
    /* fail. */
    if (err) {
        printk(KERN_INFO "__READER Error %d.\n", err);
        result = -ENOMEM;
    } else {
        printk (KERN_INFO "__READER major: %d, minor: %d.\n", reader_major, reader_minor);
        result = 0;
    }
    /* init mutex. */
	mutex_init(&lock);
    /* set-up consumer thread. */
    readerConsumer = kthread_create(consume, NULL, "readerConsumer");
    if (readerConsumer == NULL) {
        printk(KERN_INFO "__READER: Cannot create kthread.\n");
        return -ENOMEM;
    }
    /* set-up queue. */
    queue_clear();
    printk(KERN_INFO "__READER Init success.\n");
    return 0; // success.
}

void __exit reader_exit(void) {
    /* deallocate resources. */
    cdev_del(&reader_cdev);
    unregister_chrdev_region(reader_dev, DEVICE_COUNT);
    /* stop kthread. */
	kthread_stop(readerConsumer);
    printk(KERN_INFO "__READER unloaded.\n");
}

module_init(reader_init);
module_exit(reader_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Razvan AGAPE");
MODULE_DESCRIPTION("Read 'ps -axjf' output.");
