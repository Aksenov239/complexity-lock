#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <time.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <map>
#include <unistd.h>
#include <math.h>
#include <sys/syscall.h>
#include <atomic>

#include "common/cache_aligned_alloc.h"
#include "common/my_time.h"
#include "common/cmdline.hpp"

#define MAX_THREADS 64
#define PER_CORE 8

#define ASM __asm__ __volatile__
#define membarstoreload() { ASM ("mfence;") ; }

#define __CAS64(m,c,s)                                          \
({ int64_t _x = (c);                                          \
__asm__ __volatile (                                       \
"lock; cmpxchgq %1,%2;"                                  \
: "+a" (_x)                                              \
: "r" ((int64_t)(s)), "m" (*(volatile int64_t *)(m))     \
: "cc", "memory") ;                                      \
_x; })


#define CPU_RELAX ACM("pause\n": : :"memory");

#define CAS(m, c, s) __CAS64((m),(c),(s))

#define NOP __asm__ __volatile__("")

struct cmd_line_args_t {
  unsigned time;
  unsigned threads_count;
  std::string pin_type;
  unsigned parallel_work;
  unsigned critical_work;
};

struct thread_data_t {
  unsigned tid;
  
  unsigned long tries;
  unsigned long iterations;

  unsigned total;

  std::atomic<bool> finish;
};

static void print_usage();
static void parse_cmd_line_args(cmd_line_args_t &args, int argc, char** argv);
static void* thread_fun(void* data);
static unsigned thread_main(thread_data_t* data );

void pin(pid_t t, int cpu);

unsigned (*pin_func) (unsigned );

unsigned greedy_pin(unsigned tid);
unsigned socket_pin(unsigned tid);
unsigned uneven_pin(unsigned tid);

static cmd_line_args_t args;

std::atomic<int> lock;
unsigned TIME;
bool ZERO = false;

int main(int argc, char **argv) {
  args.pin_type = 1;

  lock = 0;

  parse_cmd_line_args(args, argc, argv);

  thread_data_t** thread_data = (thread_data_t**) cache_size_aligned_alloc(
                                                                           sizeof(thread_data_t*) * args.threads_count);
  for (unsigned i = 0; i < args.threads_count; i++) {
    thread_data_t* td = (thread_data_t*) cache_size_aligned_alloc(sizeof(thread_data_t));
    
    td->tid = i;

    td->tries = 0;
    td->iterations = 0;

    thread_data[i] = td;
  }

  pthread_t* threads = (pthread_t*) cache_size_aligned_alloc(sizeof(pthread_t) * args.threads_count);

  for (unsigned i = 0; i < args.threads_count; i++) {
    if (pthread_create(threads + i, NULL, thread_fun, (void*) thread_data[i])) {
      std::cerr << "Cannot create required threads. Exiting." << std::endl;
      exit(1);
    }
  }

  sleep_ms(TIME);

  for (unsigned i = 0; i < args.threads_count; i++) {
    thread_data[i]->finish.store(true);
  }

  for (unsigned i = 0; i < args.threads_count; i++) {
    pthread_join(threads[i], NULL);
  }

  unsigned long total_iterations = 0;
  unsigned long total_tries = 0;

  for (int i = 0; i < args.threads_count; i++) {
    total_iterations += thread_data[i]->iterations;
    total_tries += thread_data[i]->tries;
  }

  printf("Total throughput: %f op/sec\n", 1. * total_iterations / TIME);
  printf("Average tries: %f\n", 1. * total_tries / total_iterations);

  for (int i = 0; i < args.threads_count; i++) {
    cache_aligned_free(thread_data[i]);
  }

  cache_aligned_free(thread_data);
  cache_aligned_free(threads);

  return 0;  
}


void pin(pid_t t, int cpu) {
  pthread_t thread;
  cpu_set_t cpuset;
   
  thread = pthread_self();
    
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);
    
  if(pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset) != 0) {
	  printf("error setting affinity for %d %d\n", (int) t, (int) cpu);
  }
    
  int j, s;
  s = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
  if (s != 0) {
	printf("error\n" );
  } else {
    printf("Set for %d:\n", t);
    for (j = 0; j < CPU_SETSIZE; j++)
      if (CPU_ISSET(j, &cpuset))
        printf("    CPU %d\n", j);
  }
}

static void parse_cmd_line_args(cmd_line_args_t &args, int argc, char **argv) {
  deepsea::cmdline::set(argc, argv);
  // set default

  args.threads_count = deepsea::cmdline::parse_or_default_int("proc", 1);
  args.time = deepsea::cmdline::parse_or_default_int("time", 1000);
  args.pin_type = deepsea::cmdline::parse_or_default_string("pin", "greedy");
  args.critical_work = deepsea::cmdline::parse_or_default_int("critical", 1000);
  args.parallel_work = deepsea::cmdline::parse_or_default_int("parallel", 1000);
  
  // some argument checking, but not very extensive
  
  if(args.threads_count == 0) {
      std::cerr << "threads_count must be higher than 0" << std::endl;
      exit(0);
  }
  if(args.time <= 1) {
      std::cerr << "time must be higher than 1 millisecond" << std::endl;
      exit(0);
  }
  
  TIME = args.time;
  
  // print values
  std::cout << "Using parameters:\n" <<
  "\tthread count         = " << args.threads_count << "\n" <<
  "\ttime count         = " << args.time << "\n";
}

unsigned greedy_pin(unsigned tid) {
  unsigned target = tid;
  printf("thread %d to %d\n", tid, target);
  return target;
}

unsigned socket_pin(unsigned tid) {
  unsigned cores = MAX_THREADS / PER_CORE;
  unsigned target = (tid % cores) * PER_CORE + tid / cores;
  printf("thread %d to %d\n", tid, target);
  return target;
}

static void* thread_fun(void* data) {
  if (args.pin_type == "greedy") {
      pin(((thread_data_t *)data)->tid, greedy_pin(((thread_data_t *) data)->tid));
  } else if (args.pin_type == "socket") {
      pin(((thread_data_t *)data)->tid, socket_pin(((thread_data_t *) data)->tid));
  } else {
      printf( "wrong pin parameter!" );
      exit(0);
  }
  
  thread_data_t* thread_data = (thread_data_t*) data;

  while (!thread_data->finish.load(std::memory_order_relaxed)) {
    thread_data->iterations++;
    thread_main(thread_data);
  }
  
  return NULL;
}

static unsigned thread_main(thread_data_t* data) {
  for (int i = 0; i < args.parallel_work; i++) {
    NOP;
  }

  int start = lock.load(std::memory_order_relaxed);
  int current = start;
  while (current & 1 == 1) {
    current = lock.load(std::memory_order_relaxed);
  }

  while (!lock.compare_exchange_strong(current, current + 1, std::memory_order_acq_rel)) {
    do {
      current = lock.load(std::memory_order_relaxed);
    } while (current & 1 == 1);
  }
  
  data->tries += current / 2 - start / 2 + 1;

  for (int i = 0; i < args.critical_work; i++) {
    NOP;
  }

  lock.store(current + 2, std::memory_order_release);
}                                                                                     	