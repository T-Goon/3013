#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_PACKAGES 20
#define MAX_INSTR 4

// Instructions
enum instr{
  WEIGH,
  BARCODE,
  XRAY,
  JOSTLE,
  DONE
};

// worker types
enum color{
  RED,
  BLUE,
  GREEN,
  YELLOW
};

// Linked list node for the package instructions
struct instrNode{
  enum  instr instruction;
  struct instrNode* next;
};

// The package
struct package{
  int id;
  struct instrNode* instrList;
};

// Linked list node for the packages
struct pkgQueueNode{
  struct package* pkg;
  struct pkgQueueNode* next;
};

// The args for the worker threads
struct workerArgs{
  int id;
  enum color c;
};

sem_t pkg_q_lock;
sem_t scale_lock;
sem_t barcoder_lock;
sem_t xray_lock;
sem_t shaker_lock;

sem_t red_sem;
sem_t blue_sem;
sem_t yellow_sem;
sem_t green_sem;

struct pkgQueueNode* head = NULL;

void doWork(){

}

void* worker(void* args){
  int id = ((struct workerArgs*)args)->id;
  enum color type = ((struct workerArgs*)args)->c;

  while(1){
    // Lock queue
    sem_wait(&pkg_q_lock);
    if(head == NULL){

      // Unlock queue
      sem_post(&pkg_q_lock);
      break;
    }

    // Get package from queue
    struct packge* my_package = head;
    head = head->next;

    // Unlock queue
    sem_post(&pkg_q_lock);

    // Get next instruction
    struct instrNode* curInstr = my_package->instrList;
    while(curInstr != NULL){
      // do work
    }

  }

  return NULL;
}

void fillPkg(struct package* pkg){
  int num_instr = (rand() % 4) + 1;
  int list[] = {1,2,3,4};
  int numLeft = MAX_INSTR;
  struct instrNode* head = NULL;
  struct instrNode* tail = NULL;

  for(int i=0; i<num_instr; i++){
    int index = (rand() % numLeft);
    enum instr instrEnum = -1;

    // Pick instruction from list
    int new_instruction = list[index];
    // Remove instruction from list
    for(int j=index; j<MAX_INSTR-1; j++){
      list[j] = list[j+1];
    }
    numLeft --;

    switch (new_instruction) {
      case 1:
        instrEnum = WEIGH;
        break;
      case 2:
        instrEnum = BARCODE;
        break;
      case 3:
        instrEnum = XRAY;
        break;
      case 4:
        instrEnum = JOSTLE;
        break;
    }

    // Add instruction to list
    if(head == NULL && tail == NULL){
      struct instrNode* nextNode = malloc(sizeof(struct instrNode));
      nextNode->instruction = instrEnum;

      head = nextNode;
      tail = nextNode;

    } else{
      struct instrNode* nextNode = malloc(sizeof(struct instrNode));
      nextNode->instruction = instrEnum;

      tail->next = nextNode;
      tail = nextNode;
    }

  }

  struct instrNode* nextNode = malloc(sizeof(struct instrNode));
  nextNode->instruction = DONE;

  tail->next = nextNode;
  tail = nextNode;

  // Put the instruction list in the package
  pkg->instrList = head;
}

int main(void){
  FILE* seed_file = fopen("seed.txt", "r");
  int seed = -1;
  fscanf(seed_file, "%d", &seed);

  srand(seed);

  // Init semaphores
  sem_init(&pkg_q_lock, 0, 1);
  sem_init(&scale_lock, 0, 1);
  sem_init(&barcoder_lock, 0, 1);
  sem_init(&xray_lock, 0, 1);
  sem_init(&shaker_lock, 0, 1);
  sem_init(&red_sem, 0, 1);
  sem_init(&blue_sem, 0, 1);
  sem_init(&green_sem, 0, 1);
  sem_init(&yellow_sem, 0, 1);

  // Create the pile of packages
  struct pkgQueueNode* tail = NULL;
  for(int i=0; i<NUM_PACKAGES; i++){
    if(head == NULL && tail == NULL){
      struct package* pkg = malloc(sizeof(struct package));
      // Fill packge with instructions
      fillPkg(pkg);
      pkg->id = i;

      struct pkgQueueNode* nextQNode = malloc(sizeof(struct pkgQueueNode));
      nextQNode->pkg = pkg;
      nextQNode->next = NULL;

      head = nextQNode;
      tail = nextQNode;
    } else{
      struct package* pkg = malloc(sizeof(struct package));
      // Fill packge with instructions
      fillPkg(pkg);
      pkg->id = i;

      struct pkgQueueNode* nextQNode = malloc(sizeof(struct pkgQueueNode));
      nextQNode->pkg = pkg;

      tail->next = nextQNode;
      tail = nextQNode;
    }
  }

  // // Print out the package queue
  // struct pkgQueueNode* curNode = head;
  // for(int i=0; i<NUM_PACKAGES; i++){
  //
  //
  //   printf("Pkg ID: %d, Instr: ", curNode->pkg->id);
  //   struct instrNode* curInstr = curNode->pkg->instrList;
  //   while(1){
  //     printf("%d ", curInstr->instruction);
  //
  //     if(curInstr->next == NULL){
  //       break;
  //     } else{
  //       curInstr = curInstr->next;
  //     }
  //   }
  //
  //   curNode = curNode->next;
  //
  //   printf("\n");
  // }

  return 0;
}
