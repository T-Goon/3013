#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#define MAX_WAIT 3
#define NUM_DANCERS 15
#define MAX_DANCERS 4

#define NUM_JUGGLERS 8
#define MAX_JUGGLERS 4

#define NUM_SOLOIST 2

volatile sig_atomic_t done = 0;

pthread_mutex_t stage_lock = PTHREAD_MUTEX_INITIALIZER;
int num_since_dancer = 0;
int num_since_juggler = 0;
int num_since_soloist = 0;

pthread_mutex_t dancer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t dancer_cond = PTHREAD_COND_INITIALIZER;
int num_dancing = 0;

pthread_mutex_t juggler_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t juggler_cond = PTHREAD_COND_INITIALIZER;
int num_juggling = 0;

pthread_mutex_t soloist_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t soloist_cond = PTHREAD_COND_INITIALIZER;

void* dancer(void* arg){
  int position = 0;
  int ret = 0;
  int id = *((int*) arg);

  while(1){

    // Loop to acquire lock and enter the stage
    while(1){

      ret = pthread_mutex_lock(&dancer_lock);
      assert(ret == 0);

      if(num_dancing == 0){
        // No dancers dancing, try to get onstage
        ret = pthread_mutex_trylock(&stage_lock);
        if(ret == EBUSY){
          // A different type of performance is going on, go to sleep
          ret = pthread_cond_wait(&dancer_cond, &dancer_lock);
          assert(ret == 0);
          ret = pthread_mutex_unlock(&dancer_lock);
          assert(ret == 0);
          // Go back to start of loop and try to get on stage again
          continue;
        }
        assert(ret == 0);

        // Increment counters
        num_since_dancer = 0;
        num_since_juggler ++;
        num_since_soloist ++;
        num_dancing ++;
        position = num_dancing;

        // Wake another dancer then go perform
        ret = pthread_mutex_unlock(&dancer_lock);
        assert(ret == 0);

        ret = pthread_cond_signal(&dancer_cond);
        assert(ret == 0);
        break;

      } else if(num_dancing < MAX_DANCERS){
        // Another dancer is already onstage, go join
        num_dancing ++;
        position = num_dancing;

        ret = pthread_mutex_unlock(&dancer_lock);
        assert(ret == 0);

        ret = pthread_cond_signal(&dancer_cond);
        assert(ret == 0);
        break;

      } else{
        // The max number of dancers are onstage, back to sleep
        ret = pthread_cond_wait(&dancer_cond, &dancer_lock);
        assert(ret == 0);

        ret = pthread_mutex_unlock(&dancer_lock);
        assert(ret == 0);
      }
    }

    int time = (rand() % 5) + 1;
    printf("Flamenco Dancer ID %d will perform in stage position %d for %d seconds.\n",
      id, position, time);


    // Perform for [1, 5] seconds
    sleep(time);

    printf("Flamenco Dancer ID %d finished performing and will leave stage position %d.\n",
      id, position);

    // Done performing
    ret = pthread_mutex_lock(&dancer_lock);
    assert(ret == 0);
    num_dancing --;
    // Sleep if you are not the last dancer on stage
    if(num_dancing > 0){
      ret = pthread_cond_wait(&dancer_cond, &dancer_lock);
      assert(ret == 0);

      ret = pthread_mutex_unlock(&dancer_lock);
      assert(ret == 0);

    } else{
      ret = pthread_mutex_unlock(&dancer_lock);
      assert(ret == 0);
      ret = pthread_mutex_unlock(&stage_lock);
      assert(ret == 0);

      // The last dancer picks the next performer
      // Randomly pick a juggler or soloist to go next
      // Equal chance to get picked
      // Also check the starvation counters
      int pick = rand() % 2;
      if(pick == 1 || num_since_soloist >= MAX_WAIT){
        ret = pthread_cond_signal(&soloist_cond);
        assert(ret == 0);
      }
      else if(pick == 0 || num_since_juggler >= MAX_WAIT)
        ret = pthread_cond_signal(&juggler_cond);
        assert(ret == 0);


      // Go to sleep
      ret = pthread_mutex_lock(&dancer_lock);
      assert(ret == 0);

      ret = pthread_cond_wait(&dancer_cond, &dancer_lock);
      assert(ret == 0);

      ret = pthread_mutex_unlock(&dancer_lock);
      assert(ret == 0);
    }
  }

  return NULL;
}

void* juggler(void* arg){
  int position = 0;
  int id = *((int*) arg);
  int ret = 0;

  while(1){
    // Loop to acquire the lock
    while(1){
      ret = pthread_mutex_lock(&juggler_lock);
      assert(ret == 0);

      if(num_juggling == 0){
        // No jugglers juggling, try to get onstage
        ret = pthread_mutex_trylock(&stage_lock);
        if(ret == EBUSY){
          // A different type of performance is going on, go to sleep
          ret = pthread_cond_wait(&juggler_cond, &juggler_lock);
          assert(ret == 0);

          ret = pthread_mutex_unlock(&juggler_lock);
          assert(ret == 0);
          // Go back to start of loop and try to get on stage again
          continue;
        }

        // Increment the counters
        num_since_juggler = 0;
        num_since_dancer ++;
        num_since_soloist ++;
        num_juggling ++;
        position = num_juggling;

        // Wake another juggler then go perform
        ret = pthread_mutex_unlock(&juggler_lock);
        assert(ret == 0);

        ret = pthread_cond_signal(&juggler_cond);
        assert(ret == 0);
        break;

      } else if(num_juggling < MAX_JUGGLERS){
        // Another juggler is already onstage, go join
        num_juggling ++;
        position = num_juggling;

        ret = pthread_mutex_unlock(&juggler_lock);
        assert(ret == 0);
        ret = pthread_cond_signal(&juggler_cond);
        assert(ret == 0);
        break;

      } else{
        // The max number of jugglers are onstage, back to sleep
        ret = pthread_cond_wait(&juggler_cond, &juggler_lock);
        assert(ret == 0);
        ret = pthread_mutex_unlock(&juggler_lock);
        assert(ret == 0);
      }
    }

    int time = (rand() % 5) + 1;
    printf("Juggler ID %d will perform in stage position %d for %d seconds.\n",
      id, position, time);

    // Perform for [1, 5] seconds
    sleep(time);

    printf("Juggler ID %d finished performing and will leave stage position %d.\n",
      id, position);

    ret = pthread_mutex_lock(&juggler_lock);
    assert(ret == 0);
    num_juggling --;

    // Sleep if you are not the last juggler on stage
    if(num_juggling > 0){
      ret = pthread_cond_wait(&juggler_cond, &juggler_lock);
      assert(ret == 0);

      ret = pthread_mutex_unlock(&juggler_lock);
      assert(ret == 0);

    } else{
      ret = pthread_mutex_unlock(&juggler_lock);
      assert(ret == 0);

      ret = pthread_mutex_unlock(&stage_lock);
      assert(ret == 0);

      // The last juggler picks the next performer
      // Randomly pick a dancer or soloist to go next
      // 2 to 1 in favor of dancers
      // Also check the starvation counters
      int pick = rand() % 3;
      if(pick > 0 || num_since_dancer >= MAX_WAIT){
        ret = pthread_cond_signal(&dancer_cond);
        assert(ret == 0);
      }
      else if (pick == 0 || num_since_soloist >= MAX_WAIT){
        ret = pthread_cond_signal(&soloist_cond);
        assert(ret == 0);
      }

      // Go to sleep
      ret = pthread_mutex_lock(&juggler_lock);
      assert(ret == 0);
      ret = pthread_cond_wait(&juggler_cond, &juggler_lock);
      assert(ret == 0);
      ret = pthread_mutex_unlock(&juggler_lock);
      assert(ret == 0);
    }
  }

  return NULL;
}

void* soloist(void* arg){
  // printf("Entering soloist\n");
  int ret = 0;
  int id = *((int*) arg);

  while(1){
    // Loop to get lock
    while(1){

      ret = pthread_mutex_lock(&soloist_lock);
      assert(ret == 0);
      // try to get onstage
      ret = pthread_mutex_trylock(&stage_lock);
      if(ret == EBUSY){
        // A different type of performance is going on, go to sleep
        ret = pthread_cond_wait(&soloist_cond, &soloist_lock);
        assert(ret == 0);

      } else{
        assert(ret == 0);
        // Incremnt counters
        num_since_soloist = 0;
        num_since_dancer ++;
        num_since_juggler ++;

        // Go onstage
        ret = pthread_mutex_unlock(&soloist_lock);
        assert(ret == 0);
        break;
      }

      ret = pthread_mutex_unlock(&soloist_lock);
      assert(ret == 0);
    }

    int time = (rand() % 5) + 1;
    printf("Soloist ID %d will perform in all stage positions for %d seconds.\n",
      id, time);


    // Perform for [1, 5] seconds
    sleep(time);

    printf("Soloist ID %d finished performing and will leave the stage.\n",
      id);

    // Done performing
    ret = pthread_mutex_unlock(&stage_lock);
    assert(ret == 0);

    // Picks the next performer
    // Randomly pick a dancer or juggler to go next
    // 2 to 1 in favor of dancers
    // Also checks the starvation counters
    int pick = rand() % 3;
    if(pick > 0 || num_since_dancer >= MAX_WAIT){
      ret = pthread_cond_signal(&dancer_cond);
      assert(ret == 0);

    }
    else if(pick == 0 || num_since_juggler >= MAX_WAIT){
      ret = pthread_cond_signal(&juggler_cond);
      assert(ret == 0);
    }

    // Go to sleep
    ret = pthread_mutex_lock(&soloist_lock);
    assert(ret == 0);
    ret = pthread_cond_wait(&soloist_cond, &soloist_lock);
    assert(ret == 0);
    ret = pthread_mutex_unlock(&soloist_lock);
    assert(ret == 0);

  }

  return NULL;
}

// Set done to 1.
void terminate(int sig){
  done = 1;
}

int main(void){

  // Set up signal handler
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = terminate;
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGINT, &action, NULL);

  FILE* seedFile = fopen("seed.txt", "r");
  int seed;
  fscanf(seedFile, "%d", &seed);
  fclose(seedFile);

  srand(seed);

  int ret = -1;

  // Create the flamenco dancer threads
  int dancer_ids[NUM_DANCERS];
  for(int i=0; i<NUM_DANCERS; i++){
    // printf("Creating a dancer.\n");
    pthread_t thread;
    dancer_ids[i] = i;
    ret = pthread_create(&thread, NULL, dancer, (void*)&dancer_ids[i]);
    assert(ret == 0);
  }

  // Create the juggler threads
  int juggler_ids[NUM_JUGGLERS];
  for(int i=0; i<NUM_JUGGLERS; i++){
    // printf("Creating a juggler.\n");
    pthread_t thread;
    juggler_ids[i] = i;
    ret = pthread_create(&thread, NULL, juggler, (void*)&juggler_ids[i]);
    assert(ret == 0);
  }

  // Create the soloist threads
  int soloist_ids[NUM_SOLOIST];
  for(int i=0; i<NUM_SOLOIST; i++){
    // printf("Creating a soloist.\n");
    pthread_t thread;
    soloist_ids[i] = i;
    ret = pthread_create(&thread, NULL, soloist, (void*)&soloist_ids[i]);
    assert(ret == 0);
  }

  // Sleep forever
  while(!done){
    pause();
  }

  printf("\nThe show is over!!!\n");

  return 0;
}
