/* includes. */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/mutex.h> 	/* for locking. */
#include <linux/kthread.h> 	/* for threading. */
#include <linux/delay.h> 	/* for msleep function. */
#include <linux/mutex.h> 	/* for lock-ing. */
#include <linux/kmod.h>		/* for user mode helper. */
#include <linux/types.h>	/* dev_t, register_chdev... */
#include <linux/cdev.h>		/* for cdev stucture. */
#include <linux/fs.h>		/* registed_chdev_region, alloc... */
#include <linux/uaccess.h>	/* for copy_from_user */

#include "queue.h"
#include "list.h"
#include "HashTable.h"

/* defines. */
#define SLEEP_DELAY     10 		/* miliseconds. */
#define DEVICE_NAME		"ps_parser"
#define DEVICE_COUNT	1
#define HT_ERASE		'0' /* delete ht contents command. */
#define HT_INSERT		'1' /* insert ht element command. */
#define HT_PRINT		'2' /* print the hash table after insertion. */

/* exported symbols. */
static void writeQueue(int ppid, int pid, char* cmd);

/* variables set via command line arguments. */

/***** local variables. ******/
/* process tree variables. */
static struct Node* root = NULL;
static struct Node* parent = NULL;
static struct Node* child = NULL;

/* consumet thread variables. */
static struct task_struct* parserConsumer = NULL;
static struct mutex lock;

/* hash table variables. */
struct HashTable** proc_ht = NULL;
static struct mutex mutex_ht;

/* Hash table default elements. */
char* HT_Default[] =
{
	"ps"
};

/* character device file variables. */
int parser_major = 0; // default major number value (0) for dynamic allocation.
int parser_minor = 0;
struct cdev parser_cdev;
dev_t parser_dev; // major and minor number 'holder'.

static ssize_t parser_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static struct file_operations fops = {
    .owner = THIS_MODULE,
	.write = parser_write,
};

/* debug purposes. */
static int allocNo = 0;
static int deallocNo = 0;
static int depth = 0;

/***** prototypes. *****/
void rootInit(void);
void nodeInsert(int, int, char*);
struct Node* getRoot(void);
struct Node* nodeAllocate(void);
void nodeDeallocateTree(struct Node*);
struct Node* nodeSetUpChild(struct Node*, int, char*);
struct Node* nodeSetUpRoot(int);
char* nodeSetCommand(char*);
void parseTreeKill(struct Node*);
void killProcessGroup(struct Node*);
char* itoa(int);

// debug purposes.
int getAlloc(void);
int getDealloc(void);
void parseTree(struct Node*, int);
static int consume(void* param);

//////////////////////////////////////////////////////////////
void printQueue(void) {
	char _head[] = "[head]";
	char _tail[] = "[tail]";
	char _empty[] = "";
	char* name;
	if (!queue_isEmpty()) {
		struct Queue* el = head;
		while (el != NULL) {
			name = _empty;
			if (el == head) {
				name = _head;
			} else if (el == tail) {
				name = _tail;
			}
			printk(KERN_INFO "Qel: %s %s.\n", el->elem->cmd, name);
			el = el->next;
		}
	}
	else {
		printk(KERN_INFO "__PARSER Empty queue.\n");
	}
}

void printTreeStatus(void) {
	printk(KERN_INFO "root: %p.\n", root);
	if (root != NULL) {
		printk(KERN_INFO "pid: %d, cmd: %s.\n", root->pid, root->command);
	}
	printk(KERN_INFO "parent: %p.\n", parent);
	if (parent != NULL) {
		printk(KERN_INFO "pid: %d, cmd: %s.\n", parent->pid, parent->command);
	}
	printk(KERN_INFO "child: %p.\n", child);
	if (child != NULL) {
		printk(KERN_INFO "pid: %d, cmd: %s.\n", child->pid, child->command);
	}
}
//////////////////////////////////////////////////////////////

/* implementations. */
static ssize_t parser_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	int ret = 0, bytes = 0;
	char* command = (char*)kmalloc(count * sizeof(char), GFP_KERNEL);
	if (command == NULL) {
		printk(KERN_ALERT "__PARSER chrdev write function kmalloc error!\n");
		return -EFAULT;
	}
	memset(command, 0, count);
    bytes = copy_from_user(command, buf, (count - 1)); /* returns 0 on success. */
	if (bytes) {
        printk(KERN_ALERT "__PARSER copy_from_user failed! Copied %d.\n", bytes);
        ret = 0; /* 0 bytes successfully written. */
    } else {
		/* obtain lock. */
		mutex_lock(&mutex_ht);
		if (command[0] == HT_ERASE) {
			/* erase the entire hash table. */
			HashTable_delete(proc_ht);
			proc_ht = HashTable_init();
		} else if (command[0] == HT_INSERT) {
			/* insert element. */
			HashTable_insert(proc_ht, command + 2);
		} else if (command[0] == HT_PRINT) {
			/* print the hast table. */
			HashTable_print(proc_ht);
		} else {
			printk(KERN_ALERT "__PARSER char device unknown command: %c!\n", command[0]);
		}
		/* release mutex. */
		mutex_unlock(&mutex_ht);
		/* set the number of bytes successfully written. */
        ret = count;
	}
	kfree(command);
    return ret;
}

/**
 * rootInit.
 * 
 * Used to initialise the root of the tree.
 */
void rootInit(void) {
	root = nodeAllocate();
	root->command = NULL;
	root->parent = NULL;
	root->pid = 0;
}

/** 
 * nodeInsert - inserts a node into the tree.
 * 
 * @ppid: the parent process id.
 * @pid: the child process id.
 * @command: the string that identifies the command related to the process.
 * 
 * This function inserts a node into the tree. This is the main procedure 
 * of this module. The insertion procedure is adapted to our case. 
 * Everytime this function is called, it updates 2 pointers: parent and child. 
 * Theese variables are used to find the suitable place for a new node and 
 * to parse it upwards for a fast insertion.
 */   
void nodeInsert(int ppid, int pid, char* command) {
	if (parent == NULL) { /* first time we insert a node. */
		/* set-up root. */
		parent = nodeSetUpRoot(ppid);
		/* set-up first child. */
		child = nodeSetUpChild(parent, pid, command);
		/* bind child to parent. */
		parent->list = listInsert(parent->list, child);
	} else { /* insert node somewhere in the tree. */
		if (child->pid == ppid) { /* new ppid is the child. */
			parent = child;
			parent->list = listCreate();
			/* set-up child. */
			child = nodeSetUpChild(parent, pid, command);
			/* bind child to parent. */
			parent->list = listInsert(parent->list, child);
		} else if (parent->pid == ppid) { /* add new child to the parent list. */
			/* set-up child. */
			child = nodeSetUpChild(parent, pid, command);
			/* bind child to parent. */
			parent->list = listInsert(parent->list, child);
		} else { /* unknown ppid, find it. */
			while (parent != NULL && parent->pid != ppid) {
				parent = parent->parent;
			}
			if (parent != NULL) {
				nodeInsert(ppid, pid, command);
			} else {
				printk(KERN_ALERT "ppid: %d, pid: %d, cmd: %s.\n", ppid, pid, command);
			}
		}
	}
}

/**
 *  getRoot
 * 
 *  returns the root of the tree. 
 */
struct Node* getRoot(void) {
	return root;
}

/** 
 * nodeAllocate.
 * 
 * Allocates a variable of type struct Node* and returns it.
 */
struct Node* nodeAllocate(void) {
	allocNo++;
	struct Node* newNode = (struct Node*)kmalloc(sizeof(struct Node), GFP_KERNEL);
	newNode->list = NULL;
	return newNode;
}

/**
 * nodeDeallocateTree
 * 
 * deallocates the entire tree.
 */
void nodeDeallocateTree(struct Node* node) {
    if (node != NULL) {
        /* deallocate the list. */
        while (node->list != NULL) {
            struct List* old = node->list;
            node->list = node->list->next;
            nodeDeallocateTree(old->component);
            kfree(old);
        }
        /* deallocate command. */
        if (node->command != NULL) {
			kfree(node->command);
		}
        node->parent = NULL;
        /* deallocate node. */
        kfree(node);
        node = NULL; /* set pointer to NULL - useful to have the root NULL after tree delete. */
    }
}

/**
 * nodeSetUpChild - inserts a child into the list of a parent.
 * 
 * @parent: reference to parent.
 * @pid: child pid.
 * @command: command associated with the child procee.
 * @return: pointer to the new child.
 * 
 * This function inserts a child into the list of the @parent.
 * The node is inserted into the front of the children list for 
 * O(1) complexity.
 */
struct Node* nodeSetUpChild(struct Node* parent, int pid, char* command) {
	struct Node* child = nodeAllocate();
	child->pid = pid;
	child->command = nodeSetCommand(command);
	child->list = NULL;
	child->parent = parent;
	return child;
}

/**
 * nodeSetUpRoot - initialise the root of the tree.
 * 
 * @ppid: process id of the root.
 * @return: the root.
 * 
 * This function initialises the root of the tree. 
 * This variable ususaly has ppid = pid = 0.
 */
struct Node* nodeSetUpRoot(int ppid) {
	struct Node* parent = root;
	parent->pid = ppid;
	parent->command = NULL;
	parent->list = listCreate();
	parent->parent = NULL;
	return parent;
}

/**
 * nodeSetCommand - duplicates @command and returns the copy.
 * 
 * @command: the string to duplicate.
 */
char* nodeSetCommand(char* command) {
	int len = strlen(command);
	char* newCmd = (char*)kmalloc(sizeof(char) *(len + 1), GFP_KERNEL);
	strncpy(newCmd, command, (len + 1));
	return newCmd;
}

/**
 * parseTree - parses and prints the tree in tabular format.
 * 
 * @node: root.
 * @tabNo: number of tabs to print.
 */
void parseTree(struct Node* node, int tabNo) {
    char* tabs;
    char* name;
    char _empty[] = "";
    char _root[] = "[root]";
    char _parent[] = "[parent]";
    char _child[] = "[child]";
    int i;
    /* print tabs and node info. */
    if(tabNo > 0) {
		if (tabNo > depth) {
			depth = tabNo;
		}
        tabs = (char*)kmalloc((tabNo + 1) * sizeof(char), GFP_KERNEL);
        memset(tabs, 0, tabNo);
        i = 0;
        while (i++ < tabNo) {
            strcat(tabs, "\t");
        }
        /* set name. */
        name = _empty;
        if (node == root) {
			name = _root;
		} else if (node == parent) {
			name = _parent;
		} else if (node == child) {
			name = _child;
		}
        printk(KERN_INFO "%s%d, %s %s\n", tabs, node->pid, node->command, name);
        kfree(tabs);
    } else {
        tabs = NULL;
        /* set name. */
        name = _empty;
        if (node == root) {
			name = _root;
		} else if (node == parent) {
			name = _parent;
		} else if (node == child) {
			name = _child;
		}
        printk(KERN_INFO "%d, %s %s\n", node->pid, node->command, name);
    }
    /* print node children info. */
	struct List* list = node->list;
	while (list != NULL) {
		struct List* old = list;
		list = list->next;
		parseTree(old->component, (tabNo + 1));
	}
}

/*
 * killProcessGroup - function used to kill a group of processes.
 * @node: the parent of the group.
 */
char* argv[] = {
	"/bin/kill",
	"-KILL",
	"0000000",
	NULL
};
char* envp[] = {
	"HOME=/",
	"TERM=linux",
	"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
	NULL
};
 
void killProcessGroup(struct Node* node) {
	if (node != NULL) {
		/* kill children. */
		struct List* children = node->list;
		while (children != NULL) {
			killProcessGroup(children->component);
			children = children->next;
		}
		/* kill parent. */
		printk(KERN_INFO "PARSER Killing process: %s.\n", node->command);
		struct subprocess_info* subinfo;
		char* char_pid = itoa(node->pid);
		argv[2] = char_pid;
		subinfo = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
		kfree(char_pid);
	}
}

/*
 * parseTreeKill - parse the process tree and kill black-listed processes.
 * 
 * @node: the parent of the sub-tree.
 */
void parseTreeKill(struct Node* node) {
	if (node != NULL && proc_ht != NULL) {
		/* obtain lock. */
		mutex_lock(&mutex_ht);
		if ((node->command != NULL) && (HashTable_search(proc_ht, node->command) == 1)) {
			printk(KERN_INFO "PARSER Found black listed process: %s.\n", node->command);
			/* kill process. */
			killProcessGroup(node);
			/* release lock. */
			mutex_unlock(&mutex_ht);
			/* return from recursion, no need to check children. */
			return;
		}
		/* release lock. */
		mutex_unlock(&mutex_ht);
		/* check children. */
		struct List* children = node->list;
		while (children != NULL) {
			parseTreeKill(children->component);
			children = children->next;
		}
	}
}

/*
 * itoa - integer to ascii.
 */
char* itoa(int x) {
	int k = 1, aux = x;
	while (aux != 0) {
		k++;
		aux /= 10;
	}
	if (x == 0) {
		k++;
	}
	char* res = (char*)kmalloc(k * sizeof(char), GFP_KERNEL);
	memset(res, '\0', k);
	k--;
	while (k--) {
		res[k] = (x % 10) + '0';
		x = x / 10;
	}
	return res;
}

int getAlloc(void) {
	return allocNo;
}
int getDealloc(void) {
	return deallocNo;
}

/**
 * consume - reads from the queue and inserts nodes into the tree.
 * 
 * @param: not used.
 * 
 * This function is executed by a kthread launched by the ps_reader module.
 */
static int consume(void* param) {
	int count = 0;
    struct Queue* qhead = NULL;
    while(!kthread_should_stop()) {
        /* acquire lock. */
        mutex_lock(&lock);
        if (!queue_isEmpty()) {
            /* access queue. */
            qhead = queue_dequeue();
            /* consume / process. */
            struct Elem* node = qhead->elem;
            if ((node->ppid == -1) && (node->pid == -1) && (strcmp("start", node->cmd) == 0)) {
                count = 0;
                /*release lock. */
				mutex_unlock(&lock);
                printk(KERN_INFO "__PARSER consumer start tree.\n");
                printTreeStatus();
                /* do initialization of tree. */
                if (root != NULL) {
                    nodeDeallocateTree(root);
                    root = NULL;
                    parent = NULL;
                    child = NULL;
                }
                rootInit();
                parent = child = NULL;
            } else if ((node->ppid == -1) && (node->pid == -1) && (strcmp("stop", node->cmd) == 0)) {
                printk(KERN_INFO "__PARSER consumer stop. Count = %d.\n", count);
                /* print the tree. */
                parseTree(root, 0); // decomenteaza asta ca sa afisezi arborele dupa creare.
                /* parse tree and kill black listed processes. */
                parseTreeKill(root);
                /* clear queue. */
                queue_clear();
                /*release lock. */
				mutex_unlock(&lock);
            } else if ((node->ppid >= 0) && (node->pid >= 0)) {
				count++;
				/*release lock. */
				mutex_unlock(&lock);
                //printk (KERN_INFO "__PARSER primeste ppid: %d, pid: %d, cmd: %s.\n\n", node->ppid, node->pid, node->cmd);
                nodeInsert(node->ppid, node->pid, node->cmd);
            } else {
				printk(KERN_INFO "__PARSER Tree insert error: ppid=%d, pid=%d, cmd=%s.\n", node->ppid, node->pid, node->cmd);
			}
            /* release mem. */
			kfree(node->cmd);
            kfree(node);
            kfree(qhead);
            qhead = NULL;
        } else {
            /*release lock. */
            mutex_unlock(&lock);
        }
        /* sleep a little? */
        msleep(SLEEP_DELAY);
    }
    /* release memory. */
    queue_clear();
    return 0;
}

/**
 * writeQueue - inserts a node into the parser queue.
 * 
 * @ppid: parent process id.
 * @pid: prodess id.
 * @cmd: command.
 * 
 * This function inserts the contents of the node into the queue, 
 * which will be processed by the parserConsumer kthread.
 */
static void writeQueue(int ppid, int pid, char* cmd) {
    /* acquire lock. */
    mutex_lock(&lock);
    /* access queue. */
    queue_enqueue(ppid, pid, cmd);
    /*release lock. */
    mutex_unlock(&lock);
    /* start the thread if necessary. */
    if (strcmp("start", cmd) == 0) {
		wake_up_process(parserConsumer);
    }
}
EXPORT_SYMBOL(writeQueue); /* reader module will execute this function. */

/***** init function. *****/
int __init parser_init(void) {
	int  result, err, device_no;
	/* init hash table. */
    proc_ht = HashTable_init();
    int i = 0;
    for (; i < HT_DEFAULT_ARR_SIZE; ++i)
    {
		HashTable_insert(proc_ht, HT_Default[i]);
	}
	HashTable_print(proc_ht);
	/* init char device. */
	/* set-up dev structure. */
    if (parser_major != 0) { // static allocation.
        parser_dev = MKDEV(parser_major, parser_minor);
        result = register_chrdev_region(parser_dev, DEVICE_COUNT, DEVICE_NAME);
    } else { /* dynamic allocation. */
        result = alloc_chrdev_region(&parser_dev, parser_minor, DEVICE_COUNT, DEVICE_NAME);
        parser_major = MAJOR(parser_dev);
    }
    /* check result. */
    if (result < 0) {
        printk(KERN_INFO "__READER can't get major number %d.\n", parser_major);
    }
    /* set-up cdev structure. */
    device_no = MKDEV(parser_major, parser_minor);
    cdev_init(&parser_cdev, &fops);
    parser_cdev.owner = THIS_MODULE;
    parser_cdev.ops = &fops;
    err = cdev_add(&parser_cdev, parser_dev, DEVICE_COUNT);
    /* fail. */
    if (err) {
        printk(KERN_INFO "__PARSER Error %d.", err);
        result = -ENOMEM;
    } else {
        printk (KERN_INFO "__PARSER major: %d, minor: %d.\n", parser_major, parser_minor);
        result = 0;
    }
    /* init lock. */
    mutex_init(&lock);
    mutex_init(&mutex_ht);
    /* init queue. */
    queue_clear();
    /* init kthread. */
    parserConsumer = kthread_create(consume, NULL, "parserConsumer");
    if (parserConsumer == NULL) {
        printk(KERN_INFO "__PARSER: Cannot create kthread.\n");
        return -ENOMEM;
    } else {
		wake_up_process(parserConsumer);
	}
    if (root != NULL) {
        nodeDeallocateTree(root);
    }
    printk(KERN_INFO "__PARSER init success.\n");
    return 0;
}

/***** exit function. *****/
void __exit parser_exit(void) {
    /* stop the kthread worker. */
    kthread_stop(parserConsumer);
    /* free tree. */
    if (root != NULL) {
        nodeDeallocateTree(root);
    }
    /* free hash table. */
    if (proc_ht != NULL) {
		HashTable_delete(proc_ht);
	}
	/* release char device. */
	cdev_del(&parser_cdev);
    unregister_chrdev_region(parser_dev, DEVICE_COUNT);
    printk(KERN_INFO "__PARSER unloaded.\n");
}

module_init(parser_init);
module_exit(parser_exit);

MODULE_AUTHOR("Razvan AGAPE");
MODULE_DESCRIPTION("Create a tree out of the 'ps -axjf' command output.");
MODULE_LICENSE("GPL");
