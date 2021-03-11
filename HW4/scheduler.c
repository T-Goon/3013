#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct job{
  int id;
  int length;
  int time_left;

  struct job* next;
};

struct metric{
  int id;
  int rt;
  int tt;
  int wt;

  struct metric* next;
};


struct metric* mhead = NULL;
struct metric* mtail = NULL;

void add_metric(int id, int time, int len){
  if(mhead == NULL && mtail == NULL){
    // empty
    mhead = malloc(sizeof(struct metric));
    mhead->id = id;
    mhead->rt = time;
    mhead->tt = time + len;
    mhead->wt = time;
    mhead->next = NULL;

    mtail = mhead;
  } else{
    // add to tail
    struct metric* nm = malloc(sizeof(struct metric));
    nm->id = id;
    nm->rt = time;
    nm->tt = time + len;
    nm->wt = time;
    nm->next = NULL;

    mtail->next = nm;
    mtail = nm;
  }
}

int main(int argc, char **argv){

  if(argc != 4){
    printf("Usage: ./scheduler [FIFO | SJF | RR] <path_to_job_trace> <time_slice>\n");
    return 1;
  }

  struct job* head = NULL;
  struct job* tail = NULL;

  // Open test file
  FILE* f  = fopen(argv[2], "r");

  if (f == NULL){
    printf("Cannot find job trace file.\n");
    return 2;
  }

  // Load all jobs
  int num = 0;
  int IDcounter = 0;
  while(1){
    int len = 0;

    num = fscanf(f, "%d", &len);

    // No more jobs
    if(num == EOF){
      break;
    }

    // Add to linked list
    if(head == NULL && tail == NULL){
      // empty
      head = malloc(sizeof(struct job));
      head->id = IDcounter;
      head->length = len;
      head->time_left = len;
      head->next = NULL;

      tail = head;

    } else{
      // add to tail
      struct job* nj = malloc(sizeof(struct job));
      nj->id = IDcounter;
      nj->length = len;
      nj->time_left = len;
      nj->next = NULL;

      tail->next = nj;
      tail = nj;

    }

    IDcounter ++;
  }

  fclose(f);

  // struct job* cur = head;
  // while(cur != NULL){
  //   printf("%d %d\n", cur->id, cur->length);
  //
  //   cur = cur->next;
  // }

  if(strcmp(argv[1], "FIFO") == 0){
    printf("Execution trace with FIFO:\n");

    int start_time = 0;

    struct job* cur = head;

    // Do the jobs
    while(cur != NULL){
      printf("Job %d ran for: %d\n", cur->id, cur->length);

      // Add to metrics list
      add_metric(cur->id, start_time, cur->length);

      start_time += cur->length;

      // Get next job
      struct job* temp = cur;
      cur = cur->next;
      head = cur;
      free(temp);
    }

    printf("End of execution with FIFO.\n");
  } else if(strcmp(argv[1], "SJF") == 0){
    printf("Execution trace with SJF:\n");

    int start_time = 0;

    // Do all jobs
    while(head != NULL){

      // Find shortest job
      struct job* shortest = NULL;
      struct job* sprev = NULL;
      int time = INT_MAX;

      struct job* cur = head;
      struct job* prev = NULL;
      while(cur != NULL){

        if(cur->length < time){
          time = cur->length;
          shortest = cur;
          sprev = prev;
        }

        prev = cur;
        cur = cur->next;
      }

      printf("Job %d ran for: %d\n", shortest->id, shortest->length);

      // Add to metrics list
      add_metric(shortest->id, start_time, shortest->length);

      start_time += shortest->length;

      // Head not shortest
      if(sprev != NULL){
        sprev->next = shortest->next;
        free(shortest);
      } else{
        // Head shortest
        head = head->next;
        free(shortest);
      }
    }

    printf("End of execution with SJF.\n");
  } else if(strcmp(argv[1], "RR") == 0){
    printf("Execution trace with RR:\n");

    int time = atoi(argv[3]);

    if(time <= 0){
      printf("Invalid time slice for Round Robin\n");
      return 3;
    }

    // Do all jobs
    while(head != NULL){
      if(head->length > time){
        head->length -= time;

        printf("Job %d ran for: %d\n", head->id, time);

        // Move to end of list
        struct job* temp = head;
        tail->next = head;

        tail = tail->next;

        head = head->next;

        tail->next = NULL;

      } else{
        // Pop off head
        struct job* temp = head;

        printf("Job %d ran for: %d\n", head->id, head->length);

        head = head->next;
        free(temp);
      }
    }

    printf("End of execution with RR.\n");
  } else{
    printf("Usage: ./scheduler [FIFO | SJF | RR] <path_to_job_trace> <time_slice>\n");
    return 1;
  }

  float sum_rt = 0;
  float sum_tt = 0;
  float sum_wt = 0;
  int count = 0;

  // print out metrics
  printf("Begin analyzing %s:\n", argv[1]);
  while(mhead != NULL){
    printf("Job %d -- Response time: %d Turnaround: %d Wait: %d\n",
    mhead->id, mhead->rt, mhead->tt, mhead->wt);

    sum_rt += mhead->rt;
    sum_tt += mhead->tt;
    sum_wt += mhead->wt;
    count ++;

    struct metric* temp = mhead;
    mhead = mhead->next;
    free(temp);
  }

  printf("Average -- Response time: %.02f Turnaround: %.02f Wait: %.02f\n",
  sum_rt/count, sum_tt/count, sum_wt/count);

  printf("End of analyzing %s.\n", argv[1]);

  return 0;
}
