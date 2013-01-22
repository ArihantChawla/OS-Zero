/* This code was donated by Matthew 'kinetik' Gregan. Thanks, Matthew. :) */

#include <sys/time.h>

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

struct list_node
{
  void * m;
  struct list_node * n;
};

struct queue
{
  struct list_node * h;
  pthread_mutex_t m;
  pthread_cond_t c_empty;
  pthread_cond_t c_full;
  int done;
};

struct task
{
  void (*f)(struct task * t);
  struct queue * q;
};

static pthread_barrier_t barrier;
static int g_threads;
static int g_size;
static int g_tasks;
static int g_loops;
static int g_debug;
static int g_touchmem;

static void
debugprint(const char * from, const char * msg)
{
  if (g_debug)
    fprintf(stderr, "%x: %s: %s\n", (unsigned int) pthread_self(), from, msg);
}

static void *
threadmain(void * v)
{
  struct task * t = v;
  int rc = pthread_barrier_wait(&barrier);
  assert(rc == 0 || rc == PTHREAD_BARRIER_SERIAL_THREAD);
  t->f(t);
  return NULL;
}

static void
producer(struct task * t)
{
  struct queue * q = t->q;
  int tasks;
  int loops;
  struct list_node * node;
  struct list_node * newnode;
  struct timeval t1, t2, t3;
  unsigned long long maxalloc = 0;
  unsigned long long tmp;

  for (loops = 0; loops < g_loops; ++loops)
    {
      assert(pthread_mutex_lock(&q->m) == 0);
      while (q->h)
	{
	  assert(pthread_cond_wait(&q->c_empty, &q->m) == 0);

	  if (q->h)
	    continue;
	}

      node = NULL;
      tasks = g_tasks;
      while (tasks--)
	{
	  gettimeofday(&t1, NULL);
	  newnode = malloc(sizeof(*newnode));
	  newnode->m = malloc(g_size);
	  if (g_touchmem)
	    memset(newnode->m, '\1', g_size);
	  gettimeofday(&t2, NULL);
	  timersub(&t2, &t1, &t3);
	  tmp = t3.tv_sec;
	  tmp *= 1000000;
	  tmp += t3.tv_usec;
	  if (tmp > maxalloc)
	    maxalloc = tmp;
	  newnode->n = NULL;
	  if (!node)
	    {
	      node = newnode;
	    }
	  else
	    {
	      newnode->n = node;
	      node = newnode;
	    }
	  debugprint("producer", "creating chunk");
	}

      q->h = node;

      debugprint("producer", "broadcast");
      assert(pthread_mutex_unlock(&q->m) == 0);
      assert(pthread_cond_signal(&q->c_full) == 0);
    }

  debugprint("producer", "done");
  //fprintf(stderr, "producer: max alloc: %llu\n", maxalloc);
  assert(pthread_mutex_lock(&q->m) == 0);
  q->done = 1;
  assert(pthread_mutex_unlock(&q->m) == 0);
  assert(pthread_cond_signal(&q->c_full) == 0);

  free(t);
}

static void
consumer(struct task * t)
{
  struct queue * q = t->q;
  int done = 0;
  struct list_node * node;
  struct list_node * oldnode;
  struct timeval t1, t2, t3;
  unsigned long long maxfree = 0;
  unsigned long long tmp;

  while (!done)
    {
      assert(pthread_mutex_lock(&q->m) == 0);

      while (!q->h && !q->done)
	{
	  assert(pthread_cond_wait(&q->c_full, &q->m) == 0);

	  if (!q->h && !q->done)
	    continue;
	}

      done = q->done;

      node = q->h;
      while (node)
	{
	  debugprint("consumer", "processing chunk");
	  gettimeofday(&t1, NULL);
	  if (g_touchmem)
	    memset(node->m, '\2', g_size);
	  free(node->m);
	  gettimeofday(&t2, NULL);
	  timersub(&t2, &t1, &t3);
	  tmp = t3.tv_sec;
	  tmp *= 1000000;
	  tmp += t3.tv_usec;
	  if (tmp > maxfree)
	    maxfree = tmp;
	  oldnode = node;
	  node = node->n;
	  free(oldnode);
	}
      q->h = NULL;

      debugprint("consumer", "broadcast");
      assert(pthread_mutex_unlock(&q->m) == 0);
      assert(pthread_cond_signal(&q->c_empty) == 0);
    }

  debugprint("consumer", "done");
  //fprintf(stderr, "consumer: max free: %llu\n", maxfree);
  q->h = NULL;
  assert(pthread_cond_destroy(&q->c_empty) == 0);
  assert(pthread_cond_destroy(&q->c_full) == 0);
  assert(pthread_mutex_destroy(&q->m) == 0);
  q->done = 0;
  free(q);
  t->q = NULL;
  free(t);
}

static void
usage(char const * prog)
{
  fprintf(stderr, "Usage: %s [-d] [-h] [-l loops] [-m] [-t threads] [-s allocsize] [-w work]\n", prog);
  fprintf(stderr, "\nOptions:\n");
  fprintf(stderr, "-d              debug logging\n");
  fprintf(stderr, "-h              print usage\n");
  fprintf(stderr, "-l loops        producer loop iterations\n");
  fprintf(stderr, "-m              touch memory during allocations\n");
  fprintf(stderr, "-t threads      thread count (must be even)\n");
  fprintf(stderr, "-s allocsize    size of each work allocation\n");
  fprintf(stderr, "-w work         work units per loop iteration\n");
  exit(EXIT_FAILURE);
}

int
main(int argc, char * argv[])
{
  int i;
  int opt;
  pthread_t * tcbs;
  struct timeval t1, t2, t3;

  g_threads = 2;
  g_size = 128;
  g_tasks = 10;
  g_loops = 1000;

  while ((opt = getopt(argc, argv, "dhl:mt:s:w:")) != -1)
    {
      switch (opt)
	{
	case 'd':
	  g_debug = 1;
	  break;
	case 'l':
	  g_loops = atoi(optarg);
	  break;
	case 'm':
	  g_touchmem = 1;
	  break;
	case 't':
	  g_threads = atoi(optarg);
	  break;
	case 's':
	  g_size = atoi(optarg);
	  break;
	case 'w':
	  g_tasks = atoi(optarg);
	  break;
	default:
	  usage(argv[0]);
	}
    }

  if (g_threads % 2)
    usage(argv[0]);

#if defined(__FreeBSD__)
  pthread_setconcurrency(g_threads);
#endif

//  fprintf(stderr, "debug:     %d\n", g_debug);
//  fprintf(stderr, "loops:     %d\n", g_loops);
//  fprintf(stderr, "touchmem:  %d\n", g_touchmem);
//  fprintf(stderr, "threads:   %d\n", g_threads);
//  fprintf(stderr, "allocsize: %d\n", g_size);
//  fprintf(stderr, "work:      %d\n", g_tasks);

  assert(pthread_barrier_init(&barrier, NULL, g_threads) == 0);

  tcbs = malloc(g_threads * sizeof(*tcbs));
  for (i = 0; i < g_threads; i += 2)
    {
      struct queue * q;
      struct task * t;

      q = malloc(sizeof(*q));
      q->h = NULL;
      assert(pthread_cond_init(&q->c_full, NULL) == 0);
      assert(pthread_cond_init(&q->c_empty, NULL) == 0);
      assert(pthread_mutex_init(&q->m, NULL) == 0);
      q->done = 0;

      t = malloc(sizeof(*t));
      t->f = producer;
      t->q = q;
      assert(pthread_create(&tcbs[i + 0], NULL, threadmain, t) == 0);

      t = malloc(sizeof(*t));
      t->f = consumer;
      t->q = q;
      assert(pthread_create(&tcbs[i + 1], NULL, threadmain, t) == 0);
    }

  gettimeofday(&t1, NULL);
  for (i = 0; i < g_threads; ++i)
    assert(pthread_join(tcbs[i], NULL) == 0);
  gettimeofday(&t2, NULL);
  timersub(&t2, &t1, &t3);
  fprintf(stderr, "%.4f seconds\n", (t3.tv_sec * 1000000.0 + t3.tv_usec) / 1000000.0);

  free(tcbs);
  assert(pthread_barrier_destroy(&barrier) == 0);

  return EXIT_SUCCESS;
}
