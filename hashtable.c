#include "hashtable.h"

#define XXH_STATIC_LINKING_ONLY /* access advanced declarations */
#define XXH_IMPLEMENTATION
#include "xxhash/xxhash.h"
#include <assert.h>

bool ht_init(Hashtable *ht, size_t key_size, size_t value_size) {
    if (!ht) {
        fprintf(stderr, "A valid hashtable pointer is required for init\n");
        return false;
    }
    ht->arr_cap = 17; // prime
    ht->arr = (HTNode **)malloc(ht->arr_cap * sizeof(HTNode *));
    if (!ht->arr) {
        fprintf(stderr, "Failed to allocate memory for internal hashtable array during ht_init\n");
        return false;
    }
    ht->count = 0;
    ht->key_size = key_size;
    ht->value_size = value_size;
    memset(ht->arr, 0, ht->arr_cap * sizeof(HTNode *)); 
    return true;
}

Hashtable* _ht_create(size_t key_size, size_t value_size) {
    Hashtable *ht = (Hashtable *)malloc(sizeof(Hashtable));
    if (!ht) {
        fprintf(stderr, "Failed to allocate hashtable during ht_create\n");
        return false;
    }
    if (!ht_init(ht, key_size, value_size)) {
        fprintf(stderr, "Failed to call ht_init hashtable during ht_create\n");
        return false;
    }
    return ht;
}

static HTNode* ht_create_node(Hashtable *ht, const void *key, const void *value) {
    HTNode *new_node = (HTNode *)malloc(sizeof(HTNode));
    if (!new_node) {
        fprintf(stderr, "Failed to allocate new HTNode in ht_put\n");
        return NULL;
    }

    new_node->key = malloc(ht->key_size);
    if (!new_node->key) {
        free(new_node);
        fprintf(stderr, "Failed to allocate new key during ht_put\n");
        return NULL;
    }

    new_node->value = malloc(ht->value_size);
    if (!new_node->value) {
        free(new_node->key);
        free(new_node);
        fprintf(stderr, "Failed to allocate new value during ht_put\n");
        return NULL;
    }

    memset(new_node->key, 0, ht->key_size);
    memset(new_node->value, 0, ht->value_size);
    memcpy(new_node->key, key, ht->key_size);
    memcpy(new_node->value, value, ht->value_size);
    new_node->next = NULL;
    return new_node;
}

bool ht_put(Hashtable *ht, const void *key, const void *value) {
    assert(ht); assert(key); assert(value);
    if ((float)ht->count / ht->arr_cap >= 0.75) {
        if (!ht_resize(ht, 2 * ht->arr_cap)) {
            fprintf(stderr, "Failed call to ht_resize in ht_put\n");
            return false;
        }
    }

    unsigned int key_hash = hash_func(key, ht->key_size);
    unsigned int bucket_idx = key_hash % ht->arr_cap;
    for (HTNode *curr_node = ht->arr[bucket_idx]; curr_node != NULL; curr_node = curr_node->next) {
        if (key_hash == curr_node->stored_hash && memcmp(key, curr_node->key, ht->key_size) == 0) {
            memcpy(curr_node->value, value, ht->value_size);
            return true;
        }
    }

    HTNode *new_node = ht_create_node(ht, key, value);
    if (!new_node) {
        fprintf(stderr, "Failed to allocate new node in ht_put\n");
        return false;
    }
    new_node->stored_hash = key_hash;
    new_node->next = ht->arr[bucket_idx];
    ht->arr[bucket_idx] = new_node;
    ht->count++;
    return true;
}

bool ht_resize(Hashtable *ht, unsigned int new_cap) {
    HTNode** old_arr = ht->arr;
    unsigned int old_cap = ht->arr_cap;
    unsigned int new_capacity = next_prime(new_cap);
    if (new_cap < ht->count) {
        printf("Warning, resizing hashtable to smaller capacity from %u to %u\n", ht->arr_cap, new_cap );
    }

    ht->arr = (HTNode **)malloc(new_capacity * sizeof(HTNode *));
    if (!ht->arr) {
        fprintf(stderr, "ht_resize, failed to allocate new ptr to arr of buckets, old ht preserved\n");
        return false; // Return early on allocation failure
    }

    ht->arr_cap = new_capacity;
    memset(ht->arr, 0, (new_capacity * sizeof(HTNode *)));
    for (unsigned int i = 0; i < old_cap; i++) {
        HTNode *ll_head = old_arr[i];
        for (HTNode *node = ll_head, *next; node != NULL; node = next) {
            next = node->next;
            node->next = NULL;
            unsigned int bucket_idx = hash_func(node->key, ht->key_size) % new_capacity;
            if (ht->arr[bucket_idx]) {
                node->next = ht->arr[bucket_idx];
            }
            ht->arr[bucket_idx] = node;
        }
    }
    free(old_arr);
    return true;
}

// returns a pointer to the value associated with a key if the key exists, else NULL
void *ht_find(const Hashtable *ht, const void *key) {
    unsigned int key_hash = hash_func(key, ht->key_size);
    unsigned int bucket_idx = key_hash % ht->arr_cap;
    for (HTNode *curr_node = ht->arr[bucket_idx]; curr_node != NULL; curr_node = curr_node->next) {
        if (key_hash == curr_node->stored_hash && memcmp(key, curr_node->key, ht->key_size) == 0) {
            return curr_node->value;
        }
    }
    return NULL;
}

// copies value associated to the key to out_value and returns true if the key is found 
// otherwise if the key doesn't exist out_value is unchanged and false is returned 
bool ht_get(const Hashtable *ht, const void *key, void *out_value) {
    unsigned int key_hash = hash_func(key, ht->key_size);
    unsigned int bucket_idx = key_hash % ht->arr_cap;
    for (HTNode *curr_node = ht->arr[bucket_idx]; curr_node != NULL; curr_node = curr_node->next) {
        if (key_hash == curr_node->stored_hash && memcmp(key, curr_node->key, ht->key_size) == 0) {
            memcpy(out_value, curr_node->value, ht->value_size);
            return true;
        }
    }
    return false;
}

bool ht_contains(const Hashtable *ht, const void *key) {
    return ht_find(ht, key) != NULL;
}

// internal function freeing memory associated with an HTNode
static void ht_destroy_node(HTNode *node) {
    if (!node || !node->key || !node->value) {
        fprintf(stderr, "node to destroy is NULL or previously destroyed\n");
        return;
    }
    free(node->key);
    free(node->value);
    free(node);
    node->key = NULL;
    node->value = NULL;
}


void ht_delete(Hashtable *ht, const void *key) {
    if (ht_empty(ht)) {
        fprintf(stderr, "Unable to remove key from empty Hashtable\n");
        return;
    }
    unsigned int key_hash = hash_func(key, ht->key_size);
    unsigned int bucket_idx = key_hash % ht->arr_cap;
    HTNode *prev_node = NULL;
    HTNode *curr_node = ht->arr[bucket_idx];
    while (curr_node) {
        if (curr_node->stored_hash == key_hash && memcmp(key, curr_node->key, ht->key_size) == 0) {
            if (prev_node) {
                prev_node->next = curr_node->next;
            } else { // no prev_node means removing the head so head->next is the new head
                ht->arr[bucket_idx] = curr_node->next;
            }
            ht_destroy_node(curr_node);
            ht->count--;
            return;
        }
        prev_node = curr_node;
        curr_node = curr_node->next;
    }
}

void ht_clear(Hashtable *ht) {
    for (int i = 0; i < ht->arr_cap; i++) {
        HTNode *curr_node = ht->arr[i];
        while (curr_node) {
            HTNode *next_node = curr_node->next;
            ht_destroy_node(curr_node);
            curr_node = next_node;
        }
        ht->arr[i] = NULL;
    }
    ht->count = 0;
}

void ht_deinit(Hashtable *ht) {
    ht_clear(ht);
    free(ht->arr);
    ht->arr = NULL;
}

bool ht_empty(const Hashtable *ht) {
    return ht->count == 0;
}

unsigned int ht_count(const Hashtable *ht) {
    return ht->count;
}

void _ht_destroy(Hashtable **ht_ptr) {
    if (ht_ptr && *(ht_ptr)) {
        Hashtable *ht = *ht_ptr;
        ht_deinit(ht);
        free(ht);
        *ht_ptr = NULL;
    }
}


typedef struct HTIterator {
    const Hashtable *ht;
    unsigned int bucket_idx;
    HTNode *curr_node;
} HTIterator;

HTIterator* ht_iterator_init(Hashtable *ht) {
    if (!ht) {
        fprintf(stderr, "To initialize an iterator you need a valid hashtable pointer\n");
        return NULL;
    }

    HTIterator *iter = malloc(sizeof(HTIterator));
    if (!iter) {
        fprintf(stderr, "Failed to initialize hashtable iterator\n");
        return NULL;
    }
    iter->bucket_idx = 0;
    iter->curr_node = NULL;
    // iter->curr_node = (HTNode*)ht->arr[0];
    return iter;
}


HTNode* ht_iterator_next(HTIterator *iter) {
    HTNode *curr_node = iter->ht->arr[iter->bucket_idx]; // get LL head 
    if (!curr_node) {
        while (!curr_node) {
            curr_node = iter->ht->arr[++iter->bucket_idx];
        }
    }
    if (iter->bucket_idx == iter->ht->arr_cap && !curr_node->next) {
        return NULL;
    }
    iter->curr_node = curr_node->next;
    return curr_node;
}


static unsigned int hash_func(const void *key, size_t key_size) {
    return XXH32(key, key_size, 0);
}

inline bool is_even(int x) {
    return x % 2 == 0;
}

bool is_prime(unsigned int x) {
    if (x <= 1) return false;
    if (x == 2) return true;
    for (unsigned int i = 3; (i * i <= x); i += 2) {
        if (x % i == 0) {
            return false;
        }
    }
    return true;
}

unsigned int next_prime(unsigned int x) {
    if (x <= 2) return 2;
    if (is_even(x)) x++;
    while (!is_prime(x)) {
        x += 2;
    }
    return x;
}


/*
int main() {
    Hashtable *ht = ht_create(sizeof(int), sizeof(int));
    int in_arr[] = {0, 1, 2, 3, 4};
    for (int i = 0; i < 5; i++) {
        assert(ht_put(ht, &i, &i) == true);
    }
    printf("Passed test all test keys correctly ht_put into table\n");

    for (int i = 0; i < 5; i++) {
        assert(ht_contains(ht, &in_arr[i])== true);
    }
    printf("Passed test all original keys are contained in hashtable\n");



    ht_resize(ht, 40);
    for (int i = 0; i < 5; i++) {
        assert(ht_contains(ht, &in_arr[i])== true);
    }
    printf("Passed test all original keys are contained in hashtable EVEN AFTER ht_resize\n");


    ht_delete(ht, &in_arr[2]);
    assert(ht->count == 4);
    assert(!ht_contains(ht, &in_arr[2]));
    printf("Passed test, deletion of key in hashtable verified\n");


    ht_clear(ht);
    for (int i = 0; i < ht->arr_cap; i++) {
        HTNode *node = ht->arr[i];
        assert(node == NULL);
    }
    assert(ht->count == 0);
    printf("Succesfully verified ht_clear removes all nodes\n");

    for (int i = 0; i < 100; i++) {
        int temp = i + 1000;
        assert(ht_put(ht, &i, &temp) == true);
    }
    printf("Successfully put 100 new elements into previously cleared hashtable\n");

    int *x;
    for (int i = 0; i < 100; i++) {
        assert(ht_contains(ht, &i));
        x = ht_find(ht, &i);
        assert(ht);
        assert(*x == i + 1000);
        // printf("%d -> %d\n", i, *(x));
    } 
    printf("Successfully verified hashtable contains new 100 elements ht_contains\n");

    ht_destroy(ht);
    ht_destroy(ht);
    ht_destroy(ht);
    ht_destroy(ht);
    printf("Program survies multiple ht_destroy calls on the same hashtable\n");
    return 0;
} */
