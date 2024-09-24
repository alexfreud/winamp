  #include "ThreadQueue.h"
#include <assert.h>
#include <time.h>

static inline __attribute__((always_inline))
void get_exceed_time(struct timespec* ptime, long delay)
{
    clock_gettime(CLOCK_REALTIME, ptime);

    ptime->tv_nsec += delay;
    if (ptime->tv_nsec >= 1000000000L) // overflow
    {
        ptime->tv_nsec -= 1000000000L;
        ++ptime->tv_sec;
    }
}

ThreadQueue::ThreadQueue()
{
  buffer.reserve(256 * sizeof(void *));
  sem_init(&event, 0, 0);
}

ThreadQueue::~ThreadQueue()
{
  sem_destroy(&event);
}

void ThreadQueue::Queue(const void *in)
{
  buffer.write(&in, sizeof(in));
  sem_post(&event);
}

void *ThreadQueue::Get()
{
  sem_wait(&event);
  void *out=0;
  size_t read = buffer.read(&out, sizeof(out));
  assert(read == sizeof(out));
  return out;
}

int ThreadQueue::Wait(long delay, void **val)
{
  timespec t;
  get_exceed_time(&t, delay);
  int ret = sem_timedwait(&event, &t);
  if (ret == 0)
  {
    size_t read = buffer.read(val, sizeof(*val));
    assert(read == sizeof(*val));
  }
  return ret;
}

int ThreadQueue::Try(void **val)
{
  int ret = sem_trywait(&event);
  if (ret == 0)
  {
    size_t read = buffer.read(val, sizeof(*val));
    assert(read == sizeof(*val));
  }
  return ret;
}
