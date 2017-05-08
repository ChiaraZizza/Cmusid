#include <assert.h>
#include <chromaprint.h>
#include <FLAC/stream_decoder.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

FLAC__StreamDecoderWriteStatus processFrame(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void* context) {
  int channels = frame->header.channels;

  if(!chromaprint_feed(context,buffer,SAMPLE_RATE*channels)) {
    fprintf(stderr, "Error feeding Chromaprint from buffer\n");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void processError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *context) {
  printf("There was an error in FLAC decoding: %d\n", status);
}

int fingerprintFile(char* filePath, char** fingerprint) {
  FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
  ChromaprintContext *ctx;
  ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);

  if (!chromaprint_start(ctx,SAMPLE_RATE,NUM_CHANNELS)) {
    fprintf(stderr, "Error initializing Chromaprint context\n");
    exit(2);
  }

  if (FLAC__stream_decoder_init_file(decoder, filePath, processFrame, NULL, processError, ctx) != 0) {
    fprintf(stderr, "Error initializing libFlac decoder. State: %d\n", FLAC__stream_decoder_get_state(decoder));
    exit(2);
  }
  if (!FLAC__stream_decoder_process_until_end_of_metadata(decoder)) {
    fprintf(stderr, "Error processing metadata with libFlac. State: %d\n", FLAC__stream_decoder_get_state(decoder));
    exit(2);
  }

  if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)) {
    if (!(FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_END_OF_STREAM)) {
      exit(2);
    }
  }

  if (!chromaprint_finish(ctx)) {
    fprintf(stderr, "Error finishing Chromaprint feed\n");
    exit(2);
  }
  assert(FLAC__stream_decoder_finish(decoder));
  if (!chromaprint_get_fingerprint(ctx, fingerprint)) {
    fprintf(stderr, "Error retrieving fingerprint from Chromaprint\n");
    exit(2);
  }
  chromaprint_free(ctx);
  return -1;
}


