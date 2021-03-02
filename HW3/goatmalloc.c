#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include "goatmalloc.h"

#include <string.h>

void* _arena_start = NULL;
size_t _size = 0;
int statusno = 0;
struct __node_t* head = NULL;

int init(size_t size){
  size_t page_size = getpagesize();

  printf("Initializing arena:\n");
  printf("...requested size %lu bytes\n", size);

  // Make sure input is not negative
  if(size > MAX_ARENA_SIZE){
    printf("...error: requested size larger then MAX_ARENA_SIZE (%d)\n", MAX_ARENA_SIZE);
    return ERR_BAD_ARGUMENTS;
  }

  // Caculate the actual number of bytes allocated
  // ceiling division size/page_size * page_size
  printf("...pagesize is %ld bytes\n", page_size);
  size_t actual_size = ((size + page_size - 1)/page_size) * page_size;
  if(actual_size != size){
    printf("...adjusting size with page boundaries\n");
    printf("...adjusted size is %ld bytes\n", actual_size);
  }

  _size = actual_size;

  // Open file for mmap
  int fd = open("/dev/zero", O_RDWR|O_CREAT);
  if(fd < 0){
    printf("Failed to open file\n");
    return ERR_SYSCALL_FAILED;
  }

  printf("...mapping arena with mmap()\n");

  // Get memory with mmap
  _arena_start = mmap(NULL, _size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if(_arena_start < 0){
    printf("mmap failed\n");
    return ERR_SYSCALL_FAILED;
  }

  printf("...arena starts at %p\n", _arena_start);
  printf("...arena ends at %p\n", _arena_start+actual_size);

  // Close file descriptor
  int cls = close(fd);
  if(cls < 0){
    printf("Failed to close file descriptor\n");
    return ERR_SYSCALL_FAILED;
  }

  printf("...initializing header for initial free chunk\n");

  // Init the chunk list head
  head = _arena_start;
  head->size = actual_size - sizeof(struct __node_t);
  head->is_free = 1;
  head->fwd = NULL;
  head->bwd = NULL;

  printf("...header size is %ld bytes\n", sizeof(struct __node_t));

  return actual_size;
}

int destroy(){
  printf("Destroying Arena:\n");

  if(_arena_start == NULL){
    printf("...error: cannot destroy unintialized area. Setting error status\n");
    statusno = ERR_UNINITIALIZED;
    return ERR_UNINITIALIZED;
  }


  printf("...unmapping arena with munmap()\n");

  int ret = munmap(_arena_start, _size);
  if(ret < 0){
    printf("munmap failed\n");
    return ERR_SYSCALL_FAILED;
  }

  // Reset state variables
  _arena_start = NULL;
  _size = 0;
  head = NULL;

  return 0;
}

void* walloc(size_t size){
  // Check arena is initialized
  if(_arena_start == NULL){
    printf("...Error: Unitialized. Setting status code\n");
    statusno = ERR_UNINITIALIZED;
    return NULL;
  }

  printf("Allocating memory:\n");
  printf("...looking for free chunk of >= %ld bytes\n", size);

  // Walk chunk list and look for free space large enough
  // Assume all free spaces are coalesced
  struct __node_t* cur_node = head;
  char found = 'n';
  while(found != 'y'){

    // Found first free chunk large enough
    if(cur_node->is_free == 1 &&
      cur_node->size >= size){
        found = 'y';
        printf("...found free chunk of %ld bytes with header at %p\n",
        cur_node->size, cur_node);
    } else{
      // Go to next node
      cur_node = cur_node->fwd;
    }

    // End of list no more free space
    if(cur_node == NULL){
      printf("...no such free chunk exists\n");
      printf("...setting error code\n");
      statusno = ERR_OUT_OF_MEMORY;
      return NULL;
    }
  }

  printf("...free chunk->fwd currently points to %p\n", cur_node->fwd);
  printf("...free chunk->bwd currenlty points to %p\n", cur_node->bwd);

  printf("...checking of splitting is required\n");
  // if there is space for a new header
  if(cur_node->size-size >= sizeof(struct __node_t)){
    printf("...splitting free chunk\n");

    // Embed new header
    struct __node_t* new_header;
    new_header = ((void*)cur_node) + sizeof(struct __node_t) + size;
    new_header->is_free = 1;
    new_header->size = cur_node->size - size - sizeof(struct __node_t);
    new_header->fwd = cur_node->fwd;
    new_header->bwd = cur_node;

    // The allocated nodes next node
    if(cur_node->fwd != NULL){
      cur_node->fwd->bwd = new_header;
    }

    cur_node->fwd = new_header;

    printf("...updating chunk header at %p\n", cur_node);

    // Allocate the memory
    cur_node->is_free = 0;
    cur_node->size = size;

  } else if(cur_node->size == size){
    printf("...splitting not required\n");
    printf("...updating chunk header at %p\n", cur_node);

    // Allocate the memory
    cur_node->is_free = 0;
  } else{
    printf("...splitting not possible\n");
    printf("...updating chunk header at %p\n", cur_node);

    // Allocate the memory
    cur_node->is_free = 0;
  }

  printf("...being carful with my pointer arithmetic and void pointer casting\n");
  printf("...allocation starts at %p\n", cur_node+sizeof(struct __node_t));

  return ((void*)cur_node) + sizeof(struct __node_t);
}

void wfree(void *ptr){
  printf("Freeing allocated memory:\n");
  printf("...suplied pointer %p\n", ptr);

  printf("...being careful with my pointer arithmetic and void pointer casting\n");
  // Find the header
  struct __node_t* header = (struct __node_t*)(ptr - sizeof(struct __node_t));

  printf("...accessing chunk header at %p\n", header);
  header->is_free = 1;

  printf("...chunk of size %ld\n", header->size);

  printf("...checking if coalescing is needed\n");
  if(header->fwd != NULL && header->fwd->is_free == 1 &&
    header->bwd != NULL && header->bwd->is_free == 1){
    // coalesce with prev and next node
    printf("...col. case 1: previous, current, and next chunks all free.\n");

    // next node
    struct __node_t* next = header->fwd;
    if(next->fwd != NULL){
      next->fwd->bwd = header;
    }
    header->fwd = next->fwd;

    header->size += sizeof(struct __node_t) + next->size;

    // prev node
    struct __node_t* prev = header->bwd;
    prev->fwd = header->fwd;
    if(header->fwd != NULL){
      header->fwd->bwd = prev;
    }

    prev->size += sizeof(struct __node_t) + header->size;

    header = prev;
  }else if(header->fwd != NULL && header->fwd->is_free == 1){
    // coalesce with next node
    printf("...col. case 3: current and next chunks free.\n");
    struct __node_t* next = header->fwd;
    if(next->fwd != NULL){
      next->fwd->bwd = header;
    }
    header->fwd = next->fwd;

    header->size += sizeof(struct __node_t) + next->size;

  } else if(header->bwd != NULL && header->bwd->is_free == 1){
    // coalesce with previous node
    printf("...col. case 2: previous and current chunks free.\n");
    struct __node_t* prev = header->bwd;
    prev->fwd = header->fwd;

    prev->size += sizeof(struct __node_t) + header->size;
    if(header->fwd != NULL){
      header->fwd->bwd = prev;
    }

    header = prev;
  } else{
    printf("...coalescing not needed.\n");
  }
}
