
#include "HashTable.h"

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
