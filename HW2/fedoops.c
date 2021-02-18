#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_PACKAGES 20
#define MAX_INSTR 4
#define TEAM_SIZE 10

// Instructions
enum instr{
  WEIGH,
  BARCODE,
  XRAY,
  JOSTLE
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
  struct instrNode* prev;
};

// The package
struct package{
  int id;
  struct instrNode* instrListHead;
  struct instrNode* instrListTail;
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

void get_station_lock(enum instr instruction){
  switch (instruction) {
    case WEIGH:
      sem_wait(&scale_lock);
      break;
    case BARCODE:
      sem_wait(&barcoder_lock);
      break;
    case XRAY:
      sem_wait(&xray_lock);
      break;
    case JOSTLE:
      sem_wait(&shaker_lock);
      break;
  }
}

void* worker(void* args){
  int id = ((struct workerArgs*)args)->id;
  enum color type = ((struct workerArgs*)args)->c;

  while(1){
    // Try to get the team lock
    while(1){
      switch (type) {
        case RED:
          sem_wait(&red_sem);
          break;
        case BLUE:
          sem_wait(&blue_sem);
          break;
        case GREEN:
          sem_wait(&green_sem);
          break;
        case YELLOW:
          sem_wait(&yellow_sem);
          break;
      }
    }

    // Lock queue
    sem_wait(&pkg_q_lock);
    if(head == NULL){

      // Unlock queue
      sem_post(&pkg_q_lock);
      break;
    }

    // Get node from queue
    struct pkgQueueNode* pkg_node = head;
    head = head->next;

    // Unlock queue
    sem_post(&pkg_q_lock);

    // unpack the package
    struct package* my_package = pkg_node->pkg;
    free(pkg_node);

    // Get first station lock
    struct instrNode* curInstr = my_package->instrListHead;

    enum instr instruction  = curInstr->instruction;
    get_station_lock(instruction);

    while(curInstr != NULL){
      // do work

      // Get the next station lock
      curInstr = curInstr->next;

      enum instr instruction2  = curInstr->instruction;
      get_station_lock(instruction2);

      // Get
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
      nextNode->next = NULL;
      nextNode->prev = NULL;

      head = nextNode;
      tail = nextNode;

    } else{
      struct instrNode* nextNode = malloc(sizeof(struct instrNode));
      nextNode->instruction = instrEnum;

      tail->next = nextNode;
      nextNode->prev = tail;
      tail = nextNode;
    }

  }

  // Put the instruction list in the package
  pkg->instrListHead = head;
  pkg->instrListTail = tail;
}

void create_worker(int ids[TEAM_SIZE], enum color type){

  for(int i=0; i<TEAM_SIZE; i++){
    pthread_t thread;
    ids[i] = i;

    struct workerArgs* arg = malloc(sizeof(struct workerArgs));
    arg->id = ids[i];
    arg->c = type;

    pthread_create(&thread, NULL, worker, (void*)arg);
  }
}

/*TODO
Solution: If the path of a package does not backtrack the path of any other
currenlty processing package then it is allowed to pick up the first lock for
its station and start.
 */
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

  // // Create the worker threads
  // int red_ids[TEAM_SIZE];
  // create_worker(red_ids, RED);
  //
  // int blue_ids[TEAM_SIZE];
  // create_worker(blue_ids, BLUE);
  //
  // int green_ids[TEAM_SIZE];
  // create_worker(green_ids, GREEN);
  //
  // int yellow_ids[TEAM_SIZE];
  // create_worker(yellow_ids, YELLOW);

  // Print out the package queue
  struct pkgQueueNode* curNode = head;
  for(int i=0; i<NUM_PACKAGES; i++){


    printf("Pkg ID: %d, Instr: ", curNode->pkg->id);
    struct instrNode* curInstr = curNode->pkg->instrListHead;
    while(1){
      printf("%d ", curInstr->instruction);

      if(curInstr->next == NULL){
        break;
      } else{
        curInstr = curInstr->next;
      }
    }

    curNode = curNode->next;

    printf("\n");
  }

  return 0;
}
