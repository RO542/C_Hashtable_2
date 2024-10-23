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
    for (int i = 0; i < ht->arr_cap; i++) {
        ht->arr[i] = NULL;
    }
    return true;
}


Hashtable *ht_create(size_t key_size, size_t value_size) {
    Hashtable *ht = (Hashtable *)malloc(sizeof(Hashtable));
    if (!ht)  {
        fprintf(stderr, "Failed to allocate hashtable during ht_create\n");
        return false;
    }
    if (!ht_init(ht, key_size, value_size)) {
        fprintf(stderr, "Failed to call ht_init hashtable during ht_create\n");
        return false;
    }
    return ht;
}

HTNode *ht_create_node(Hashtable *ht, void *key, void *value) {
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
    memcpy(new_node->key, key, ht->key_size);

    new_node->value = malloc(ht->value_size);
    if (!new_node->value) {
        free(new_node->key);
        free(new_node);
        fprintf(stderr, "Failed to allocate new value during ht_put\n");
        return NULL;
    }
    memcpy(new_node->value, value, ht->value_size);
    return new_node;
}

bool ht_put(Hashtable *ht, void *key, void *value) {
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
    new_node->stored_hash = key_hash;
    new_node->next = ht->arr[bucket_idx];
    ht->arr[bucket_idx] = new_node;
    ht->count++;
    return true;
}

bool ht_resize(Hashtable *ht, unsigned int new_size) {
    HTNode *old_arr = *ht->arr;
    unsigned int old_cap = ht->arr_cap;
    unsigned int new_cap = next_prime(new_size); //TODO: warn if new_size is too low

    ht->arr = (HTNode **)malloc(new_cap * sizeof(HTNode *));
    if (!ht->arr) {
        fprintf(stderr, "ht_resize, failed to allocate new ptr  to arr of buckets, old ht preserved\n");
        return false;
    }
    ht->count = 0;
    ht->arr_cap = new_cap;

    for (int i = 0; i < ht->arr_cap; i++) {
        ht->arr[i] = NULL;
    }

    for (int i = 0; i < old_cap; i++) {
        HTNode *curr_node;
        while (curr_node) {
            HTNode *next_node = curr_node->next;
            if (!ht_put(ht, curr_node->key, curr_node->value)) {
                fprintf(stderr, "ht_put failed in ht_resize\n");
                return false; // maybe just nuke the whole table
            }
            curr_node = next_node;
        }
    }
    free(old_arr);
    return true;
}

void *ht_find(Hashtable *ht, void *key) {
    unsigned int key_hash = hash_func(key, ht->key_size);
    unsigned int bucket_idx = key_hash % ht->arr_cap;
    for (HTNode *curr_node = ht->arr[bucket_idx]; curr_node != NULL; curr_node = curr_node->next) {
        if (key_hash == curr_node->stored_hash && memcmp(key, curr_node->key, ht->key_size) == 0) {
            return curr_node->value;
        }
    }
    return NULL;
}

bool ht_contains(Hashtable *ht, void *key) {
    return (ht_find(ht, key) != NULL);
}

void ht_destroy_node(HTNode *node) {
    if (!node) return;
    // if a node exists in this table then it must have  key/val
    free(node->key);
    free(node->value);
    free(node);
    node->key = NULL;
    node->value = NULL;
    node = NULL;
}


void ht_delete(Hashtable *ht, void *key) {
    if (ht_empty(ht)) {
        fprintf(stderr, "Unable to remvoe key from empty Hashtable\n");
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
        ht->arr[i] = NULL; // redundant ?

    }
    ht->count = 0;
}

void ht_deinit(Hashtable *ht) {
    ht_clear(ht);
    free(ht->arr);
    ht->arr = NULL;
}

bool ht_empty(Hashtable *ht) {
    return ht->count == 0;
}

void ht_destroy(Hashtable **ht_ptr) {
    if (ht_ptr && (*ht_ptr)) {
        Hashtable *ht = *ht_ptr;
        ht_deinit(ht);
        free(ht);
        *ht_ptr = NULL;
    }
}

unsigned int hash_func(void *key, size_t key_size) {
    return XXH32(key, key_size, 0); 
}

inline bool is_even(int x) {
    return x % 2 == 0;
}

bool is_prime(unsigned int x) {
    if (x <= 1) return false;
    if (x == 2) return true;
    for (unsigned int i = 3; (i * i < x); i += 2) {
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
    int key = 42;
    int value = 24;
    int new_value = 48;
    int non_existent_key = 100;

    // Test ht_put
    assert(ht_put(ht, &key, &value));
    assert(ht->count == 1);

    // Test ht_find
    void *found_value = ht_find(ht, &key);
    assert(found_value != NULL);
    assert(*(int *)found_value == value);

    // Test ht_update
    assert(ht_put(ht, &key, &new_value));
    found_value = ht_find(ht, &key);
    assert(found_value != NULL);
    assert(*(int *)found_value == new_value);

    // Test ht_contains
    assert(ht_contains(ht, &key));
    assert(!ht_contains(ht, &non_existent_key));

    // Test ht_delete
    ht_delete(ht, &key);
    assert(ht->count == 0);
    assert(!ht_contains(ht, &key));

    // Test ht_clear
    ht_clear(ht);
    assert(ht->count == 0);

    // Test ht_empty
    assert(ht_empty(ht));
    ht_destroy(&ht);
    return 0;
} */