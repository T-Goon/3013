# Project 3 Memory Allocation

A custom implementation and test harness of malloc() and free() which allocates/frees a specified amount of memory.

The program manages memory requests by using embedded metadata headers in an mmap()ed chunk of memory.

![image](https://user-images.githubusercontent.com/32044950/120710245-e962da00-c48b-11eb-86f2-fce8816a7b94.png)

More details on the assignment can be found in "HW3.pdf".

## Usage
### Compile
Place
- goatmalloc.c,
- goatmalloc.h,
- Makefile, and
- test_goatmalloc.c
into the same directory.

`make`: same as `make all`

`make all`: same as `make test`

`make test`: compiles project and creates "test_goatmalloc"

`make clean`: deletes "test_goatmalloc"

### Running
Do `./test_goatmalloc` to run the all test cases for the project.
