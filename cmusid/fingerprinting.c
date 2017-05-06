#include <chromaprint.h>
#include <stdlib.h>
#include <stdio.h>

#define FILE_BLOCK_SIZE 128
#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

int fingerprintFile(char* filePath, char** fingerprint) {
  FILE *file = fopen(filePath, "r");
  ChromaprintContext *ctx;
  ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);

  chromaprint_start(ctx,SAMPLE_RATE,NUM_CHANNELS);

 int16_t *buffer = (int16_t*) malloc((FILE_BLOCK_SIZE * sizeof(int16_t)));
  int samples;
  int read;
  while ((read = fread(buffer,FILE_BLOCK_SIZE * sizeof(int16_t), 1, file))) {
    if(!chromaprint_feed(ctx,buffer,FILE_BLOCK_SIZE)) {
      fprintf(stderr, "Error feeding Chromaprint from buffer\n");
      exit(2);
    }
    samples += chromaprint_get_item_duration(ctx);
  }

  if (!chromaprint_finish(ctx)) {
    fprintf(stderr, "Error finishing Chromaprint feed\n");
    exit(2);
  }
  fclose(file);
  free(buffer);
  if (!chromaprint_get_fingerprint(ctx, fingerprint)) {
    fprintf(stderr, "Error retrieving fingerprint from Chromaprint\n");
    exit(2);
  }
  chromaprint_free(ctx);
  return (samples * 1.0) / SAMPLE_RATE;
}


