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
const char* xkcdhashstr = "5b4da95f5fa08280fc9879df44f418c8f9f12ba424b7757de02bbdfbae0d4c4fdf9317c80cc5fe04c6429073466cf29706b8c25999ddd2f6540d4475cc977b87f4757be023f19b8f4035d7722886b78869826de916a79cf9c94cc79cd4347d24b567aa3e2390a573a373a48a5e676640c79cc70197e1c5e7f902fb53ca1858b6";
const uint8_t* xkcdhash =
	"\x5b\x4d\xa9\x5f\x5f\xa0\x82\x80\xfc\x98\x79\xdf\x44\xf4\x18\xc8"
	"\xf9\xf1\x2b\xa4\x24\xb7\x75\x7d\xe0\x2b\xbd\xfb\xae\x0d\x4c\x4f"
	"\xdf\x93\x17\xc8\x0c\xc5\xfe\x04\xc6\x42\x90\x73\x46\x6c\xf2\x97"
	"\x06\xb8\xc2\x59\x99\xdd\xd2\xf6\x54\x0d\x44\x75\xcc\x97\x7b\x87"
	"\xf4\x75\x7b\xe0\x23\xf1\x9b\x8f\x40\x35\xd7\x72\x28\x86\xb7\x88"
	"\x69\x82\x6d\xe9\x16\xa7\x9c\xf9\xc9\x4c\xc7\x9c\xd4\x34\x7d\x24"
	"\xb5\x67\xaa\x3e\x23\x90\xa5\x73\xa3\x73\xa4\x8a\x5e\x67\x66\x40"
	"\xc7\x9c\xc7\x01\x97\xe1\xc5\xe7\xf9\x02\xfb\x53\xca\x18\x58\xb6";
 
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
#if SKEIN_HASH_SIZE == 128
	size_t score = 0;
       /* uint64_t *a = (uint64_t *)str1;*/
	/*uint64_t *b = (uint64_t *)str2;*/
 
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
	/*score += __builtin_popcountll(*a++ ^ *b++);*/
 
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
 
	return score;
#else
	return distance_old(str1, str2);
#endif
}
 
/**
 * Calculate the difference between the two strings
 */
static inline size_t distance_old(const uint8_t* str1, const uint8_t* str2)
{
	size_t score = 0;
	char bits = 0;
 
	for(size_t i = 0; i < SKEIN_HASH_SIZE; i++)
		score += __builtin_popcount(str1[i] ^ str2[i]);
 
	return score;
}
 
/**
 * Convert a string of bytes into a hexadecimal string
 */
void to_hex(const uint8_t* str, char *out, size_t len)
{
	char* p = out;
 
	for(size_t i = 0; i < len; i++)
		p += sprintf(p, "%.2x", str[i]);
 
	*p = '\0';
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
 
 
/* Discard the curl output */
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
 
void* threaded_solver(void* data)
{
	size_t threadid = (size_t)data;
	uint8_t skeinhash[SKEIN_HASH_SIZE];
	char str[STR_MAX + 1];
	size_t mybest = best;
	size_t mycount = 0;
 
	printf("Thread %zu started.\n", threadid);
	/*size_t len = randrange(STR_MIN, STR_MAX);*/
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
		/*assert(score == distance_old(skeinhash, xkcdhash));*/
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
					char str1[STR_MAX+1]; strncpy(str1, str, len); str1[len] = '\0';
					printf("[%zu] New best string(%zu): %s (old: %zu) (new: %zu)\n", threadid, len, str1, mybest, score);
					fprintf(logfile, "[%zu] New best string(%zu): %s (old: %zu) (new: %zu)\n", threadid, len, str1, mybest, score);
					fflush(logfile);
					submit_to_xkcd(str);
				}
				pthread_mutex_unlock(&best_mutex);
			}
		}
#else
		while (mybest > score) {
			if (__sync_bool_compare_and_swap(&best, mybest, score)) {
				char str1[STR_MAX+1]; strncpy(str1, str, len); str1[len] = '\0';
				printf("[%zu] New best string(%zu): %s (old: %zu) (new: %zu)\n", threadid, len, str1, mybest, score);
				fprintf(logfile, "[%zu] New best string(%zu): %s (old: %zu) (new: %zu)\n", threadid, len, str1, mybest, score);
				fflush(logfile);
				mybest = score;
				submit_to_xkcd(str);
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
