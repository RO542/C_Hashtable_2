run: build
	./ht

build: hashtable.c hashtable.h
	gcc hashtable.c -o ht

run_tests: build_tests
	./ht_tests	

build_tests:
	gcc -I./ ht_tests.c hashtable.c -o ht_tests 

