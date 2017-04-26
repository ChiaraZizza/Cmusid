#define _GNU_SOURCE

#include <chromaprint.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  CURL *curl = curl_easy_init();
  if(curl) {
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, ACOUSTID_API_URL);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    curl_easy_cleanup(curl);
  } else {
    fprintf(stderr, "Error Initializing cURL");
  }
 
}


int main () {
  FILE *file;
  file = fopen("input/truth.flac", "r");
  char *fingerprint;
  int duration;
  fingerprintFile(file, &fingerprint, &duration);
  fetchMetadata(fingerprint, duration);
  return 0;
}
