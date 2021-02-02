#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int NUM_VISITS = 5;
int MAX_PATH_LEN =  1000;

char* PATHS[] = {
  "/home",
  "/proc",
  "/proc/sys",
  "/usr",
  "/usr/bin",
  "/bin"
};

int main(int argc, char* argv[]){
  // Get the seed value from the seed file
  FILE* f = fopen("seed.txt", "r");

  int seed;
  fscanf(f, "%d", &seed);

  // Seed the random number genrator
  srand(seed);
  printf("Read seed value: %d\n", seed);

  printf("It's time to see the world/file system!\n");

  // Visit 5 random locations
  for(int i=0; i<NUM_VISITS; i++){
    int num = rand() % 6;

    // Change the current working directory
    if(chdir(PATHS[num]) == 0){
      printf("Selection #%d: %s [SUCCESS]\n", i+1, PATHS[num]);

      // Print out the current working directory
      char cwd[MAX_PATH_LEN];
      getcwd(cwd, sizeof(cwd));
      printf("Current reported directory: %s\n", cwd);

    } else{ // chdir() failed
      perror("Change directory failed.\n");

      return -1;
    }

    // Create childs to execute 'ls -alh'
    int PID = fork();

    if(PID < 0){ // fork() failed
      printf("Failed to create child process.\n");

      return -1;

    } else if(PID == 0){ // Child executes this

      printf("\t[Child, PID: %d]: Executing 'ls -alh' command...\n", getpid());

      // Execute 'ls -alh'
      char* args[] = {"ls",
      "-alh",
      NULL};
      execvp(args[0], args);

    } else{ // Parent executes this
      printf("[Parent]: I'm waiting for PID %d, to finish.\n", PID);

      // Wait for child to finish
      int status;
      waitpid(PID, &status, 0);
      status = WEXITSTATUS(status);

      printf("[Parent]: Child %d finished with status code %d. Onward!\n", PID, status);

    }
  }

  return 0;
}
