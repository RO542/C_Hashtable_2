#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

typedef struct HTNode {
    void *key;
    void *value;
    unsigned int stored_hash;
    struct HTNode *next;
} HTNode;

typedef struct Hashtable {
    unsigned int count;
    unsigned int arr_cap;
    size_t value_size;
    size_t key_size;
    HTNode **arr;
} Hashtable;

// utitlity functions 
bool is_even(int x);
bool is_prime(unsigned int x);
unsigned int next_prime(unsigned int x);

void ht_destroy_node(HTNode *);
unsigned int hash_func(void *key, size_t key_size);
// Hashtable *ht_create(size_t element_size);
bool ht_init(Hashtable *ht, size_t key_size, size_t element_size);

void ht_deinit(Hashtable *ht);

HTNode *ht_create_node(Hashtable *ht, void *key, void *value);
bool ht_put(Hashtable *ht, void *key, void *value);

bool ht_resize(Hashtable *ht, unsigned int new_size);

void *ht_find(Hashtable *ht, void *key);
bool ht_contains(Hashtable *ht, void *key);

void ht_delete(Hashtable *ht, void *key);

void ht_clear(Hashtable *ht);

void ht_destroy(Hashtable **ht);

bool ht_empty(Hashtable *ht);

