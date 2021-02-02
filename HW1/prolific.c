#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){

  // Get the seed value from the seed file
  FILE* f = fopen("seed.txt", "r");

  int seed;
  fscanf(f, "%d", &seed);

  // Seed the random number genrator
  srand(seed);
  printf("Read seed value: %d\n", seed);

  // Get random number of children [10,15]
  int num = (rand() % 6) + 10;
  printf("Random Child Count: %d\n", num);

  // Generate a random number for each child
  int childNums[num];
  for(int i=0; i<num; i++){
    childNums[i] = rand();
  }

  printf("I'm feeling prolific!\n");

  // Create and handle all the child processes
  for(int i=0; i<num; i++){

    int PID = fork();

    if(PID < 0){ // Failed to create child
      printf("fork() failed\n");
      return 1;

    } else if(PID == 0){ // Child executes this
      int exitCode = ((childNums[i] % 50) + 1);
      int waitTime = ((childNums[i] % 3) + 1);

      printf("[Child, PID: %d]: I am the child and I will wait %d seconds and exit with code %d.\n", getpid(), waitTime, exitCode);

      sleep(waitTime);

      printf("[Child, PID: %d]: Now exiting...\n", getpid());

      return exitCode;

    } else{ // Parent executes this
      printf("[Parent]: I am waiting for PID %d to finish.\n", PID);

      // Wait for child to finish and get exit code
      int status;
      waitpid(PID, &status, 0);

      status = WEXITSTATUS(status);
      printf("[Parent]: Child %d finished with status code %d. Onward!\n", PID, status);
    }
  }

  return 0;
}
