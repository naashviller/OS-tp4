#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOOPS 20
#define BUF_SIZE 4

pthread_mutex_t mutex_m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t non_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t non_full = PTHREAD_COND_INITIALIZER;

int count = 0;
int in = 0;
int out = 0;
int buffer[BUF_SIZE];

void pushing(int val) {

  pthread_mutex_lock(&mutex_m);

  while (count == BUF_SIZE) {
    pthread_cond_wait(&non_full, &mutex_m);
  }

  buffer[in] = val;
  printf("pushing value %d\n", val);
  in = (in + 1) % BUF_SIZE;
  count++;

  pthread_cond_signal(&non_empty);
  pthread_mutex_unlock(&mutex_m);
  /* NOTE: pay attention to where to put the call to printf to be
      sure it can be helpful for debugging */
}

int fetching(void) {
  int val = 0;
  pthread_mutex_lock(&mutex_m);

  while (count == 0) {
    pthread_cond_wait(&non_empty, &mutex_m);
  }

  val = buffer[out];
  out = (out + 1) % BUF_SIZE;
  count--;

  pthread_cond_signal(&non_full);
  pthread_mutex_unlock(&mutex_m);

  /* NOTE: pay attention to where to put the call to printf to be
     sure it can be helpful for debugging */
  printf("\t feched value %d\n", val);

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
