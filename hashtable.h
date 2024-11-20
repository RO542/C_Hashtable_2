#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

typedef enum {
    HT_TYPE_PTR_KEY,
    HT_TYPE_OWNS_KEY,
} HT_TYPE;

typedef struct HTEntry{
    void *key;
    void *value;
} HTEntry;

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
    HTNode **arr; // array of linked list heads
    HT_TYPE key_type;
} Hashtable;


unsigned int next_prime(unsigned int x);
bool is_even(int x);
bool is_prime(unsigned int x);
static unsigned int hash_func(const void *key, size_t key_size);

bool ht_init(Hashtable *ht, size_t key_size, size_t value_size);
Hashtable *_ht_create(size_t key_size, size_t value_size);
#define ht_create(key_size, value_size) _ht_create(sizeof(key_size), sizeof(value_size))
bool ht_put(Hashtable *ht, const void *key, const void *value);
static HTNode *ht_create_node(Hashtable *ht, const void *key, const void *value);
bool ht_resize(Hashtable *ht, unsigned int new_cap);

void ht_deinit(Hashtable *ht);
static void ht_destroy_node(HTNode *node);
void _ht_destroy(Hashtable **ht);
#define ht_destroy(ht) _ht_destroy(&ht);
void ht_delete(Hashtable *ht, const void *key);
void ht_clear(Hashtable *ht);

void *ht_find(const Hashtable *ht, const void *key);
bool ht_get(const Hashtable *ht, const void *key, void *out_value);
bool ht_contains(const Hashtable *ht, const void *key);
bool ht_empty(const Hashtable *ht);
unsigned int ht_count(const Hashtable *ht);
