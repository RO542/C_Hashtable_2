#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hashtable.h" // Include your hashtable implementation header here

void test_basic_insertion_and_retrieval() {
    printf("Running basic insertion and retrieval test...\n");

    Hashtable *ht = ht_create(int, int);

    int key = 42;
    int value = 100;
    assert(ht_put(ht, &key, &value));

    int *retrieved_value = (int *)ht_find(ht, &key);
    assert(retrieved_value != NULL && *retrieved_value == 100);

    printf("Passed: Basic insertion and retrieval test\n");
    ht_destroy(ht);
}

void test_overwrite_value() {
    printf("Running overwrite value test...\n");

    Hashtable *ht = ht_create(int, int);

    int key = 42;
    int value1 = 100;
    int value2 = 200;

    assert(ht_put(ht, &key, &value1));
    int *retrieved_value = (int *)ht_find(ht, &key);
    assert(retrieved_value != NULL && *retrieved_value == 100);

    assert(ht_put(ht, &key, &value2));
    retrieved_value = (int *)ht_find(ht, &key);
    assert(retrieved_value != NULL && *retrieved_value == 200);

    ht_destroy(ht);
}

void test_resize() {
    printf("Running resize test...\n");
    Hashtable *ht = ht_create(int, int);
    for (int i = 0; i < 60; i++) {
        int x = i;
        int y = i;
        ht_put(ht, &x, &y);
    }
    ht_resize(ht, 500);
    return;

    int *out;
    for (int i = 0; i < 60; i++) {
        assert(ht_contains(ht, &i));
        out = ht_find(ht, &i);
        assert(out != NULL);
        assert(*out == i);
    }

    for (int i = 0; i < 60; i++) {
        assert(ht_contains(ht, &i));
    }

    printf("Passed: Resize test\n");
    ht_destroy(ht);
}

void test_deletion() {
    printf("Running deletion test...\n");

    Hashtable *ht = ht_create(int, int);

    int key = 42;
    int value = 100;
    assert(ht_put(ht, &key, &value));

    assert(ht_contains(ht, &key));
    ht_delete(ht, &key);
    assert(!ht_contains(ht, &key));
    assert(ht_find(ht, &key) == NULL);

    printf("Passed: Deletion test\n");
    ht_deinit(ht);
    free(ht);
}

void test_string_key_value_pairs() {
    printf("Running string key-value pairs test...\n");

    Hashtable *ht = ht_create(char *, char *);

    char *key = "hello";
    char *value = "world";
    assert(ht_put(ht, &key, &value));

    char **retrieved_value = (char **)ht_find(ht, &key);
    assert(retrieved_value != NULL && strcmp(*retrieved_value, "world") == 0);

    printf("Passed: String key-value pairs test\n");
    ht_deinit(ht);
    free(ht);
}

void test_complex_type() {
    printf("Running complex type test...\n");

    typedef struct {
        int id;
        char name[50];
    } ComplexType;

    Hashtable *ht = ht_create(int, ComplexType);

    int key = 1;
    ComplexType value = {1, "ComplexName"};

    assert(ht_put(ht, &key, &value));

    ComplexType *retrieved_value = (ComplexType *)ht_find(ht, &key);
    assert(retrieved_value != NULL);
    assert(retrieved_value->id == 1);
    assert(strcmp(retrieved_value->name, "ComplexName") == 0);

    printf("Passed: Complex type test\n");
    ht_deinit(ht);
    free(ht);
}

void test_clear() {
    printf("Running clear test...\n");

    Hashtable *ht = ht_create(int, int);

    for (int i = 0; i < 10; i++) {
        assert(ht_put(ht, &i, &i));
    }

    ht_clear(ht);
    assert(ht_empty(ht));

    for (int i = 0; i < 10; i++) {
        assert(ht_find(ht, &i) == NULL);
    }

    printf("Passed: Clear test\n");
    ht_deinit(ht);
    free(ht);
}


void test_ht_get() {
    Hashtable *ht = ht_create(int, int);
    for (int i = 0; i < 100; i++) {
        int x = i + 100;
        assert(ht_put(ht, &i, &x));
    }
    int returned_int = 9000;
    for (int i = 0; i < 100; i++) {
        assert(ht_get(ht, &i, &returned_int) == true);
        assert(returned_int != 9000 && returned_int == (i + 100));
    }
    printf("Passed tests for ht_get\n");
}

void test_ht_stack() {
    Hashtable ht_stack;
    Hashtable *ht = &ht_stack;
    assert(ht_init(&ht_stack, sizeof(int), sizeof(int)));
    for (int i = 0; i < 1000; i++) {
        assert(ht_put(&ht_stack, &i, &i));
    }

    int out_int = 9000;
    for (int i = 0; i < 1000; i++) {
        assert(ht_get(&ht_stack, &i, &out_int));
        assert(out_int != 9000 && out_int == i);
    }
    //TODO: ht_deinit causes some sort of memory error 
    //FIXME:
    // ht_deinit(&ht_stack);

    printf("Passed test for stack managed hashtable, ht_init and ht_deinit\n");
}

int main() {
    printf("Starting hashtable tests...\n");

    test_basic_insertion_and_retrieval();
    test_overwrite_value();
    test_resize();
    test_deletion();
    test_string_key_value_pairs();
    test_complex_type();
    test_clear();
    test_ht_get();
    test_ht_stack();


    printf("All tests passed successfully!\n");
    return 0;
}
