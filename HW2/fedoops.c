#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_PACKAGES 20
#define MAX_INSTR 4
#define NUM_STATIONS 4
#define TEAM_SIZE 10
#define MAX_WAIT 3

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
};

// The package
struct package{
  int id;
  struct instrNode* instrListHead;
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

sem_t matrix_lock;
int station_matrix[NUM_STATIONS][NUM_STATIONS];

sem_t counter_lock;
int num_since_red = 0;
int num_since_blue = 0;
int num_since_green = 0;
int num_since_yellow = 0;

char* get_type_string(enum color c){
  switch (c) {
    case RED:
      return "red";
    case BLUE:
      return "blue";
    case GREEN:
      return "green";
    case YELLOW:
      return "yellow";
  }
}

char* get_instruction_string(enum instr i){
  switch (i) {
    case WEIGH:
      return "weigh";
    case BARCODE:
      return "barcode";
    case XRAY:
      return "x-ray";
    case JOSTLE:
      return "jostle";
  }
}

char* get_station_string(enum instr i){
  switch (i) {
    case WEIGH:
      return "scale";
    case BARCODE:
      return "barcoder";
    case XRAY:
      return "x-ray";
    case JOSTLE:
      return "shaker";
  }
}

void shout_package(int id, enum color type, struct package* pkg){
  char* color = get_type_string(type);

  printf("[%s #%d]: Picked package #%d with instruction(s):", color, id, pkg->id);

  struct instrNode* node = pkg->instrListHead;

  while(node != NULL){
    char* instruction = get_instruction_string(node->instruction);

    printf(" %s", instruction);

    node = node->next;
  }

  printf("\n");
}

void shout_start(int id, enum color type, struct package* pkg, enum instr instr) {
  char* color = get_type_string(type);

  char* instruction = get_station_string(instr);

  printf("[%s #%d]: Started working on package #%d at station %s.\n",
  color, id, pkg->id, instruction);
}

void shout_end(int id, enum color type, struct package* pkg, enum instr instr){
  char* color = get_type_string(type);

  char* instruction = get_station_string(instr);

  printf("[%s #%d]: Finished working on package #%d at station %s.\n",
  color, id, pkg->id, instruction);
}

void get_team_lock(enum color c){
  // Get the lock for the team
  switch (c) {
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

void release_team_lock(enum color c){
  // Release the team lock
  switch (c) {
    case RED:
      sem_post(&red_sem);
      break;
    case BLUE:
      sem_post(&blue_sem);
      break;
    case GREEN:
      sem_post(&green_sem);
      break;
    case YELLOW:
      sem_post(&yellow_sem);
      break;
  }
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

void release_station_lock(enum instr instruction){
  switch (instruction) {
    case WEIGH:
      sem_post(&scale_lock);
      break;
    case BARCODE:
      sem_post(&barcoder_lock);
      break;
    case XRAY:
      sem_post(&xray_lock);
      break;
    case JOSTLE:
      sem_post(&shaker_lock);
      break;
  }
}

void update_counters(enum color type){
  sem_wait(&counter_lock);
  switch (type) {
    case RED:
      num_since_red ++;
      num_since_blue = 0;
      num_since_green = 0;
      num_since_yellow = 0;
      break;
    case BLUE:
      num_since_red = 0;
      num_since_blue ++;
      num_since_green = 0;
      num_since_yellow = 0;
      break;
    case GREEN:
      num_since_red = 0;
      num_since_blue = 0;
      num_since_green ++;
      num_since_yellow = 0;
      break;
    case YELLOW:
      num_since_red = 0;
      num_since_blue = 0;
      num_since_green = 0;
      num_since_yellow ++;
      break;
  }

  sem_post(&counter_lock);
}

int check_counter(enum color type){
  sem_wait(&counter_lock);

  switch (type) {
    case RED:
      if(num_since_red >= MAX_WAIT){
        sem_post(&counter_lock);
        pthread_yield();
      return 0;
      }
    case BLUE:
    if(num_since_blue >= MAX_WAIT){
      sem_post(&counter_lock);
      pthread_yield();
      return 0;
      }
    case GREEN:
    if(num_since_green >= MAX_WAIT){
      sem_post(&counter_lock);
      pthread_yield();
      return 0;
      }
    case YELLOW:
    if(num_since_yellow >= MAX_WAIT){
      sem_post(&counter_lock);
      pthread_yield();
      return 0;
      }
    default:
      sem_post(&counter_lock);
      return 1;
  }

  sem_post(&counter_lock);
  return 1;
}

void remove_from_matrix(int old, int new){
  // Remove a finished connection from the matrix
  sem_wait(&matrix_lock);

  station_matrix[old][new] -= 1;

  sem_post(&matrix_lock);
}

/*
  Checks for a cycle in the graph recursively.

  return: 0 if there is a cycle, 1 if there is no cycle
*/
int check_cycle(int matrix[NUM_STATIONS][NUM_STATIONS],
   int visited[NUM_STATIONS], int count, int start, int end){

  // Add current node to visited count
  visited[count] = start;
  count ++;

  // Check if the node has been visisted
  for(int i=0; i<count; i++){
    if(visited[i] == end){
      // There is a cycle
      return 0;
    }
  }

  // Visited all stations without finding any cycles
  if(count == NUM_STATIONS){
    return 1;
  }

  // Check all connections in the 'end' row
  int flag = 0;
  for(int i=0; i<NUM_STATIONS; i++){

    if(end != i){
      if(matrix[end][i] > 0){
        flag = 1;
        return check_cycle(matrix, visited, count, end, i);
      }
    }

  }

  // No connections out of 'end' row, no cycle
  if(!flag){
    return 1;
  }
}

int check_matrix(struct instrNode* node){
  // Get the matrix lock
  sem_wait(&matrix_lock);

  // Make a copy of the matrix
  int copy[NUM_STATIONS][NUM_STATIONS];
  for(int i=0; i<NUM_STATIONS; i++){
    for(int j=0; j<NUM_STATIONS; j++){
      copy[i][j] = station_matrix[i][j];
    }
  }

  // printf("copy\n");
  // for(int i=0; i<NUM_STATIONS; i++){
  //   for(int j=0; j<NUM_STATIONS; j++){
  //     printf("%d ", copy[i][j]);
  //   }
  //   printf("\n");
  // }

  // Add the new connections to the copy
  struct instrNode* curNode = node;

  // Only one instruction no fear of deadlock
  if(curNode->next == NULL){
    sem_post(&matrix_lock);
    return 1;
  }

  struct instrNode* nextNode = curNode->next;
  while(nextNode != NULL){
    copy[curNode->instruction][nextNode->instruction] += 1;

    curNode = nextNode;
    nextNode = curNode->next;
  }

  // printf("copy\n");
  // for(int i=0; i<NUM_STATIONS; i++){
  //   for(int j=0; j<NUM_STATIONS; j++){
  //     printf("%d ", copy[i][j]);
  //   }
  //   printf("\n");
  // }

  // Check for cyles
  for(int i=0; i<NUM_STATIONS; i++){
    for(int j=0; j<NUM_STATIONS; j++){

      int counter = 0;
      int visited[NUM_STATIONS] = {-1, -1, -1, -1};
      if(i != j){ // From station to itself

        int found;
        if(copy[i][j] > 0)
          found = check_cycle(copy, visited, counter, i, j);

        if(found == 0){
          // Found a cycle
          sem_post(&matrix_lock);
          return 0;
        }

      }
    }
  }

  // There are no cycles
  // Set the matrix as the new matrix
  for(int i=0; i<NUM_STATIONS; i++){
    for(int j=0; j<NUM_STATIONS; j++){
      station_matrix[i][j] = copy[i][j];
    }
  }

  sem_post(&matrix_lock);

  return 1;
}

void* worker(void* args){
  int id = ((struct workerArgs*)args)->id;
  enum color type = ((struct workerArgs*)args)->c;

  while(1){
    // Try to get the team lock
    get_team_lock(type);

    sleep(1);

    // Lock queue
    sem_wait(&pkg_q_lock);
    if(head == NULL){

      // Unlock queue
      sem_post(&pkg_q_lock);
      release_team_lock(type);
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
    struct instrNode* curInstr = my_package->instrListHead;

    // Print after getting a package
    shout_package(id, type, my_package);

    // Try to add connections to the matix
    int check = 0;
    while(!check){
      check = check_matrix(curInstr);
      if(!check){
        pthread_yield();
      }
    }

    // Process the package
    enum instr old_instruction  = curInstr->instruction;
    enum instr new_instruction;
    struct instrNode* oldInstr;
    get_station_lock(old_instruction);
    while(1){

      shout_start(id, type, my_package, old_instruction);

      // Work for [1,5] seconds
      int time = (rand() % 3) + 1;
      printf("[%s #%d] Working for %d seconds.\n", get_type_string(type), id, time);
      sleep(time);

      shout_end(id, type, my_package, old_instruction);

      oldInstr = curInstr;
      curInstr = curInstr->next;
      free(oldInstr);

      if(curInstr == NULL){
        release_station_lock(old_instruction);
        break;
      }
      new_instruction = curInstr->instruction;

      get_station_lock(new_instruction);
      remove_from_matrix(old_instruction, new_instruction);
      release_station_lock(old_instruction);

      old_instruction = new_instruction;
    }

    printf("[%s #%d]: Finished processing package #%d.\n",
    get_type_string(type), id, my_package->id);

    // Pass lock to next team member
    release_team_lock(type);
    pthread_yield();

    free(my_package);
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

      head = nextNode;
      tail = nextNode;

    } else{
      struct instrNode* nextNode = malloc(sizeof(struct instrNode));
      nextNode->instruction = instrEnum;

      tail->next = nextNode;
      tail = nextNode;
    }

  }

  // Put the instruction list in the package
  pkg->instrListHead = head;
}

void create_worker(int ids[TEAM_SIZE], enum color type, pthread_t threads[TEAM_SIZE]){

  for(int i=0; i<TEAM_SIZE; i++){
    pthread_t thread;
    ids[i] = i;

    struct workerArgs* arg = malloc(sizeof(struct workerArgs));
    arg->id = ids[i];
    arg->c = type;

    pthread_create(&thread, NULL, worker, (void*)arg);
    threads[i] = thread;
  }
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
  sem_init(&matrix_lock, 0, 1);
  sem_init(&counter_lock, 0, 1);

  // Init the adjacency matrix
  for(int i=0; i<NUM_STATIONS; i++){
    for(int j=0; j<NUM_STATIONS; j++){
      if(i == j){
        station_matrix[i][j] = 1;
      } else {
        station_matrix[i][j] = 0;
      }
    }
  }

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

  // Create the worker threads
  int red_ids[TEAM_SIZE];
  pthread_t red_threads[TEAM_SIZE];
  create_worker(red_ids, RED,red_threads);

  int blue_ids[TEAM_SIZE];
  pthread_t blue_threads[TEAM_SIZE];
  create_worker(blue_ids, BLUE, blue_threads);

  int green_ids[TEAM_SIZE];
  pthread_t green_threads[TEAM_SIZE];
  create_worker(green_ids, GREEN, green_threads);

  int yellow_ids[TEAM_SIZE];
  pthread_t yellow_threads[TEAM_SIZE];
  create_worker(yellow_ids, YELLOW, yellow_threads);

  for(int i=0; i<TEAM_SIZE; i++){
    pthread_join(red_threads[i], NULL);
    pthread_join(blue_threads[i], NULL);
    pthread_join(green_threads[i], NULL);
    pthread_join(yellow_threads[i], NULL);
  }

  printf("All the packages have been processed~.\n");

  // // Print out the package queue
  // struct pkgQueueNode* curNode = head;
  // for(int i=0; i<NUM_PACKAGES; i++){
  //
  //
  //   printf("Pkg ID: %d, Instr: ", curNode->pkg->id);
  //   struct instrNode* curInstr = curNode->pkg->instrListHead;
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
