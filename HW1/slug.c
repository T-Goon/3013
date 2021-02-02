#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

char* SEED_FILE_NAMES[] = {"seed_slug_1.txt",
"seed_slug_2.txt",
"seed_slug_3.txt",
"seed_slug_4.txt"};

char* COMMAND1[] = {
  "last",
  "-d",
  "--fulltimes",
  NULL
};

char* COMMAND2[] = {
  "id",
  "-u",
  NULL
};

int main(int argc, char* argv[]){

  // Check to make sure the command line args are valid
  if(argc != 2){
    printf("Usage: ./slug (1|2|3|4)\n");

    return 1;
  }

  if(!isdigit(argv[1][0])){
    printf("Usage: ./slug (1|2|3|4)\n");

    return 1;
  }

  int arg = atoi(argv[1]);

  if(arg < 1 || arg > 4){
    printf("Usage: ./slug (1|2|3|4)\n");

    return 1;
  }

  // Get the seed from the seed file.
  FILE* f;
  f = fopen(SEED_FILE_NAMES[arg-1], "r");
  int seed;
  fscanf(f, "%d", &seed);

  printf("[Slug PID: %d]: Read seed value: %d\n", getpid(), seed);

  srand(seed);

  // Get the random wait time [1,5] and coin flip [0,1]
  int seconds = (rand() % 5) + 1;
  int coin = rand() % 2;

  printf("[Slug PID: %d]: Delay time is %d seconds. Coin flip: %d\n", getpid(), seconds, coin);
  printf("[Slug PID: %d]: I'll get the job done. Eventually...\n", getpid());

  // Wait seconds amount of seconds
  sleep(seconds);

  if(coin == 1){ // Execute 'id -u'
    printf("[Slug PID: %d]: Break time is over! I am running the '%s %s' command.\n",
    getpid(), COMMAND2[0], COMMAND2[1]);

    execvp(COMMAND2[0], COMMAND2);
  } else if(coin == 0){ // Execute 'last -d --fulltimes'
    printf("[Slug PID: %d]: Break time is over! I am running the '%s %s %s' command.\n",
     getpid(), COMMAND1[0], COMMAND1[1], COMMAND1[2]);

   execvp(COMMAND1[0], COMMAND1);
  }

  return 0;
}
