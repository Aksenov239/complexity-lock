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

  bool hand_finish;
};

static void print_usage();
static void parse_cmd_line_args(cmd_line_args_t &args, int argc, char** argv);
static void* thread_fun(void* data);
static void thread_main_simple(thread_data_t* data);
static void thread_main_tts(thread_data_t* data);
static void thread_main_ticket(thread_data_t* data);

void pin(pid_t t, int cpu);

unsigned (*pin_func) (unsigned );

unsigned greedy_pin(unsigned tid);
unsigned socket_pin(unsigned tid);
unsigned uneven_pin(unsigned tid);

static cmd_line_args_t args;

int p1[128];

volatile int hand_lock = 0;

int p2[128];

volatile int hand_ticket = 0;

unsigned TIME;

int main(int argc, char **argv) {
  args.pin_type = "greedy";

  hand_lock = 0;

  hand_ticket = 0;

  parse_cmd_line_args(args, argc, argv);

  thread_data_t** thread_data = (thread_data_t**) cache_size_aligned_alloc(
                                                                           sizeof(thread_data_t*) * args.threads_count);
  for (unsigned i = 0; i < args.threads_count; i++) {
    thread_data_t* td = (thread_data_t*) cache_size_aligned_alloc(sizeof(thread_data_t));
    
    td->tid = i;

    td->queue = 0;
    td->iterations = 0;

    td->hand_finish = false;

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
    thread_data[i]->hand_finish = true;
  }

  std::atomic_thread_fence(std::memory_order_seq_cst);

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
    int zero = 0;

    while (!thread_data->hand_finish) {
      iterations++;
      for (int i = 0; i < P; i++) {
        NOP;
      }

      while (__sync_val_compare_and_swap(&hand_lock, 0, 1) != 0) {
      }

      for (int i = 0; i < C; i++) {
        NOP;
      }

      std::atomic_thread_fence(std::memory_order_release);
      hand_lock = 0;
    }
  } else if (args.lock_type == "tts") {
    int current, start;
    while (!thread_data->hand_finish) {
      iterations++;
      for (int i = 0; i < P; i++) {
        NOP;
      }

      start = hand_lock;
      current = start;
      while (current & 1 == 1) {
//        std::atomic_thread_fence(std::memory_order_seq_cst);
//        std::atomic_thread_fence(std::memory_order_acquire);
//        __asm__ __volatile__("":::"memory");
        current = hand_lock;
      }

      while (__sync_val_compare_and_swap(&hand_lock, current, current + 1) != current) {
        do {
          current = hand_lock;
//          std::atomic_thread_fence(std::memory_order_seq_cst);
//          std::atomic_thread_fence(std::memory_order_acquire);
//          __asm__ __volatile__("":::"memory");
        } while (current & 1 == 1);
      }
//      std::atomic_thread_fence(std::memory_order_acquire);

      queue += current / 2 - start / 2 + 1;

      for (int i = 0; i < C; i++) {
        NOP;
      }
                   
      std::atomic_thread_fence(std::memory_order_release);      	
      hand_lock = current + 2;
    }
  } else if (args.lock_type == "ticket") {
    int current, start;
    while (!thread_data->hand_finish) {
      iterations++;
      for (int i = 0; i < P; i++) {
        NOP;
      }

      start = __sync_fetch_and_add(&hand_ticket, 1);
      current = hand_lock;
      queue += start - current + 1;

      do {
//        std::atomic_thread_fence(std::memory_order_acquire);
//        std::atomic_thread_fence(std::memory_order_seq_cst);
//        __asm__ __volatile__("":::"memory");
      } while (hand_lock != start);
      std::atomic_thread_fence(std::memory_order_acquire);

      for (int i = 0; i < C; i++) {
        NOP;
      }

      std::atomic_thread_fence(std::memory_order_release);
      hand_lock = start + 1;
    }
  } else {
    std::cerr << "Wrong lock parameter" << std::endl;
  }

  thread_data->iterations = iterations;
  thread_data->queue = queue;

  std::atomic_thread_fence(std::memory_order_release);
  
  return NULL;
}
