#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#define NUM_DANCERS 15
#define MAX_DANCERS 4

#define NUM_JUGGLERS 8
#define MAX_JUGGLERS 4

#define NUM_SOLOIST 2

int num_dancing = 0;
int num_juggling = 0;

pthread_mutex_t stage_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pos_1_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pos_2_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pos_3_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pos_4_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t dancer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t dancer_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t juggler_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t juggler_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t soloist_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t soloist_cond = PTHREAD_COND_INITIALIZER;

void* dancer(void* arg){
  int position = 0;
  int ret = 0;

  // printf("Entering Dancer\n");
  // ret = pthread_mutex_lock(&dancer_lock);
  // assert(ret == 0);
  // ret = pthread_cond_wait(&dancer_cond, &dancer_lock);
  // assert(ret == 0);
  // ret = pthread_mutex_unlock(&dancer_lock);
  // assert(ret == 0);

  while(1){

    while(1){
      // printf("Entering Dancer lock loop\n");
      ret = pthread_mutex_lock(&dancer_lock);
      assert(ret == 0);
      if(num_dancing == 0){
        // No dancers dancing, try to get onstage
        ret = pthread_mutex_trylock(&stage_lock);
        if(ret == EBUSY){
          // A different type of performance is going on, go to sleep
          pthread_cond_wait(&dancer_cond, &dancer_lock);
          pthread_mutex_unlock(&dancer_lock);
          // Go back to start of loop and try to get on stage again
          continue;
        }

        // Wake another dancer then go perform
        num_dancing ++;
        position = num_dancing;
        pthread_mutex_unlock(&dancer_lock);

        pthread_cond_signal(&dancer_cond);
        break;

      } else if(num_dancing < MAX_DANCERS){
        // Another dancer is already onstage, go join
        num_dancing ++;
        position = num_dancing;
        pthread_mutex_unlock(&dancer_lock);
        pthread_cond_signal(&dancer_cond);
        break;

      } else{
        // The max number of dancers are onstage, sleep
        pthread_cond_wait(&dancer_cond, &dancer_lock);
        pthread_mutex_unlock(&dancer_lock);
      }
    }

    int time = (rand() % 5) + 1;
    int id = (pthread_self()%NUM_DANCERS)+1;
    printf("Flamenco Dancer ID %d will perform in stage position %d for %d seconds.\n",
      id, position, time);


    // Perform for [1, 5] seconds
    sleep(time);

    printf("Flamenco Dancer ID %d finished performing and will leave stage position %d.\n",
      id, position);

    pthread_mutex_lock(&dancer_lock);
    num_dancing --;
    // Sleep if you are not the last dancer on stage
    if(num_dancing > 0){
      pthread_cond_wait(&dancer_cond, &dancer_lock);
      pthread_mutex_unlock(&dancer_lock);
    } else{
      pthread_mutex_unlock(&dancer_lock);
      pthread_mutex_unlock(&stage_lock);
      // The last dancer picks the next performer
      // Randomly pick a juggler or soloist to go next
      // Equal chance to get picked
      int pick = rand() % 2;
      if(pick == 1)
        pthread_cond_signal(&soloist_cond);
      else
        pthread_cond_signal(&juggler_cond);

      // Go to sleep
      pthread_mutex_lock(&dancer_lock);
      pthread_cond_wait(&dancer_cond, &dancer_lock);
      pthread_mutex_unlock(&dancer_lock);
    }
  }

  return NULL;
}

void* juggler(void* arg){
  int position = 0;
  int ret = 0;

  // ret = pthread_mutex_lock(&juggler_lock);
  // assert(ret == 0);
  // ret = pthread_cond_wait(&juggler_cond, &juggler_lock);
  // assert(ret == 0);
  // ret = pthread_mutex_unlock(&juggler_lock);
  // assert(ret == 0);

  // printf("Entering juggler\n");

  while(1){
    while(1){
      // printf("Entering juggler lock loop\n");
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

        // Wake another juggler then go perform
        num_juggling ++;
        position = num_juggling;
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
        // The max number of jugglers are onstage, sleep
        ret = pthread_cond_wait(&juggler_cond, &juggler_lock);
        assert(ret == 0);
        ret = pthread_mutex_unlock(&juggler_lock);
        assert(ret == 0);
      }
    }

    int time = (rand() % 5) + 1;
    int id = (pthread_self()%NUM_JUGGLERS)+1;
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
      int pick = rand() % 3;
      if(pick > 0){
        ret = pthread_cond_signal(&dancer_cond);
        assert(ret == 0);
      }
      else{
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

  // ret = pthread_mutex_lock(&soloist_lock);
  // assert(ret == 0);
  // ret = pthread_cond_wait(&soloist_cond, &soloist_lock);
  // assert(ret == 0);
  // ret = pthread_mutex_unlock(&soloist_lock);
  // assert(ret == 0);

  while(1){
    while(1){
      // printf("Entering soloist lock loop\n");
      ret = pthread_mutex_lock(&soloist_lock);
      assert(ret == 0);
      // try to get onstage
      ret = pthread_mutex_trylock(&stage_lock);
      if(ret == EBUSY){
        // A different type of performance is going on, go to sleep
        ret = pthread_cond_wait(&soloist_cond, &soloist_lock);
        assert(ret == 0);
      } else{
        ret = pthread_mutex_unlock(&soloist_lock);
        assert(ret == 0);
        break;
      }

      ret = pthread_mutex_unlock(&soloist_lock);
      assert(ret == 0);
    }

    int time = (rand() % 5) + 1;
    int id = (pthread_self()%NUM_SOLOIST)+1;
    printf("Soloist ID %d will perform in all stage positions for %d seconds.\n",
      id, time);


    // Perform for [1, 5] seconds
    sleep(time);

    printf("Soloist ID %d finished performing and will leave the stage.\n",
      id);

    ret = pthread_mutex_unlock(&stage_lock);
    assert(ret == 0);
    // Picks the next performer
    // Randomly pick a dancer or juggler to go next
    // 2 to 1 in favor of dancers
    int pick = rand() % 3;
    if(pick > 0){
      // printf("Soloist picking dancer.\n");
      ret = pthread_cond_signal(&dancer_cond);
      assert(ret == 0);
    }
    else{
      ret = pthread_cond_signal(&juggler_cond);
      assert(ret == 0);
      // printf("Soloist picking juggler.\n");
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

int main(void){
  FILE* seedFile = fopen("seed.txt", "r");
  int seed;
  fscanf(seedFile, "%d", &seed);
  fclose(seedFile);

  srand(seed);

  pthread_t* dancers[NUM_DANCERS];
  pthread_t* jugglers[NUM_JUGGLERS];
  pthread_t* solos[NUM_SOLOIST];

  // Create the flamenco dancer threads
  for(int i=0; i<NUM_DANCERS; i++){
    // printf("Creating a dancer.\n");
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, dancer, NULL);
    assert(ret == 0);
    dancers[i] = &thread;
  }

  // Create the juggler threads
  for(int i=0; i<NUM_JUGGLERS; i++){
    // printf("Creating a juggler.\n");
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, juggler, NULL);
    assert(ret == 0);
    jugglers[i] = &thread;
  }


  // Create the soloist threads
  for(int i=0; i<NUM_SOLOIST; i++){
    // printf("Creating a soloist.\n");
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, soloist, NULL);
    assert(ret == 0);
    solos[i] = &thread;
  }

  // Spin
  while(1){
    int ret = pthread_yield();
    assert(ret == 0);
  }

  return 0;
}
