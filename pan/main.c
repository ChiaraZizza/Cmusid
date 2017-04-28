#define _GNU_SOURCE

#include <chromaprint.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

typedef struct FileNode {
  FILE file;
  char* filename;
  char* fingerprint;
  int duration;
} FileNode_t;

typedef struct arg {
  FileNode_t* arr;
  int threadNo;
  int totalNumFiles;
} arg_t;

#define NO_THREADS 4
const int sample_rate = 44100;
const int num_channels = 2;
const int FILE_BLOCK_SIZE = 128;
const char* ACOUSTID_API_URL = "http://api.acoustid.org/v2/lookup";
const char* APPLICATION_ID = "jif76R78Wd";

void fingerprintFile(FILE *file, char** fingerprint, int* duration) {
  ChromaprintContext *ctx;
  ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);

  chromaprint_start(ctx,sample_rate,num_channels);

  int16_t *buffer = (int16_t*) malloc((FILE_BLOCK_SIZE * sizeof(int16_t)));
  while (fread(buffer,FILE_BLOCK_SIZE, 1, file)) {
    if(!chromaprint_feed(ctx,buffer,FILE_BLOCK_SIZE)) {
      fprintf(stderr, "Error feeding Chromaprint from buffer");
      exit(2);
    }
  }

  if (!chromaprint_finish(ctx)) {
    fprintf(stderr, "Error feeding Chromaprint from buffer");
    exit(2);
  }
  free(buffer);

  chromaprint_get_fingerprint(ctx, fingerprint);
  *duration = chromaprint_get_item_duration(ctx);
  chromaprint_free(ctx);
}

void fetchMetadata(char* fingerprint, int duration) {
  printf("Fetching Metadata...\n | Duration: %d\n | Fingerprint: %s\n",duration, fingerprint);
  CURL *curl = curl_easy_init();
  if(curl) {
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, ACOUSTID_API_URL);
    char *body;
    asprintf(&body,"client=%s&duration=%d&fingerprint=%s", APPLICATION_ID, duration, fingerprint);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
          curl_easy_strerror(res));
    curl_easy_cleanup(curl);
    free(body);
  } else {
    fprintf(stderr, "Error Initializing cURL");
  }
}

int consumeDirectory(char* directory, FileNode** flacFiles) {
  DIR *d = opendir(directory);
  int numFiles = 0;
  struct dirent *dir;
  while ((dir = readdir(d)) != NULL) {
    if (strstr(dir->d_name, ".flac") != NULL) {
      numFiles++;
    }
  }
  closedir(d);

  int i = 0;
  d = opendir(directory);
  flacFile = (FileNode_t*) malloc(sizeof(FileNode_t)*numFiles);
  while ((dir = readdir(d)) != NULL) {
    ret[i]->file = fopen(dir->d_name, "r");
    ret[i]->filename = dir->d_name;
    i++;
  }

  return numFiles;
}

void* threadFunction (void* voidArgs) {
  arg_t* args = (arg_t*) voidArgs;
  for (int i = args->threadNo; i < args->totalNumFiles; i+= NO_THREADS) {
    FileNode_t* cur = args->arr[i];
    fingerprintFile(cur->file, cur->fingerprint, cur->duration);
  }
}

int main (int argc, char *argv[]) {
  FileNode_t* flacFiles;
  int totalNumFiles = consumeDirectory(argv[1], FileNode_t* flacFiles);
  char *fingerprint;
  int duration;

  pthread_t threads[NO_THREADS];
  for (int i = 0; i < NO_THREADS; i++) {
    arg_t arg = (arg_t*) malloc(sizeof(arg_t));
    arg->threadNo = i;
    if (pthread_create(&(threads[i]), NULL, threadFunction, arg)) {
      perror("Error creating thread");
    }
  }
  for(int i = 0; i < NO_THREADS; i++) {
    if (pthread_join(threads[i],0)) {
      perror("Error joining with thread");
      exit(2);
    }
  }
  for(int i = 0; i < totalNumFiles; i++) {
   // print files or something lol  
  }
  return 0;
}
