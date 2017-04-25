#include <stdio.h>
#include <stdlib.h>
#include <chromaprint.h>

const int sample_rate = 44100;
const int num_channels = 2;
const int FILE_BLOCK_SIZE = 2;

void fingerprintFile(FILE *file) {
  ChromaprintContext *ctx;
  ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);

  chromaprint_start(ctx,sample_rate,num_channels);
  int16_t *buffer = (int16_t*) malloc((FILE_BLOCK_SIZE * sizeof(int16_t)));
  while (fread(buffer,FILE_BLOCK_SIZE, 1, file)) {
    chromaprint_feed(ctx,buffer,FILE_BLOCK_SIZE);
  }

  chromaprint_finish(ctx);

  char *fingerprint;
  chromaprint_get_fingerprint(ctx, &fingerprint);
  printf("%s\n", fingerprint);

  free(buffer);
  chromaprint_free(ctx);
}

int main () {
  FILE *file;
  file = fopen("input/truth.flac", "r");
  fingerprintFile(file);
  fclose(file);
  return 0;
}
