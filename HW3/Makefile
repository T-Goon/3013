all: goatmalloc.c goatmalloc.h test_goatmalloc.c
	gcc -o test_goatmalloc test_goatmalloc.c goatmalloc.c

test: goatmalloc.c goatmalloc.h test_goatmalloc.c
	gcc -o test_goatmalloc test_goatmalloc.c goatmalloc.c

clean:
	rm -f test_goatmalloc
