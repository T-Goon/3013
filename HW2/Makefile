all: summer.c fedoops.c
	gcc -pthread -o summer summer.c
	gcc -pthread -o fedoops fedoops.c

summer: summer.c
	gcc -pthread -o summer summer.c

fedoops: fedoops.c
	gcc -pthread -o fedoops fedoops.c

clean:
	rm -f summer fedoops
