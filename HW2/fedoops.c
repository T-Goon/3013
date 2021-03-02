#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

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

sem_t move_lock;
sem_t matrix_lock;
int station_matrix[NUM_STATIONS][NUM_STATIONS];
sem_t counter_lock;
int rc = 0;
int bc = 0;
int gc = 0;
int yc = 0;

// Return the worker type as a string
// param c: The color of the worker
// return: The sting cooresponding to the worker type.
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

// Return the package instruction as a string
// param i: The instruction enum.
// return: The string cooresponding to the instruction.
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

// Return the station type based on the instruction
// param i: The instruction enum
// return: The string cooresponding to the station that the instruction is to be
// carried out at.
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

// Print out information when a worker gets a package.
// param id: The id of the worker.
// param type: The color of the worker.
// param pkg: The package that the worker just picked up.
void shout_package(int id, enum color type, struct package* pkg){
  char* color = get_type_string(type);

  printf("[%s #%d]: Picked package #%d with instruction(s):", color, id, pkg->id);

  struct instrNode* node = pkg->instrListHead;

  // Print out all instructions on the package
  while(node != NULL){
    char* instruction = get_instruction_string(node->instruction);

    printf(" %s", instruction);

    node = node->next;
  }

  printf("\n");
}

// Print out information when the worker starts working at a station.
// param id: The id of the worker.
// param type: The color of the worker.
// param pkg: The package that the worker is working on.
// param instr: The instruction that the worker is currently executing.
void shout_start(int id, enum color type, struct package* pkg, enum instr instr) {
  char* color = get_type_string(type);

  char* instruction = get_station_string(instr);

  printf("[%s #%d]: Started working on package #%d at station %s.\n",
  color, id, pkg->id, instruction);
}

// Print out information when the worker finishes working at a station.
// param id: The id of the worker.
// param type: The color of the worker.
// param pkg: The package the worker is currently working on.
// param instr: The instruction the worker just finished.
void shout_end(int id, enum color type, struct package* pkg, enum instr instr){
  char* color = get_type_string(type);

  char* instruction = get_station_string(instr);

  printf("[%s #%d]: Finished working on package #%d at station %s.\n",
  color, id, pkg->id, instruction);
}

// Down the semaphore that cooresponds to a certian team.
// param c: The color of the team.
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

// Up the semaphore that cooresponds to a team.
// param c: The color of the team.
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

// Up the semaphore that cooresponds to a work station.
// param instruction: The instuction that needs to be carried out at the station.
int get_station_lock(enum instr instruction){
  int ret = -1;
  switch (instruction) {
    case WEIGH:
      ret = sem_trywait(&scale_lock);
      if(ret == EAGAIN){
        sem_post(&move_lock);
      } else if(ret == 0 ){
        return 1;
      }
      return 0;
    case BARCODE:
      ret = sem_trywait(&barcoder_lock);
      if(ret == EAGAIN){
        sem_post(&move_lock);
      } else if(ret == 0 ){
        return 1;
      }
      return 0;
    case XRAY:
      ret = sem_trywait(&xray_lock);
      if(ret == EAGAIN){
        sem_post(&move_lock);
      } else if(ret == 0 ){
        return 1;
      }
      return 0;
    case JOSTLE:
      ret = sem_trywait(&shaker_lock);
      if(ret == EAGAIN){
        sem_post(&move_lock);
      } else if(ret == 0 ){
        return 1;
      }
      return 0;
  }
}

// Up the semaphore that cooresponds to a work station.
// param instruction: The instuction that needs to be carried out at the station.
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

// Check the counters to prevent starvation of threads.
// para type: The type of thread that wants to get a package.
// return: 0 - Another type of thread is starving, 1 - No threads are starving.
int check_counter(enum color type){
  sem_wait(&counter_lock);
  // Find max counter that is over MAX_WAIT
  int max = 0;
  enum color y = -1;
  if(rc > max && rc >= MAX_WAIT){
    max = rc;
    y = RED;
  }
  if(bc > max && bc >= MAX_WAIT){
    max = bc;
    y = BLUE;
  }
  if(gc > max && gc >= MAX_WAIT){
    max = gc;
    y = GREEN;
  }
  if(yc > max && yc >= MAX_WAIT){
    max = yc;
    y = YELLOW;
  }
  sem_post(&counter_lock);

  // All threads trying to get a new package yield to that thread
  if(y != -1 && type != y){
    return 0;
  }

  return 1;
}

// Add to a counter to guage starvation.
// para type: The type of thread just got a package.
void add_counter(enum color type){
  sem_wait(&counter_lock);
  switch (type) {
    case RED:
      rc = 0;
      bc += 1;
      gc += 1;
      yc += 1;
      break;
    case BLUE:
      rc += 1;
      bc = 0;
      gc += 1;
      yc += 1;
      break;
    case GREEN:
      rc += 1;
      bc += 1;
      gc = 0;
      yc += 1;
      break;
    case YELLOW:
      rc += 1;
      bc += 1;
      gc += 1;
      yc = 0;
      break;
  }

  sem_post(&counter_lock);
}

// Remove a connection from the adjacency matrix.
// param old: The source node in the graph.
// param new: The destination node in the graph.
void remove_from_matrix(int old, int new){
  // Remove a finished connection from the matrix
  sem_wait(&matrix_lock);

  station_matrix[old][new] -= 1;

  sem_post(&matrix_lock);
}


// Checks for a cycle in the graph recursively.
// param matrix: The adjacency matrix to look for cycles in.
// param visited: The nodes that were alread visisted.
// param count: Number of nodes visited.
// param start: Source node of the first connection to look at.
// param end: Destination node of the first connection.
// return: 0 if there is a cycle, 1 if there is no cycle
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
  for(int i=0; i<NUM_STATIONS; i++){

    if(end != i){
      if(matrix[end][i] > 0){
        if(check_cycle(matrix, visited, count, end, i) == 0){
          return 0;
        }
      }
    }

  }

  // No connections out of 'end' row, no cycle
  return 1;
}

// Check if a new set of instructions would end in deadlock.
// param node: Head node of linked list of instructions.
// return: 0 - There is a cycle in the graph, 1 - There is no cycles in the graph.
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


  struct instrNode* curNode = node;

  // Only one instruction no fear of deadlock
  if(curNode->next == NULL){
    sem_post(&matrix_lock);
    return 1;
  }

  // Add the new connections to the copy
  struct instrNode* nextNode = curNode->next;
  while(nextNode != NULL){
    copy[curNode->instruction][nextNode->instruction] += 1;

    curNode = nextNode;
    nextNode = curNode->next;
  }

  // Check for cyles
  for(int i=0; i<NUM_STATIONS; i++){
    for(int j=0; j<NUM_STATIONS; j++){

      int counter = 0;
      int visited[NUM_STATIONS] = {-1, -1, -1, -1};
      if(i != j){ // From station to itself

        int found = -1;
        if(copy[i][j] > 0){
          found = check_cycle(copy, visited, counter, i, j);
        }

        if(found == 0){
          // Found a cycle
          sem_post(&matrix_lock);
          return 0;
        }
      }

    }
  }

  // There are no cycles
  // Set the adjacency matrix as the new matrix
  for(int i=0; i<NUM_STATIONS; i++){
    for(int j=0; j<NUM_STATIONS; j++){
      station_matrix[i][j] = copy[i][j];
    }
  }

  sem_post(&matrix_lock);

  return 1;
}

// The worker thread.
// param args: Struct of workerArgs.
// return: NULL
void* worker(void* args){
  int id = ((struct workerArgs*)args)->id;
  enum color type = ((struct workerArgs*)args)->c;

  while(1){
    // Try to get the team lock
    get_team_lock(type);

    int c = 0;
    while(1){
      c = check_counter(type);
      sem_wait(&pkg_q_lock);
      if(c == 1 || head == NULL){
        sem_post(&pkg_q_lock);
        break;
      }
      sem_post(&pkg_q_lock);
      pthread_yield();
    }


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

    add_counter(type);

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

    // Put package on the first station
    int m1 = 0;
    while(1){
      sem_wait(&move_lock);
      m1 = get_station_lock(old_instruction);
      sem_post(&move_lock);
      if(m1 == 1){
        break;
      }
    }

    while(1){

      shout_start(id, type, my_package, old_instruction);

      // Work for [1,3] seconds
      int time = (rand() % 3) + 1;
      printf("[%s #%d]: Working for %d seconds.\n", get_type_string(type), id, time);
      sleep(time);

      shout_end(id, type, my_package, old_instruction);

      oldInstr = curInstr;
      curInstr = curInstr->next;
      free(oldInstr);

      // There are no more instructions to process
      if(curInstr == NULL){
        release_station_lock(old_instruction);
        break;
      }

      // Get the next instruction
      new_instruction = curInstr->instruction;

      // Move the package to the next station.
      int m2 = -1;
      while(1){
        sem_wait(&move_lock);
        m2 = get_station_lock(new_instruction);
        sem_post(&move_lock);
        if(m2 == 1){
          break;
        }
      }
      release_station_lock(old_instruction);
      remove_from_matrix(old_instruction, new_instruction);

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

// Fill a package with instructions.
// param pkg: Pointer to the package to fill.
void fillPkg(struct package* pkg){
  int num_instr = (rand() % 4) + 1;
  int list[] = {1,2,3,4};
  int numLeft = MAX_INSTR;
  struct instrNode* head = NULL;
  struct instrNode* tail = NULL;

  // Put in random number of [1,4] instructions
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
      // List is empty
      struct instrNode* nextNode = malloc(sizeof(struct instrNode));
      nextNode->instruction = instrEnum;
      nextNode->next = NULL;

      head = nextNode;
      tail = nextNode;

    } else{
      // List is not empty
      struct instrNode* nextNode = malloc(sizeof(struct instrNode));
      nextNode->instruction = instrEnum;

      tail->next = nextNode;
      tail = nextNode;
    }

  }

  // Put the instruction list in the package
  pkg->instrListHead = head;
}

// Create worker threads for each team.
// param ids: Array of ids for the threads.
// param type: The color of the team.
// param threads: Array to hold the pthread_t structs after creation.
void create_worker(int ids[TEAM_SIZE], enum color type, pthread_t threads[TEAM_SIZE]){

  for(int i=0; i<TEAM_SIZE; i++){
    pthread_t thread;
    ids[i] = i;

    // Fill struct for thread args.
    struct workerArgs* arg = malloc(sizeof(struct workerArgs));
    arg->id = ids[i];
    arg->c = type;

    pthread_create(&thread, NULL, worker, (void*)arg);
    threads[i] = thread;
  }
}

int main(void){
  // Seed the random number genrator.
  FILE* seed_file = fopen("seed.txt", "r");
  int seed = -1;
  fscanf(seed_file, "%d", &seed);

  srand(seed);

  fclose(seed_file);

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
  sem_init(&move_lock, 0, 1);

  // Init the adjacency matrix
  for(int i=0; i<NUM_STATIONS; i++){
    for(int j=0; j<NUM_STATIONS; j++){
      station_matrix[i][j] = 0;
    }
  }

  // Create the pile of packages
  struct pkgQueueNode* tail = NULL;
  for(int i=0; i<NUM_PACKAGES; i++){
    if(head == NULL && tail == NULL){
      // The queue is empty
      struct package* pkg = malloc(sizeof(struct package));
      // Fill packge with instructions
      fillPkg(pkg);
      pkg->id = i;

      // Create new queue node.
      struct pkgQueueNode* nextQNode = malloc(sizeof(struct pkgQueueNode));
      nextQNode->pkg = pkg;
      nextQNode->next = NULL;

      head = nextQNode;
      tail = nextQNode;
    } else{
      // The queue is not empty
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

  // Wait for all the workers to finish processing packages before terminating.
  for(int i=0; i<TEAM_SIZE; i++){
    pthread_join(red_threads[i], NULL);
    pthread_join(blue_threads[i], NULL);
    pthread_join(green_threads[i], NULL);
    pthread_join(yellow_threads[i], NULL);
  }

  printf("All the packages have been processed~.\n");

  return 0;
}
