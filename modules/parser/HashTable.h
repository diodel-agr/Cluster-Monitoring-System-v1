
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <linux/slab.h>

#define HT_SIZE 				13 /* 23, 29... */
#define HT_DEFAULT_ARR_SIZE 	1

struct HashTable
{
	char* value; /* the string stored. */
	struct HashTable* next; /* Collision resolved using chaining. */
};

/***** HASH TABLE INTERFACE: init, delete, insert, search. *****/

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

/***** HASH TABLE 'PRIVATE' FUNCTIONS. *****/

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

#endif /* HASHTABLE_H_ */
