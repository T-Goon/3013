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

  // Get random lifespan[7,10]
  int lifespan = (rand() % 4) + 7;
  printf("Random Descendant Count: %d\n", lifespan);

  printf("Time to meet the kids/grandkids/great grandkids...\n");

  // Create and handle all the child processes
  while(lifespan > 0){
    int PID = fork();

    if(PID < 0){ // fork failed
      printf("fork() failed.\n");
      return -1;

    } else if(PID == 0){ // This runs after becoming a child
      printf("[Child, PID %d]: I was called with descendant count=%d. I'll have %d descendant(s).\n",
      getpid(), lifespan, lifespan-1);

      // Decrement lifespan
      lifespan--;

    } else{ // This runs after becoming a parent
      printf("[Parent, PID: %d] I amd waiting for PID %d to finish.\n", getpid(), PID);

      // Wait for child to finish
      int status;
      waitpid(PID, &status, 0);
      status = WEXITSTATUS(status);

      printf("[Parent, PID: %d]: Child %d finished with status code %d. It's now my turn to exit.\n",
      getpid(), PID, status);

      // return number of descendants
      return lifespan;
    }
  }

  return 0;
}
