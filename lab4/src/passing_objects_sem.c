#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOOPS 20
#define BUF_SIZE 4

sem_t empty;
sem_t full;
sem_t mutex;

int count = 0;
int in = 0;
int out = 0;
int buffer[BUF_SIZE];


void pushing(int val) {

  sem_wait(&empty);
  sem_wait(&mutex);

    buffer[in] = val;
    printf("pushing value %d\n", val);
    in = (in + 1) % BUF_SIZE;
    count++;

  sem_post(&mutex);
  sem_post(&full);


}

int fetching(void) {
  int val = 0;
  
    sem_wait(&full);
    sem_wait(&mutex);
  val = buffer[out];
  out = (out + 1) % BUF_SIZE;
  count--;
printf("\t fetched value %d\n", val);
  sem_post(&mutex);
  sem_post(&empty);

  return val;
}

void *thread_generating(void *arg) {
  int i = 0;
  int new_value = 0;

  for (i = 0; i < LOOPS; i++) {
    new_value = rand() % 10;
    pushing(new_value);
  }

  return NULL;
}

void *thread_using(void *arg) {
  int i = 0;
  int value = 0;

  for (i = 0; i < LOOPS; i++) {
    value = fetching();
  }

  return NULL;
}

int main(void) {
  pthread_t tids[2];
  int i = 0;


    sem_init(&empty, 0, BUF_SIZE);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);
  struct timespec tt;
  clock_gettime(CLOCK_MONOTONIC, &tt);
  /* seed for the random number generator */
  srand(tt.tv_sec);

  if (pthread_create(&tids[0], NULL, thread_using, NULL) != 0) {
    fprintf(stderr, "Failed to create the using thread\n");
    return EXIT_FAILURE;
  }

  if (pthread_create(&tids[1], NULL, thread_generating, NULL) != 0) {
    fprintf(stderr, "Failed to create the generating thread\n");
    return EXIT_FAILURE;
  }

  /* Wait until every thread ended */
  for (i = 0; i < 2; i++) {
    pthread_join(tids[i], NULL);
  }

  return EXIT_SUCCESS;
}
