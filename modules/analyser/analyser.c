#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h> 		/* for threading stuff. */
#include <linux/sched.h> 		/* for struct task_struct. */
#include <linux/mutex.h> 		/* for locking. */
#include <linux/delay.h> 		/* for msleep. */
#include <linux/slab.h>
#include <linux/ktime.h>		/* for ktime structure. */
#include <linux/timekeeping.h>	/* for time functions. */
#include <linux/fs.h> 			/* used for registering the device. */
#include <linux/uaccess.h> 		/* for copy to user. */

/*************************************************/
#define Node char

/**
 * Queue structure. It holds a Node pointer which could be any type
 * set up at compile time.
 */
struct Queue {
	Node* elem;
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
void queue_enqueue(Node* str);
void queue_clear(void);

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
 * @str: the Node value to be inserted.
 *
 * Inserts an element into the queue.
 */
void queue_enqueue(Node* str) {
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
/********************* Queue Definision end. **************************/
/********************** Hash Table prototypes. ************************/
#define HT_SIZE 				13 /* 23, 29... */
#define HT_DEFAULT_ARR_SIZE 	4

struct HashTable
{
	char* value; /* the string stored. */
	struct HashTable* next; /* Collision resolved using chaining. */
};

/*
 * HashTable_init - function used to create a hash table variable.
 *
 * @return: the new hash table.
 */
struct HashTable** HashTable_init(void);

/*
 * HashTable_delete - delete all the elements of the hash table and the table.
 *
 * @table: the table to delete.
 * @return: null pointer to the deleted table.
 */
struct HashTable** HashTable_delete(struct HashTable** table);

/*
 * HashTable_insert - inserts a new element to the @table.
 *
 * @table: the hash table.
 * @value: the value (process name) to insert.
 */
void HashTable_insert(struct HashTable** table, char* value);

/*
 * HashTable_search - method used to check if the @value string is found in the hash table.
 *
 * @table: the hash table.
 * @value: the string to search.
 */
int HashTable_search(struct HashTable** table, char* value);

/*
 * HashTable_hash - hash function. Compute the hash value of the @value string.
 *
 * @value: string to compute the hash value.
 * @return: hash value.
 */
int HashTable_hash(char* value);

/*
 * HashTable_print - prints the hash table.
 *
 * @table: the hash table to print.
 */
void HashTable_print(struct HashTable** table);

/***** Definitions. *****/
struct HashTable** HashTable_init()
{
	struct HashTable** table = (struct HashTable**)kmalloc(HT_SIZE * sizeof(struct HashTable*), GFP_KERNEL);
	int i = 0;
	for (; i < HT_SIZE; ++i)
	{
		table[i] = NULL;
	}
	return table;
}

struct HashTable** HashTable_delete(struct HashTable** table)
{
	if (table != NULL)
	{
		int i = 0;
		for (;i < HT_SIZE; ++i)
		{
			while (table[i] != NULL)
			{
				struct HashTable* ht = table[i];
				table[i] = ht->next;
				kfree(ht->value);
				kfree(ht);
			}
		}
		kfree(table);
	}
	return table;
}

void HashTable_insert(struct HashTable** table, char* value)
{
	int key = HashTable_hash(value);
	struct HashTable* elem = (struct HashTable*)kmalloc(sizeof(struct HashTable), GFP_KERNEL);
	int size = 1 + strlen(value);
	elem->value = (char*)kmalloc(size * sizeof(char), GFP_KERNEL);
	strcpy(elem->value, value);
	elem->next = table[key];
	table[key] = elem;
}

int HashTable_search(struct HashTable** table, char* value)
{
	int key = HashTable_hash(value);
	struct HashTable* ht = table[key];
	while (ht != NULL)
	{
		if (strstr(ht->value, value) != NULL)
		{
			return 1;
		}
		ht = ht->next;
	}
	return 0;
}

int HashTable_hash(char* value)
{
	int key = 0;
	int i = 0;
	int size = strlen(value);
	for (;i < size; ++i)
	{
		key += (int)value[i];
	}
	key = key % HT_SIZE;
	return key;
}

void HashTable_print(struct HashTable** table)
{
	if (table != NULL)
	{
		int i = 0;
		for (; i < HT_SIZE; ++i)
		{
			struct HashTable* ht = table[i];
			printk(KERN_INFO "__HT i = %d.\n", i);
			while (ht != NULL)
			{
				printk(KERN_INFO "__HT \t%s.\n", ht->value);
				ht = ht->next;
			}
		}
	}
}
/************ End of Hash Table prototypes and definision *************/

/***** defines. *****/
#define SECOND 				1000	/* miliseconds. */
#define LOG_BUFFER_SIZE 	1024 	/* size of th11e buffer that will hold the log until next read. */
#define TIME_BUFFER_SIZE	100		/* size of the buffer that will hold the string timestamp representation. */
#define TIME_OFFSET			10800	/* seconds offset between Greenwich to Bucharest. */
#define HT_ERASE			'0' 	/* delete ht contents command. */
#define HT_INSERT			'1' 	/* insert ht element command. */
#define HT_PRINT			'2' 	/* print the hash table after insertion. */

/***** external variables. *****/
/* this function is exported by the keylog module. */
extern struct Queue* readKeylogQueue(void);
/* this functione is exported by the reader module. */
extern void getPSOutput(void);

/***** local variables. *****/
char* month[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
char* words[] = { /* modify the DEFAULT_ARR_SIZE aftes setting some values to this array. */
	"facebook",
	"instagram",
	"youtube",
	"music"
};
static struct task_struct* read_thread = NULL;
char log_buffer[LOG_BUFFER_SIZE];
char time_buffer[TIME_BUFFER_SIZE];
char* log_ptr;
struct mutex buffer_mutex;
struct mutex hash_mutex;
struct HashTable** word_hash;

/***** prototypes. *****/
/*
 * dev_read - character device read function.
 * 
 * This function is used by the client app to obtain the logs.
 */
static ssize_t dev_read(struct file *, char __user *, size_t count, loff_t * off_p);

/*
 * dev_write - character device write function.
 * 
 * This function is used by the client app to modify the contents of 
 * the word hash table.
 */
static ssize_t dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);

/*
 * read_queue - reads the queue's contents exported by the keylog module.
 * 
 * @ptr - unused, required by kthread.
 * @return - return value for kthread.
 */
int read_queue(void* ptr);

/*
 * analyse - reads a string and checks if it contains any forbidden processes.
 * 
 * @sentence the string to analyse.
 * 
 * This function will read the hash-map that will store forbidden processes.
 * If it finds any application/processes names which are not allowed, will
 * issue a request to close user apps.
 */
void analyse(char* sentence);

/***** char device variables *****/
/* Module Info. */
#define DEVICE_NAME 	"analyser"  /* The Device name for our Character Device Driver. */
static int major;  /* The Major Number that will be assigned to our Device Driver. */

/* Setting the Device Driver read function. */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.write = dev_write,
	.read = dev_read
};

/***** implementations. *****/
int read_queue(void* ptr) {
    printk(KERN_INFO "__ANALYSER read_thread is running!\n");
	int len = 0;
	int buf_size;
	int time_size;
	struct Queue* head = NULL;
	struct timeval time;
	struct tm date_time;
    while (!kthread_should_stop()) {
        /* obtain sentence from keylog module. */
        head = readKeylogQueue();
        if (head != NULL) {
			/* get current time. */
			do_gettimeofday(&time);
			time_to_tm(time.tv_sec, TIME_OFFSET, &date_time);
			len = strlen(head->elem);
			if (len != 0) {
				printk(KERN_INFO "__ANALYSER: Process string: %s.\n", head->elem);
				/* obtain lock. */
				mutex_lock(&buffer_mutex);
				buf_size = log_ptr - log_buffer;
				/* log this sentence. */
				if (len > (LOG_BUFFER_SIZE - buf_size)) {
					printk(KERN_INFO "__ANALYSER Buffer size overload!\n");
					printk(KERN_INFO "__ANALYSER Log buffer:\n%s size: %d.\n", log_buffer, buf_size);
					memset(log_buffer, '\0', LOG_BUFFER_SIZE);
					log_ptr = log_buffer;
				} else {
					memset(time_buffer, '\0', TIME_BUFFER_SIZE);
					time_size = sprintf(time_buffer, "[%d.%s.%d %d:%d:%d]: ", (1900 + date_time.tm_year), month[date_time.tm_mon], date_time.tm_mday, date_time.tm_hour, date_time.tm_min, date_time.tm_sec);
					strcat(log_ptr, time_buffer);
					strcat(log_ptr, head->elem);
					strcat(log_ptr, "\n");
					log_ptr += (1 + len + time_size) * sizeof(char);
					printk(KERN_INFO "__ANALYSER Log buffer:\n%s size: %d (%d).\n", log_buffer, (log_ptr - log_buffer - 1), (1 + len + time_size));
				}
				/* release mutex. */
				mutex_unlock(&buffer_mutex);
				/* analyse sentence. */
				analyse(head->elem);
				/* release string memory */
				kfree(head->elem);
				kfree(head);
			}
		}
        /* sleep a little bit. */
        msleep(SECOND);
    }
    return 0;
}

void analyse(char* sentence) {
    char* word;
    while ((word = strsep(&sentence, " "))) {
        if (HashTable_search(word_hash, word) == 1) {
            printk(KERN_INFO "_ANALYSER found word %s! Request process kill.\n", word);
            /* from here, send signal to the parser module to kill bad processes. */
            getPSOutput();
            /* return, no need to analyse further. */
            return;
        }
    }
}

static ssize_t dev_read(struct file *fp, char __user *buf, size_t length, loff_t *offset) {
	/* obtain lock. */
	mutex_lock(&buffer_mutex);
	/* copy data. */
	int len = strlen(log_buffer);
	int ret = copy_to_user(buf, log_buffer, len);
	if (ret) {
		printk(KERN_INFO "__ANALYSER Device read couldn't copy all data to user space.\n");
		/* release lock. */
		mutex_unlock(&buffer_mutex);
		return ret;
	}
	memset(log_buffer, '\0', LOG_BUFFER_SIZE); /* Reset buffer after each read. */
	log_ptr = log_buffer; /* Reset buffer pointer. */
	/* release mutex. */
	mutex_unlock(&buffer_mutex);
	return len;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
	int ret = 0, bytes = 0;
	char* command = (char*)kmalloc(count * sizeof(char), GFP_KERNEL);
	if (command == NULL) {
		printk(KERN_ALERT "__ANALYSER chrdev write function kmalloc error!\n");
		return -EFAULT;
	}
	memset(command, 0, count);
    bytes = copy_from_user(command, buf, (count - 1)); /* returns 0 on success. */
	if (bytes) {
        printk(KERN_ALERT "__ANALYSER copy_from_user failed! Copied %d.\n", bytes);
        ret = 0; /* 0 bytes successfully written. */
    } else {
		/* obtain lock. */
		mutex_lock(&hash_mutex);
        printk(KERN_INFO "__ANALYSER Write function: %s.\n", command);
		if (command[0] == HT_ERASE) {
			/* erase the entire hash table. */
			HashTable_delete(word_hash);
			word_hash = HashTable_init();
		} else if (command[0] == HT_INSERT) {
			/* insert element. */
            printk(KERN_INFO "__ANALYSER HT INSERT %s.\n", command);
			HashTable_insert(word_hash, command + 2);
		} else if (command[0] == HT_PRINT) {
			/* print the hast table. */
			HashTable_print(word_hash);
		} else {
			printk(KERN_ALERT "__PARSER char device unknown command: %c!\n", command[0]);
		}
		printk(KERN_INFO "__ANALYSER Hash table:\n");
		HashTable_print(word_hash);
		/* release mutex. */
		mutex_unlock(&hash_mutex);
		/* set the number of bytes successfully written. */
        ret = count;
	}
	kfree(command);
    return ret;
}

/***** init and exit functions. *****/
static int __init analyser_init(void) {
	/* init lock. */
    mutex_init(&buffer_mutex);
    mutex_init(&hash_mutex);
    /* init hash. */
    word_hash = HashTable_init();
    int i;
    for (i = 0; i < HT_DEFAULT_ARR_SIZE; ++i)
    {
		HashTable_insert(word_hash, words[i]);
	}
	printk(KERN_INFO "__ANALYSER hash table:\n");
	HashTable_print(word_hash);
	/* set the character device. */
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "__ANALYSER failed to register a major number.\n");
		return major;
	}
	printk(KERN_INFO "__ANALYSER Registered keylogger with major number: %d.\n", major);
	/* set log buffer. */
	memset(log_buffer, '\0', LOG_BUFFER_SIZE);
	log_ptr = log_buffer;
    /* init the thread and start it. */
    read_thread = kthread_create(read_queue, NULL, "A_read_queue_th");
    if (read_thread) {
        wake_up_process(read_thread);
    } else {
        printk(KERN_ERR "__ANALYSER Cannot create kthread.\n");
    }
    printk(KERN_INFO "_ANALYSER Init success.\n");
    return 0;
}

static void __exit analyser_exit(void) {
    /* wait for the thread to wake-up and stop it. */
    kthread_stop(read_thread);
    /* unregister character device. */
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "__ANALYSER unloaded.\n");
}

module_init(analyser_init);
module_exit(analyser_exit);

MODULE_AUTHOR("Razvan AGAPE");
MODULE_LICENSE("GPL");
