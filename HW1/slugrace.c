#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <time.h>

int  NUM_CHILDREN = 4;
int QUARTER_SECOND = 250000;

int main(int argc, char* argv[]){

  int active_racers[NUM_CHILDREN];

  // Create timestamp for start of race
  struct timespec* start = malloc(sizeof(struct timespec));
  clock_gettime(CLOCK_MONOTONIC, start);

  float startSeconds = start->tv_sec + (start->tv_nsec/1000000000);

  free(start);

  // fork off 4 children
  for(int i=1; i<=NUM_CHILDREN; i++){
    int PID = fork();

    if(PID < 0){ // fork() failed
      perror("Failed to create children.\n");

      return -1;
    } else if(PID == 0){ // The child executes this

      char a[2];
      sprintf(a, "%d", i);
      char* command[] = {
        "slug",
        a,
        NULL
      };

      printf("\t[Child, PID: %d]: Executing '%s %s' command...\n",
      getpid(), command[0], command[1]);

      // Child executs 'slug #'
      execv(command[0], command);

    } else{ // Parent executes this
      printf("[Parent]: I forked off child %d.\n", PID);
      active_racers[i-1] = PID;
    }
  }

  // Loop forever while waiting for children to terminate
  while(1){
    // Check if a child terminated or not
    int status;
    int wc = waitpid(-1, &status, WNOHANG);

    if(wc > 0){ // A child has terminated
      // Get race ending timestamp
      struct timespec* end = malloc(sizeof(struct timespec));
      clock_gettime(CLOCK_MONOTONIC, end);

      float endSeconds = end->tv_sec + (end->tv_nsec/1000000000);

      printf("Child %d has crossed the finish line! It took %f seconds.\n",
      wc, endSeconds - startSeconds);

      free(end);

      // Remove child from list of active racers
      for(int i=0; i<NUM_CHILDREN; i++){
        if(active_racers[i] == wc){
          active_racers[i] = 0;
        }
      }

      // Check if there are any currently racing children
      int flag = 0;
      for(int i=0; i<NUM_CHILDREN; i++){
        if(active_racers[i] != 0){
          flag = 1;
        }
      }

      if(flag == 0){ // There are no more racing children
        break;
      }
    }

    usleep(QUARTER_SECOND);

    // Print out the active children
    printf("The race is ongoing. The following children are still racing: ");
    for(int j=0; j<NUM_CHILDREN; j++){
      if(active_racers[j] != 0)
        printf("%d ", active_racers[j]);
    }
    printf("\n");

  }

  return 0;
}
