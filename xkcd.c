/*
 * Compile with gcc -O3 -std=c99 *.c -lcurl -lpthread -DSHIFT_CHAR -o xkcd-skein
*/

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#include "skein.h"

/* The number of bits to use for the Skein 1024 hash */
#define SKEIN_BITS      1024
/* The length of the generated hash in bytes */
#define SKEIN_HASH_SIZE (SKEIN_BITS / 8)
/* The length of the hex string hash */
#define HASH_SIZE       (SKEIN_HASH_SIZE * 2)
/* The minimum length of our generated strings */
#define STR_MIN         12
/* The maximum length of our generated strings */
#define STR_MAX         20
/* The number of threads to run */
#define NUM_THREADS     8
/* The name of our institution */
#define INSTITUTION     "york.ac.uk"
/* The allowed characters in the generated strings */
#define ALLOWED         "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"
/* The length of allowed characters */
#define ALLOWED_LEN     (sizeof(ALLOWED) - 2)
/* The granularity of the counting (power of 2) */
#define COUNT_GRANULARITY (1 << 16)
/* The timer update */
#define TIMER_INTERVAL  30

size_t best = 424;
size_t count = 0;
size_t elapsed = 0;
pthread_mutex_t best_mutex = PTHREAD_MUTEX_INITIALIZER;

FILE *logfile;

/* The xkcd-provided hash */
const uint8_t xkcdhash[] =
  {0x5b, 0x4d, 0xa9, 0x5f, 0x5f, 0xa0, 0x82, 0x80, 0xfc, 0x98, 0x79, 0xdf,
   0x44, 0xf4, 0x18, 0xc8, 0xf9, 0xf1, 0x2b, 0xa4, 0x24, 0xb7, 0x75, 0x7d,
   0xe0, 0x2b, 0xbd, 0xfb, 0xae, 0x0d, 0x4c, 0x4f, 0xdf, 0x93, 0x17, 0xc8,
   0x0c, 0xc5, 0xfe, 0x04, 0xc6, 0x42, 0x90, 0x73, 0x46, 0x6c, 0xf2, 0x97,
   0x06, 0xb8, 0xc2, 0x59, 0x99, 0xdd, 0xd2, 0xf6, 0x54, 0x0d, 0x44, 0x75,
   0xcc, 0x97, 0x7b, 0x87, 0xf4, 0x75, 0x7b, 0xe0, 0x23, 0xf1, 0x9b, 0x8f,
   0x40, 0x35, 0xd7, 0x72, 0x28, 0x86, 0xb7, 0x88, 0x69, 0x82, 0x6d, 0xe9,
   0x16, 0xa7, 0x9c, 0xf9, 0xc9, 0x4c, 0xc7, 0x9c, 0xd4, 0x34, 0x7d, 0x24,
   0xb5, 0x67, 0xaa, 0x3e, 0x23, 0x90, 0xa5, 0x73, 0xa3, 0x73, 0xa4, 0x8a,
   0x5e, 0x67, 0x66, 0x40, 0xc7, 0x9c, 0xc7, 0x01, 0x97, 0xe1, 0xc5, 0xe7,
   0xf9, 0x02, 0xfb, 0x53, 0xca, 0x18, 0x58, 0xb6};

/**
 * Generate a random number in a given range (inclusive)
 */
static inline size_t randrange(size_t min, size_t max)
{
	return (rand() % (1 + max - min)) + min;
}

/**
 * Generate a random string of the given length
 */
void randbytes(char *out, size_t len)
{
	for(size_t i = 0; i < len; i++)
		out[i] = ALLOWED[randrange(0, ALLOWED_LEN)];

	out[len] = '\0';
}

/**
 * Shift char
 */
static inline void shiftchar(char *out, size_t len)
{
	size_t i = randrange(0, len - 1);
	out[i] = ALLOWED[randrange(0, ALLOWED_LEN)];
}

/**
 * Calculate the difference between the two strings
 */
static inline size_t distance(const uint8_t* str1, const uint8_t* str2)
{
	size_t score = 0;

#if SKEIN_HASH_SIZE == 128
    uint32_t *a = (uint32_t *)str1;
	uint32_t *b = (uint32_t *)str2;

	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
	score += __builtin_popcountl(*a++ ^ *b++);
#else
	for(size_t i = 0; i < SKEIN_HASH_SIZE; i++)
		score += __builtin_popcount(str1[i] ^ str2[i]);
#endif

	return score;
}

/**
 * Calculate the Skein 1024 hash of a string
 */
static inline void hash(const char* str, size_t len, size_t bytes,
	  uint8_t skeinhash[SKEIN_HASH_SIZE])
{
	Skein1024_Ctxt_t ctx;
 
	Skein1024_Init(&ctx, bytes);
	Skein1024_Update(&ctx, (uint8_t *)str, len);
	Skein1024_Final(&ctx, skeinhash);
}

/**
 * Discard the curl output
 */
size_t curl_devnull(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	return size * nmemb;
}

/**
 * Submit a string to xkcd
 */
void submit_to_xkcd(const char* str)
{
	CURL *curl;
	CURLcode res;
	char postdata[HASH_SIZE + 11] = "hashable=";

	strcat(postdata, (char*)str);

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://almamater.xkcd.com/?edu=" INSTITUTION);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_devnull);

		res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
	}
}

/**
 * Log a new best score and submit it to xkcd
 */
static inline void new_best_score(int threadid, size_t len, char* str, size_t mybest, size_t score)
{
  char str1[STR_MAX+1];
  strncpy(str1, str, len);
  str1[len] = '\0';

  printf("[%zu] New best string(%zu): %s (old: %zu) (new: %zu)\n", threadid, len, str1, mybest, score);

  fprintf(logfile, "[%zu] New best string(%zu): %s (old: %zu) (new: %zu)\n", threadid, len, str1, mybest, score);
  fflush(logfile);

  submit_to_xkcd(str);
}

/**
 * Start up a hash cracking thread
 */
void* threaded_solver(void* data)
{
	size_t threadid = (size_t)data;
	uint8_t skeinhash[SKEIN_HASH_SIZE];
	char str[STR_MAX + 1];
	size_t mybest = best;
	size_t mycount = 0;

	printf("Thread %zu started.\n", threadid);

#ifdef SHIFT_CHAR
	size_t len = STR_MAX;
	randbytes(str, len);
#endif

	while(true) {
		size_t len = randrange(STR_MIN, STR_MAX);

#ifdef SHIFT_CHAR
		shiftchar(str, len);
#else
		randbytes(str, len);
#endif

		hash(str, len, SKEIN_BITS, skeinhash);
		size_t score = distance(skeinhash, xkcdhash);
		mycount++;

		if (!(mycount & (COUNT_GRANULARITY - 1)))
          __sync_fetch_and_add(&count, COUNT_GRANULARITY);

#if defined(__APPLE__) && !defined(__GNUC__)
		if (mybest > score) {
          mybest = score;
          if (best > mybest) {
            pthread_mutex_lock(&best_mutex);
            if (best > mybest) {
              best = mybest;
              new_best_score(threadid, len, str, mybest, score);
            }
            pthread_mutex_unlock(&best_mutex);
          }
		}
#else
		while (mybest > score) {
          if (__sync_bool_compare_and_swap(&best, mybest, score)) {
            new_best_score(threadid, len, str, mybest, score);
            mybest = score;
          } else {
            mybest = best;
          }
		}
#endif
	}
}
 
static inline void set_timer()
{
	struct itimerval tout_val;

	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = TIMER_INTERVAL;
	tout_val.it_value.tv_usec = 0;

	setitimer(ITIMER_REAL, &tout_val,0);
}
 
void alarm_benchmark(int i)
{
	signal(SIGALRM, alarm_benchmark);
	elapsed += TIMER_INTERVAL;

	printf("Count: %zu, Elapsed: %zu, Speed: %g hash/sec\n",
		count, elapsed,
		(double)count / (double)elapsed);

	fprintf(logfile, "Count: %zu, Elapsed: %zu, Speed: %g hash/sec\n",
		count, elapsed,
		(double)count / (double)elapsed);

	set_timer();
}

int main(void)
{
	srand((unsigned int) time(NULL));
	curl_global_init(CURL_GLOBAL_ALL);

	pthread_t threads[NUM_THREADS];

	// Tell of the mode.
#ifdef SHIFT_CHAR
	printf("Shifting characters since 2013.\n");
#else
	printf("Being random since 2013.\n");
#endif

	// Log file.
	logfile = fopen("output.log", "a");

	// Start the benchmarking.
	set_timer();
	signal(SIGALRM, alarm_benchmark);

	// Fire off the threads.
	if (NUM_THREADS == 1) {
      threaded_solver(NULL);
	} else {
      for(size_t i = 0; i < NUM_THREADS; i++)
        pthread_create(&(threads[i]), NULL, threaded_solver, (void*) i);

      for(size_t i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
	}

	fclose(logfile);

	curl_global_cleanup();
}
