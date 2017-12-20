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
#include <immintrin.h>

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

//#define NOP __asm__ __volatile__("")
#define NOP __asm__("")

struct cmd_line_args_t {
  unsigned time;
  unsigned threads_count;
  std::string pin_type;
  unsigned parallel_work;
  unsigned critical_work;
  std::string lock_type;
};

struct thread_data_t {
  unsigned tid;
  
  unsigned long queue;
  unsigned long iterations;

  unsigned total;

  std::atomic<bool> finish;
};

static void print_usage();
static void parse_cmd_line_args(cmd_line_args_t &args, int argc, char** argv);
static void* thread_fun(void* data);

void thread_main_simple(thread_data_t* data, int P, int C);
void thread_main_tts(thread_data_t* data, int P, int C);
void thread_main_ticket(thread_data_t* data, int P, int C);

void pin(pid_t t, int cpu);

unsigned (*pin_func) (unsigned );

unsigned greedy_pin(unsigned tid);
unsigned socket_pin(unsigned tid);
unsigned uneven_pin(unsigned tid);

static cmd_line_args_t args;

alignas(128) std::atomic<int> lock;
alignas(128) std::atomic<int> ticket;
alignas(128) int gl;

unsigned TIME;

int main(int argc, char **argv) {
//  std::cerr << &gl << " " <<  &lock << " " << &ticket << std::endl;

  args.pin_type = "greedy";

  lock = 0;

  ticket = 0;

  gl = 0;

  parse_cmd_line_args(args, argc, argv);

  thread_data_t** thread_data = (thread_data_t**) cache_size_aligned_alloc(
                                                                           sizeof(thread_data_t*) * args.threads_count);
  for (unsigned i = 0; i < args.threads_count; i++) {
    thread_data_t* td = (thread_data_t*) cache_size_aligned_alloc(sizeof(thread_data_t));
    
    td->tid = i;

    td->queue = 0;
    td->iterations = 0;

    td->finish = false;

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
    thread_data[i]->finish.store(true, std::memory_order_relaxed);
  }

  for (unsigned i = 0; i < args.threads_count; i++) {
    pthread_join(threads[i], NULL);
  }

  std::atomic_thread_fence(std::memory_order_acquire);

  unsigned long total_iterations = 0;
  unsigned long total_queue = 0;

  for (int i = 0; i < args.threads_count; i++) {
    total_iterations += thread_data[i]->iterations;
    total_queue += thread_data[i]->queue;
  }

  std::cerr << total_iterations << " " << TIME << std::endl;

  printf("Throughput: %f op/sec\n", 1. * total_iterations / TIME);
  printf("Average queue: %f\n", 1. * total_queue / total_iterations);

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
/*    printf("Set for %d:\n", t);
    for (j = 0; j < CPU_SETSIZE; j++)
      if (CPU_ISSET(j, &cpuset))
        printf("    CPU %d\n", j);*/
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
  args.lock_type = deepsea::cmdline::parse_or_default_string("lock", "simple");
  
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
  "\tthread count       = " << args.threads_count << "\n" <<
  "\ttime count         = " << args.time << "\n" <<
  "\tcritical           = " << args.critical_work << "\n" <<
  "\tparallel           = " << args.parallel_work << "\n";
}

unsigned greedy_pin(unsigned tid) {
  unsigned target = tid;
//  printf("thread %d to %d\n", tid, target);
  return target;
}

unsigned socket_pin(unsigned tid) {
  unsigned cores = MAX_THREADS / PER_CORE;
  unsigned target = (tid % cores) * PER_CORE + tid / cores;
//  printf("thread %d to %d\n", tid, target);
  return target;
}

static void* thread_fun(void* data) {
  if (args.pin_type == "greedy") {
//      std::cerr << "greedy\n";
      pin(((thread_data_t *)data)->tid, greedy_pin(((thread_data_t *) data)->tid));
  } else if (args.pin_type == "socket") {
      pin(((thread_data_t *)data)->tid, socket_pin(((thread_data_t *) data)->tid));
  } else {
      printf( "wrong pin parameter!" );
      exit(0);
  }
  
  thread_data_t* thread_data = (thread_data_t*) data; 

  int P = args.parallel_work;
  int C = args.critical_work;
  int iterations = 0;
  int queue = 0;
  if (args.lock_type == "simple") {
    thread_main_simple(thread_data, P, C); 
  } else if (args.lock_type == "tts") {
    thread_main_tts(thread_data, P, C); 
  } else if (args.lock_type == "ticket") {
    thread_main_ticket(thread_data, P, C);
  } else {
    std::cerr << "Wrong lock parameter" << std::endl;
  }

  std::atomic_thread_fence(std::memory_order_seq_cst);
  
  return NULL;
}

void thread_main_simple(thread_data_t* data, int P, int C) {
  int zero = 0;
  int iterations = 0;
  int queue = 0;

//  int last_gl = 0;
//  int in_sequence = 0;
//  int total = 0;
  while (!data->finish.load(std::memory_order_relaxed)) {
    iterations++;
    for (int i = 0; i < P; i++) {
      NOP;
    }

    zero = 0;
    while (!lock.compare_exchange_strong(zero, 1, std::memory_order_acq_rel)) {
        zero = 0;
    }
    gl++;
//    if (last_gl != gl - 1) {
//      total++;
//    }
//    in_sequence++;
//    last_gl = gl;

    for (int i = 0; i < C; i++) {
      NOP;
    }

    lock.store(0, std::memory_order_release);
  }

//  std::cerr << in_sequence / total << std::endl;
  data->iterations = iterations;
  data->queue = queue;
}

void thread_main_tts(thread_data_t* data, int P, int C) {
  int current, start;
  int iterations = 0;
  int queue = 0;
  while (!data->finish.load(std::memory_order_relaxed)) {
    iterations++;
    for (int i = 0; i < P; i++) {
      NOP;
    }

    start = lock.load(std::memory_order_relaxed);
    current = start;
    while (current & 1 == 1) {
      current = lock.load(std::memory_order_relaxed);
    }
    while (!lock.compare_exchange_strong(current, current + 1, std::memory_order_acq_rel)) {
      do {
        current = lock.load(std::memory_order_relaxed);
      } while (current & 1 == 1);
    }
    
    queue += current / 2 - start / 2 + 1;

    gl++;

    for (int i = 0; i < C; i++) {
      NOP;
    }

    lock.store(current + 2, std::memory_order_release);
  }
  data->iterations = iterations;
  data->queue = queue;
}

void thread_main_ticket(thread_data_t* data, int P, int C) {
  int current, start;
  int iterations = 0;
  int queue = 0;
  while (!data->finish.load(std::memory_order_relaxed)) {
    iterations++;
    for (int i = 0; i < P; i++) {
      NOP;
    }

    start = ticket.fetch_add(1, std::memory_order_relaxed);
    current = lock.load(std::memory_order_relaxed);
    queue += start - current + 1;

    do {
    } while (lock.load(std::memory_order_relaxed) != start);
    std::atomic_thread_fence(std::memory_order_acquire);

    gl++;

    for (int i = 0; i < C; i++) {
      NOP;
    }
    lock.store(start + 1, std::memory_order_release);
  }
  data->iterations = iterations;
  data->queue = queue;
}