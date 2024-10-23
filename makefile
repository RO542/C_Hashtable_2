run: build
	./ht

build: hashtable.c hashtable.h
	gcc hashtable.c -o ht



