/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


#include "fluid_sys.h"

static char fluid_errbuf[512];  /* buffer for error message */

static fluid_log_function_t fluid_log_function[LAST_LOG_LEVEL];
static void* fluid_log_user_data[LAST_LOG_LEVEL];
static int fluid_log_initialized = 0;

static char* fluid_libname = "fluidsynth";


void fluid_sys_config()
{
  fluid_log_config();
  fluid_time_config();
}


unsigned int fluid_debug_flags = 0;

#if DEBUG
/*
 * fluid_debug
 */
int fluid_debug(int level, char * fmt, ...)
{
  if (fluid_debug_flags & level) {
    fluid_log_function_t fun;
    va_list args;

    va_start (args, fmt);
    vsnprintf(fluid_errbuf, sizeof (fluid_errbuf), fmt, args);
    va_end (args);

    fun = fluid_log_function[FLUID_DBG];
    if (fun != NULL) {
      (*fun)(level, fluid_errbuf, fluid_log_user_data[FLUID_DBG]);
    }
  }
  return 0;
}
#endif

/**
 * Installs a new log function for a specified log level.
 * @param level Log level to install handler for.
 * @param fun Callback function handler to call for logged messages
 * @param data User supplied data pointer to pass to log function
 * @return The previously installed function.
 */
fluid_log_function_t
fluid_set_log_function(int level, fluid_log_function_t fun, void* data)
{
  fluid_log_function_t old = NULL;

  if ((level >= 0) && (level < LAST_LOG_LEVEL)) {
    old = fluid_log_function[level];
    fluid_log_function[level] = fun;
    fluid_log_user_data[level] = data;
  }
  return old;
}

/**
 * Default log function which prints to the stderr.
 * @param level Log level
 * @param message Log message
 * @param data User supplied data (not used)
 */
void
fluid_default_log_function(int level, char* message, void* data)
{
  FILE* out;

#if defined(WIN32)
  out = stdout;
#else
  out = stderr;
#endif

  if (fluid_log_initialized == 0) {
    fluid_log_config();
  }

  switch (level) {
  case FLUID_PANIC:
    FLUID_FPRINTF(out, "%s: panic: %s\n", fluid_libname, message);
    break;
  case FLUID_ERR:
    FLUID_FPRINTF(out, "%s: error: %s\n", fluid_libname, message);
    break;
  case FLUID_WARN:
    FLUID_FPRINTF(out, "%s: warning: %s\n", fluid_libname, message);
    break;
  case FLUID_INFO:
    FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
    break;
  case FLUID_DBG:
#if DEBUG
    FLUID_FPRINTF(out, "%s: debug: %s\n", fluid_libname, message);
#endif
    break;
  default:
    FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
    break;
  }
  fflush(out);
}

/*
 * fluid_init_log
 */
void
fluid_log_config(void)
{
  if (fluid_log_initialized == 0) {

    fluid_log_initialized = 1;

    if (fluid_log_function[FLUID_PANIC] == NULL) {
      fluid_set_log_function(FLUID_PANIC, fluid_default_log_function, NULL);
    }

    if (fluid_log_function[FLUID_ERR] == NULL) {
      fluid_set_log_function(FLUID_ERR, fluid_default_log_function, NULL);
    }

    if (fluid_log_function[FLUID_WARN] == NULL) {
      fluid_set_log_function(FLUID_WARN, fluid_default_log_function, NULL);
    }

    if (fluid_log_function[FLUID_INFO] == NULL) {
      fluid_set_log_function(FLUID_INFO, fluid_default_log_function, NULL);
    }

    if (fluid_log_function[FLUID_DBG] == NULL) {
      fluid_set_log_function(FLUID_DBG, fluid_default_log_function, NULL);
    }
  }
}

/**
 * Print a message to the log.
 * @param level Log level (#fluid_log_level).
 * @param fmt Printf style format string for log message
 * @param ... Arguments for printf 'fmt' message string
 * @return Always returns -1
 */
int
fluid_log(int level, char* fmt, ...)
{
  fluid_log_function_t fun = NULL;

  va_list args;
  va_start (args, fmt);
  vsnprintf(fluid_errbuf, sizeof (fluid_errbuf), fmt, args);
  va_end (args);

  if ((level >= 0) && (level < LAST_LOG_LEVEL)) {
    fun = fluid_log_function[level];
    if (fun != NULL) {
      (*fun)(level, fluid_errbuf, fluid_log_user_data[level]);
    }
  }
  return FLUID_FAILED;
}

/**
 * An improved strtok, still trashes the input string, but is portable and
 * thread safe.  Also skips token chars at beginning of token string and never
 * returns an empty token (will return NULL if source ends in token chars though).
 * NOTE: NOT part of public API
 * @internal
 * @param str Pointer to a string pointer of source to tokenize.  Pointer gets
 *   updated on each invocation to point to beginning of next token.  Note that
 *   token char get's overwritten with a 0 byte.  String pointer is set to NULL
 *   when final token is returned.
 * @param delim String of delimiter chars.
 * @return Pointer to the next token or NULL if no more tokens.
 */
char *fluid_strtok (char **str, char *delim)
{
  char *s, *d, *token;
  char c;

  if (str == NULL || delim == NULL || !*delim)
  {
    FLUID_LOG(FLUID_ERR, "Null pointer");
    return NULL;
  }

  s = *str;
  if (!s) return NULL;	/* str points to a NULL pointer? (tokenize already ended) */

  /* skip delimiter chars at beginning of token */
  do
  {
    c = *s;
    if (!c)	/* end of source string? */
    {
      *str = NULL;
      return NULL;
    }

    for (d = delim; *d; d++)	/* is source char a token char? */
    {
      if (c == *d)	/* token char match? */
      {
	s++;		/* advance to next source char */
	break;
      }
    }
  } while (*d);		/* while token char match */

  token = s;		/* start of token found */

  /* search for next token char or end of source string */
  for (s = s+1; *s; s++)
  {
    c = *s;

    for (d = delim; *d; d++)	/* is source char a token char? */
    {
      if (c == *d)	/* token char match? */
      {
	*s = '\0';	/* overwrite token char with zero byte to terminate token */
	*str = s+1;	/* update str to point to beginning of next token */
	return token;
      }
    }
  }

  /* we get here only if source string ended */
  *str = NULL;
  return token;
}

/*
 * fluid_error
 */
char*
fluid_error()
{
  return fluid_errbuf;
}


/*
 *
 *  fluid_is_midifile
 */
int
fluid_is_midifile(char* filename)
{
  FILE* fp = fopen(filename, "rb");
  char id[4];

  if (fp == NULL) {
    return 0;
  }
  if (fread((void*) id, 1, 4, fp) != 4) {
    fclose(fp);
    return 0;
  }
  fclose(fp);

  return strncmp(id, "MThd", 4) == 0;
}

/*
 *  fluid_is_soundfont
 *
 */
int
fluid_is_soundfont(char* filename)
{
  FILE* fp = fopen(filename, "rb");
  char id[4];

  if (fp == NULL) {
    return 0;
  }
  if (fread((void*) id, 1, 4, fp) != 4) {
    fclose(fp);
    return 0;
  }
  fclose(fp);

  return strncmp(id, "RIFF", 4) == 0;
}

#if defined(WIN32)

/*=============================================================*/
/*                                                             */
/*                           Win32                             */
/*                                                             */
/*=============================================================*/

/***************************************************************
 *
 *               Timer
 *
 */

struct _fluid_timer_t
{
  long msec;
  fluid_timer_callback_t callback;
  void* data;
  HANDLE thread;
  DWORD thread_id;
  int cont;
  int auto_destroy;
};

static int fluid_timer_count = 0;
DWORD WINAPI fluid_timer_run(LPVOID data);

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
	       int new_thread, int auto_destroy)
{
  fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
  if (timer == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  timer->cont = 1;
  timer->msec = msec;
  timer->callback = callback;
  timer->data = data;
  timer->thread = 0;
  timer->auto_destroy = auto_destroy;

  if (new_thread) {
    timer->thread = CreateThread(NULL, 0, fluid_timer_run, (LPVOID) timer, 0, &timer->thread_id);
    if (timer->thread == NULL) {
      FLUID_LOG(FLUID_ERR, "Couldn't create timer thread");
      FLUID_FREE(timer);
      return NULL;
    }
    SetThreadPriority(timer->thread, THREAD_PRIORITY_TIME_CRITICAL);
  } else {
    fluid_timer_run((LPVOID) timer);
  }
  return timer;
}

DWORD WINAPI
fluid_timer_run(LPVOID data)
{
  int count = 0;
  int cont = 1;
  long start;
  long delay;
  fluid_timer_t* timer;
  timer = (fluid_timer_t*) data;

  if ((timer == NULL) || (timer->callback == NULL)) {
    return 0;
  }

  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

  /* keep track of the start time for absolute positioning */
  start = fluid_curtime();

  while (cont) {

    /* do whatever we have to do */
    cont = (*timer->callback)(timer->data, fluid_curtime() - start);

    count++;

    /* to avoid incremental time errors, I calculate the delay between
       two callbacks bringing in the "absolute" time (count *
       timer->msec) */
    delay = (count * timer->msec) - (fluid_curtime() - start);
    if (delay > 0) {
      Sleep(delay);
    }

    cont &= timer->cont;
  }

  FLUID_LOG(FLUID_DBG, "Timer thread finished");

  if (timer->auto_destroy) {
    FLUID_FREE(timer);
  }

  ExitThread(0);
  return 0;
}

int
delete_fluid_timer(fluid_timer_t* timer)
{
  timer->cont = 0;
  fluid_timer_join(timer);
  FLUID_FREE(timer);
  return FLUID_OK;
}

int
fluid_timer_join(fluid_timer_t* timer)
{
  DWORD wait_result;
  if (timer->thread == 0) {
    return FLUID_OK;
  }
  wait_result = WaitForSingleObject(timer->thread, INFINITE);
  return (wait_result == WAIT_OBJECT_0)? FLUID_OK : FLUID_FAILED;
}


/***************************************************************
 *
 *               Time
 */

double rdtsc(void);
double fluid_estimate_cpu_frequency(void);

static double fluid_cpu_frequency = -1.0;

void fluid_time_config(void)
{
  if (fluid_cpu_frequency < 0.0) {
    fluid_cpu_frequency = fluid_estimate_cpu_frequency() / 1000000.0;
  }
}

double fluid_utime(void)
{
  return (rdtsc() / fluid_cpu_frequency);
}

double rdtsc(void)
{
  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);
  return (double) t.QuadPart;
}

double fluid_estimate_cpu_frequency(void)
{
#if 0
  LONGLONG start, stop, ticks;
  unsigned int before, after, delta;
  double freq;

  start = rdtsc();
  stop = start;
  before = fluid_curtime();
  after = before;

  while (1) {
    if (after - before > 1000) {
	break;
    }
    after = fluid_curtime();
    stop = rdtsc();
  }

  delta = after - before;
  ticks = stop - start;

  freq = 1000 * ticks / delta;

  return freq;

#else
  unsigned int before, after;
  LARGE_INTEGER start, stop;

  before = fluid_curtime();
  QueryPerformanceCounter(&start);

  Sleep(1000);

  after = fluid_curtime();
  QueryPerformanceCounter(&stop);

  return (double) 1000 * (stop.QuadPart - start.QuadPart) / (after - before);
#endif
}



#elif defined(MACOS9)
/*=============================================================*/
/*                                                             */
/*                           MacOS 9                           */
/*                                                             */
/*=============================================================*/


/***************************************************************
 *
 *               Timer
 */

struct _fluid_timer_t
{
	TMTask myTmTask;
  long msec;
  unsigned int start;
  unsigned int count;
  int isInstalled;
  fluid_timer_callback_t callback;
  void* data;
  int auto_destroy;
};

static TimerUPP	myTimerUPP;

void
_timerCallback(fluid_timer_t *timer)
{
	int cont;
  cont = (*timer->callback)(timer->data, fluid_curtime() - timer->start);
  if (cont) {
  	PrimeTime((QElemPtr)timer, timer->msec);
	} else {
		timer->isInstalled = 0;
	}
  timer->count++;
}

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
	       int new_thread, int auto_destroy)
{
  fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
  if (timer == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

	if (!myTimerUPP)
		myTimerUPP = NewTimerProc(_timerCallback);

  /* setup tmtask */
	timer->myTmTask.tmAddr = myTimerUPP;
	timer->myTmTask.qLink = NULL;
	timer->myTmTask.qType = 0;
	timer->myTmTask.tmCount = 0L;
	timer->myTmTask.tmWakeUp = 0L;
	timer->myTmTask.tmReserved = 0L;

  timer->callback = callback;

  timer->msec = msec;
  timer->data = data;
  timer->start = fluid_curtime();
  timer->isInstalled = 1;
  timer->count = 0;
  timer->auto_destroy = auto_destroy;

  InsXTime((QElemPtr)timer);
  PrimeTime((QElemPtr)timer, msec);

  return timer;
}

int
delete_fluid_timer(fluid_timer_t* timer)
{
	if (timer->isInstalled) {
		RmvTime((QElemPtr)timer);
	}
  FLUID_FREE(timer);
  return FLUID_OK;
}

int
fluid_timer_join(fluid_timer_t* timer)
{
	if (timer->isInstalled) {
		int count = timer->count;
		/* wait until count has incremented */
		while (count == timer->count) {}
	}
  return FLUID_OK;
}

/***************************************************************
 *
 *               Time
 */
#define kTwoPower32 (4294967296.0)      /* 2^32 */

void fluid_time_config(void)
{
}

unsigned int fluid_curtime()
{
	/* could be optimized by not going though a double */
	UnsignedWide    uS;
	double mSf;
	unsigned int ms;

	Microseconds(&uS);

  mSf = ((((double) uS.hi) * kTwoPower32) + uS.lo)/1000.0f;

  ms = mSf;

  return (ms);
}



#else

/*=============================================================*/
/*                                                             */
/*                           POSIX                             */
/*                                                             */
/*=============================================================*/


/***************************************************************
 *
 *               Timer
 */

struct _fluid_timer_t
{
  long msec;
  fluid_timer_callback_t callback;
  void* data;
  pthread_t thread;
  int cont;
  int auto_destroy;
};

void*
fluid_timer_start(void *data)
{
  int count = 0;
  int cont = 1;
  long start;
  long delay;
  fluid_timer_t* timer;
  timer = (fluid_timer_t*) data;

  /* keep track of the start time for absolute positioning */
  start = fluid_curtime();

  while (cont) {

    /* do whatever we have to do */
    cont = (*timer->callback)(timer->data, fluid_curtime() - start);

    count++;

    /* to avoid incremental time errors, calculate the delay between
       two callbacks bringing in the "absolute" time (count *
       timer->msec) */
    delay = (count * timer->msec) - (fluid_curtime() - start);
    if (delay > 0) {
      usleep(delay * 1000);
    }

    cont &= timer->cont;
  }

  FLUID_LOG(FLUID_DBG, "Timer thread finished");
  if (timer->thread != 0) {
    pthread_exit(NULL);
  }

  if (timer->auto_destroy) {
    FLUID_FREE(timer);
  }

  return NULL;
}

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
	       int new_thread, int auto_destroy)
{
  pthread_attr_t *attr = NULL;
  pthread_attr_t rt_attr;
  int sched = SCHED_FIFO;
  struct sched_param priority;
  int err;

  fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
  if (timer == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }
  timer->msec = msec;
  timer->callback = callback;
  timer->data = data;
  timer->cont = 1;
  timer->thread = 0;
  timer->auto_destroy = auto_destroy;

  err = pthread_attr_init(&rt_attr);
  if (err == 0) {
	  err = pthread_attr_setschedpolicy(&rt_attr, SCHED_FIFO);
	  if (err == 0) {
		  priority.sched_priority = 10;
		  err = pthread_attr_setschedparam(&rt_attr, &priority);
		  if (err == 0) {
			  attr = &rt_attr;
		  }
	  }
  }

  if (new_thread) {
	  err = pthread_create(&timer->thread, attr, fluid_timer_start, (void*) timer);
	  if (err == 0) {
		  FLUID_LOG(FLUID_DBG, "The timer thread was created with real-time priority");
	  } else {
		  /* Create the thread with default attributes */
		  err = pthread_create(&timer->thread, NULL, fluid_timer_start, (void*) timer);
		  if (err != 0) {
			  FLUID_LOG(FLUID_ERR, "Failed to create the timer thread");
			  FLUID_FREE(timer);
			  return NULL;
		  } else {
			  FLUID_LOG(FLUID_DBG, "The timer thread does not have real-time priority");
		  }
	  }
  } else {
    fluid_timer_start((void*) timer);
  }
  return timer;
}

int
delete_fluid_timer(fluid_timer_t* timer)
{
  timer->cont = 0;
  fluid_timer_join(timer);
  FLUID_LOG(FLUID_DBG, "Joined player thread");
  FLUID_FREE(timer);
  return FLUID_OK;
}

int
fluid_timer_join(fluid_timer_t* timer)
{
  int err = 0;

  if (timer->thread != 0) {
    err = pthread_join(timer->thread, NULL);
  }
  FLUID_LOG(FLUID_DBG, "Joined player thread");
  return (err == 0)? FLUID_OK : FLUID_FAILED;
}


/***************************************************************
 *
 *               Time
 */

static double fluid_cpu_frequency = -1.0;

double rdtsc(void);
double fluid_estimate_cpu_frequency(void);

void fluid_time_config(void)
{
  if (fluid_cpu_frequency < 0.0) {
    fluid_cpu_frequency = fluid_estimate_cpu_frequency() / 1000000.0;
    if (fluid_cpu_frequency == 0.0) fluid_cpu_frequency = 1.0;
  }
}

unsigned int fluid_curtime()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_sec * 1000 + now.tv_usec / 1000;
}

double fluid_utime(void)
{
  return (rdtsc() / fluid_cpu_frequency);
}

#if !defined(__i386__)

double rdtsc(void)
{
  return 0.0;
}

double fluid_estimate_cpu_frequency(void)
{
  return 1.0;
}

#else

double rdtsc(void)
{
  unsigned int a, b;

  __asm__ ("rdtsc" : "=a" (a), "=d" (b));
  return (double)b * (double)0x10000 * (double)0x10000 + a;
}

double fluid_estimate_cpu_frequency(void)
{
  double start, stop;
  unsigned int a0, b0, a1, b1;
  unsigned int before, after;

  before = fluid_curtime();
  __asm__ ("rdtsc" : "=a" (a0), "=d" (b0));

  sleep(1);

  after = fluid_curtime();
  __asm__ ("rdtsc" : "=a" (a1), "=d" (b1));


  start = (double)b0 * (double)0x10000 * (double)0x10000 + a0;
  stop = (double)b1 * (double)0x10000 * (double)0x10000 + a1;

  return 1000 * (stop - start) / (after - before);
}
#endif


#ifdef FPE_CHECK

/***************************************************************
 *
 *               Floating point exceptions
 *
 *  The floating point exception functions were taken from Ircam's
 *  jMax source code. http://www.ircam.fr/jmax
 *
 *  FIXME: check in config for i386 machine
 *
 *  Currently not used. I leave the code here in case we want to pick
 *  this up again some time later.
 */

/* Exception flags */
#define _FPU_STATUS_IE    0x001  /* Invalid Operation */
#define _FPU_STATUS_DE    0x002  /* Denormalized Operand */
#define _FPU_STATUS_ZE    0x004  /* Zero Divide */
#define _FPU_STATUS_OE    0x008  /* Overflow */
#define _FPU_STATUS_UE    0x010  /* Underflow */
#define _FPU_STATUS_PE    0x020  /* Precision */
#define _FPU_STATUS_SF    0x040  /* Stack Fault */
#define _FPU_STATUS_ES    0x080  /* Error Summary Status */

/* Macros for accessing the FPU status word.  */

/* get the FPU status */
#define _FPU_GET_SW(sw) __asm__ ("fnstsw %0" : "=m" (*&sw))

/* clear the FPU status */
#define _FPU_CLR_SW() __asm__ ("fnclex" : : )

/* Purpose:
 * Checks, if the floating point unit has produced an exception, print a message
 * if so and clear the exception.
 */
unsigned int fluid_check_fpe_i386(char* explanation)
{
  unsigned int s;

  _FPU_GET_SW(s);
  _FPU_CLR_SW();

  s &= _FPU_STATUS_IE | _FPU_STATUS_DE | _FPU_STATUS_ZE | _FPU_STATUS_OE | _FPU_STATUS_UE;

  if (s)
  {
      FLUID_LOG(FLUID_WARN, "FPE exception (before or in %s): %s%s%s%s%s", explanation,
	       (s & _FPU_STATUS_IE) ? "Invalid operation " : "",
	       (s & _FPU_STATUS_DE) ? "Denormal number " : "",
	       (s & _FPU_STATUS_ZE) ? "Zero divide " : "",
	       (s & _FPU_STATUS_OE) ? "Overflow " : "",
	       (s & _FPU_STATUS_UE) ? "Underflow " : "");
  }

  return s;
}

/* Purpose:
 * Clear floating point exception.
 */
void fluid_clear_fpe_i386 (void)
{
  _FPU_CLR_SW();
}

#endif	// ifdef FPE_CHECK


#endif	// #else    (its POSIX)


/***************************************************************
 *
 *               Profiling (Linux, i586 only)
 *
 */

#if WITH_PROFILING

fluid_profile_data_t fluid_profile_data[] =
{
  { FLUID_PROF_WRITE_S16,        "fluid_synth_write_s16           ", 1e10, 0.0, 0.0, 0},
  { FLUID_PROF_ONE_BLOCK,        "fluid_synth_one_block           ", 1e10, 0.0, 0.0, 0},
  { FLUID_PROF_ONE_BLOCK_CLEAR,  "fluid_synth_one_block:clear     ", 1e10, 0.0, 0.0, 0},
  { FLUID_PROF_ONE_BLOCK_VOICE,  "fluid_synth_one_block:one voice ", 1e10, 0.0, 0.0, 0},
  { FLUID_PROF_ONE_BLOCK_VOICES, "fluid_synth_one_block:all voices", 1e10, 0.0, 0.0, 0},
  { FLUID_PROF_ONE_BLOCK_REVERB, "fluid_synth_one_block:reverb    ", 1e10, 0.0, 0.0, 0},
  { FLUID_PROF_ONE_BLOCK_CHORUS, "fluid_synth_one_block:chorus    ", 1e10, 0.0, 0.0, 0},
  { FLUID_PROF_VOICE_NOTE,       "fluid_voice:note                ", 1e10, 0.0, 0.0, 0},
  { FLUID_PROF_VOICE_RELEASE,    "fluid_voice:release             ", 1e10, 0.0, 0.0, 0},
  { FLUID_PROF_LAST, "last", 1e100, 0.0, 0.0, 0}
};


void fluid_profiling_print(void)
{
  int i;

  printf("fluid_profiling_print\n");

  FLUID_LOG(FLUID_INFO, "Estimated CPU frequency: %.0f MHz", fluid_cpu_frequency);
  FLUID_LOG(FLUID_INFO, "Estimated times: min/avg/max (micro seconds)");

  for (i = 0; i < FLUID_PROF_LAST; i++) {
    if (fluid_profile_data[i].count > 0) {
      FLUID_LOG(FLUID_INFO, "%s: %.3f/%.3f/%.3f",
	       fluid_profile_data[i].description,
	       fluid_profile_data[i].min,
	       fluid_profile_data[i].total / fluid_profile_data[i].count,
	       fluid_profile_data[i].max);
    } else {
      FLUID_LOG(FLUID_DBG, "%s: no profiling available", fluid_profile_data[i].description);
    }
  }
}


#endif /* WITH_PROFILING */



/***************************************************************
 *
 *               Threads
 *
 */

#if defined(MACOS9)
/* Not implemented */
fluid_thread_t* new_fluid_thread(fluid_thread_func_t func, void* data, int detach) { return NULL; }
int delete_fluid_thread(fluid_thread_t* thread) { return 0; }
int fluid_thread_join(fluid_thread_t* thread) { return 0; }

#elif defined(WIN32)

struct _fluid_thread_t {
  HANDLE thread;
  DWORD thread_id;
  fluid_thread_func_t func;
  void* data;
  int detached;
};

static DWORD WINAPI fluid_thread_start(LPVOID data)
{
  fluid_thread_t* thread = (fluid_thread_t*) data;

  thread->func(thread->data);

  if (thread->detached) {
    FLUID_FREE(thread);
  }

  return 0;
}


fluid_thread_t* new_fluid_thread(fluid_thread_func_t func, void* data, int detach)
{
  fluid_thread_t* thread;

  if (func == NULL) {
    FLUID_LOG(FLUID_ERR, "Invalid thread function");
    return NULL;
  }

  thread = FLUID_NEW(fluid_thread_t);
  if (thread == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  thread->data = data;
  thread->func = func;
  thread->detached = detach;

  thread->thread = CreateThread(NULL, 0, fluid_thread_start, (LPVOID) thread,
				0, &thread->thread_id);
  if (thread->thread == NULL) {
    FLUID_LOG(FLUID_ERR, "Couldn't create the thread");
    FLUID_FREE(thread);
    return NULL;
  }

  return thread;
}

int delete_fluid_thread(fluid_thread_t* thread)
{
  FLUID_FREE(thread);
  return FLUID_OK;
}


int fluid_thread_join(fluid_thread_t* thread)
{
  DWORD wait_result;
  if (thread->thread == 0) {
    return FLUID_OK;
  }
  wait_result = WaitForSingleObject(thread->thread, INFINITE);
  return (wait_result == WAIT_OBJECT_0)? FLUID_OK : FLUID_FAILED;
}

#else


struct _fluid_thread_t {
  pthread_t pthread;
  fluid_thread_func_t func;
  void* data;
  int detached;
};

static void* fluid_thread_start(void *data)
{
  fluid_thread_t* thread = (fluid_thread_t*) data;

  thread->func(thread->data);

  if (thread->detached) {
    FLUID_FREE(thread);
  }

  return NULL;
}

fluid_thread_t* new_fluid_thread(fluid_thread_func_t func, void* data, int detach)
{
  fluid_thread_t* thread;
  pthread_attr_t attr;

  if (func == NULL) {
    FLUID_LOG(FLUID_ERR, "Invalid thread function");
    return NULL;
  }

  thread = FLUID_NEW(fluid_thread_t);
  if (thread == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  thread->data = data;
  thread->func = func;
  thread->detached = detach;

  pthread_attr_init(&attr);

  if (detach) {
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  }

  if (pthread_create(&thread->pthread, &attr, fluid_thread_start, thread)) {
    FLUID_LOG(FLUID_ERR, "Failed to create the thread");
    FLUID_FREE(thread);
    return NULL;
  }

  return thread;
}

int delete_fluid_thread(fluid_thread_t* thread)
{
  FLUID_FREE(thread);
  return FLUID_OK;
}

int fluid_thread_join(fluid_thread_t* thread)
{
  int err = 0;

  if (thread->pthread != 0) {
    err = pthread_join(thread->pthread, NULL);
  }
  return (err == 0)? FLUID_OK : FLUID_FAILED;
}

#endif



/***************************************************************
 *
 *               Sockets
 *
 */


#if defined(MACINTOSH)
/* Not implemented */


#elif defined(WIN32)

#if 0
typedef unsigned int socklen_t;

#define fluid_socket_read(_S,_B,_L) recv(_S,_B,_L,0)
#define fluid_socket_write(_S,_B,_L) send(_S,_B,_L,0)

void fluid_socket_close(fluid_socket_t sock)
{
  int r;
  char buf[1024];
  if (sock != INVALID_SOCKET) {
    shutdown(sock, 0x02);
    while (1) {
      r = recv(sock, buf, 1024, 0);
      if ((r == 0) || (r == SOCKET_ERROR)) {
	break;
      }
    }
    closesocket(sock);
  }
}
#endif


#else
#define fluid_socket_read(_S,_B,_L) read(_S,_B,_L)
#define fluid_socket_write(_S,_B,_L) write(_S,_B,_L)
#define SOCKET_ERROR -1

void fluid_socket_close(fluid_socket_t sock)
{
  if (sock != INVALID_SOCKET) {
    close(sock);
  }
}

#endif

#if !defined(MACINTOSH) && !defined(WIN32)


fluid_istream_t fluid_socket_get_istream(fluid_socket_t sock)
{
  return sock;
}

fluid_ostream_t fluid_socket_get_ostream(fluid_socket_t sock)
{
  return sock;
}



struct _fluid_server_socket_t {
  fluid_socket_t socket;
  fluid_thread_t* thread;
  int cont;
  fluid_server_func_t func;
  void* data;
};


static void fluid_server_socket_run(void* data)
{
  fluid_server_socket_t* server_socket = (fluid_server_socket_t*) data;
  fluid_socket_t client_socket;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  FLUID_LOG(FLUID_DBG, "Server listening for connections");

  while (server_socket->cont) {

    client_socket = accept(server_socket->socket, (struct sockaddr*) &addr, &addrlen);

    FLUID_LOG(FLUID_DBG, "New client connection");

    if (client_socket == INVALID_SOCKET) {
      if (server_socket->cont) {
	FLUID_LOG(FLUID_ERR, "Failed to accept connection");
      }
      server_socket->cont = 0;
      return;
    } else {
      int r;
      r = (*server_socket->func)(server_socket->data, client_socket, inet_ntoa(addr.sin_addr));
      if (r != 0) {
	fluid_socket_close(client_socket);
      }
    }
  }

  FLUID_LOG(FLUID_DBG, "Server closing");
}

fluid_server_socket_t*
new_fluid_server_socket(int port, fluid_server_func_t func, void* data)
{
  fluid_server_socket_t* server_socket;
  struct sockaddr_in addr;
  fluid_socket_t sock;

  if (func == NULL) {
    FLUID_LOG(FLUID_ERR, "Invalid callback function");
    return NULL;
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    FLUID_LOG(FLUID_ERR, "Failed to create server socket");
    return NULL;
  }

  FLUID_MEMSET((char *)&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (bind(sock, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
    FLUID_LOG(FLUID_ERR, "Failed to bind server socket");
    fluid_socket_close(sock);
    return NULL;
  }

  if (listen(sock, 10) == SOCKET_ERROR) {
    FLUID_LOG(FLUID_ERR, "Failed listen on server socket");
    fluid_socket_close(sock);
    return NULL;
  }

  server_socket = FLUID_NEW(fluid_server_socket_t);
  if (server_socket == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    fluid_socket_close(sock);
    return NULL;
  }

  server_socket->socket = sock;
  server_socket->func = func;
  server_socket->data = data;
  server_socket->cont = 1;

  server_socket->thread = new_fluid_thread(fluid_server_socket_run, server_socket, 0);
  if (server_socket->thread == NULL) {
    FLUID_FREE(server_socket);
    fluid_socket_close(sock);
    return NULL;
  }

  return server_socket;
}

int delete_fluid_server_socket(fluid_server_socket_t* server_socket)
{
  server_socket->cont = 0;
  if (server_socket->socket != INVALID_SOCKET) {
    fluid_socket_close(server_socket->socket);
  }
  if (server_socket->thread) {
    delete_fluid_thread(server_socket->thread);
  }
  FLUID_FREE(server_socket);
  return FLUID_OK;
}

int fluid_server_socket_join(fluid_server_socket_t* server_socket)
{
  return fluid_thread_join(server_socket->thread);
}

#endif
