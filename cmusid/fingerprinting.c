#include <chromaprint.h>
#include <FLAC/stream_decoder.h>
#include <stdlib.h>
#include <stdio.h>

#define FILE_BLOCK_SIZE 128
#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

void processFrame(ChromaprintContext *context) {
  int read;
    if(!chromaprint_feed(context,buffer,FILE_BLOCK_SIZE)) {
      fprintf(stderr, "Error feeding Chromaprint from buffer\n");
      exit(2);
    }
  }
}

int fingerprintFile(char* filePath, char** fingerprint) {
  FILE *file = fopen(filePath, "rb");
  FLAC__StreamDecoder decoder;
  ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);

  assert(chromaprint_start(ctx,SAMPLE_RATE,NUM_CHANNELS));

  assert(FLAC__stream_decoder_init_FILE(&decoder, file, callback, NULL, errorcallback, NULL));

  assert(FLAC__stream_decoder_process_until_end_of_stream(decoder));

  if (!chromaprint_finish(ctx)) {
    fprintf(stderr, "Error finishing Chromaprint feed\n");
    exit(2);
  }
  assert(FLAC__stream_decoder_finish(decoder));
  fclose(file);
  free(buffer);
  if (!chromaprint_get_fingerprint(ctx, fingerprint)) {
    fprintf(stderr, "Error retrieving fingerprint from Chromaprint\n");
    exit(2);
  }
  chromaprint_free(ctx);
  return (samples * 1.0) / SAMPLE_RATE;
}


