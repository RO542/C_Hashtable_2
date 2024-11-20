run: build
	./ht

build: hashtable.c hashtable.h
	gcc hashtable.c -o ht

test:
	gcc -I./ ht_tests.c hashtable.c -o hashtable_test
	hashtable_test

