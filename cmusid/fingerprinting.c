#include <assert.h>
#include <chromaprint.h>
#include <FLAC/stream_decoder.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "fingerprinting.h"

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

#define MAX_DURATION 120
#define NUM_THREADS 4

typedef struct arg
{
  FileNode_t *arr;
  int threadNo;
  int totalNumFiles;
} arg_t;

long long int streamRemaining = MAX_DURATION * SAMPLE_RATE;

FLAC__StreamDecoderWriteStatus
processFrame (const FLAC__StreamDecoder * decoder, const FLAC__Frame * frame,
	      const FLAC__int32 * const buffer[], void *context)
{
  int channels = frame->header.channels;

  streamRemaining -= frame->header.blocksize;

  // Endianness?
  if (!chromaprint_feed
      (context, buffer, frame->header.sample_rate * channels))
    {
      fprintf (stderr, "Error feeding Chromaprint from buffer\n");
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
  if (streamRemaining < 0)
    {
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void
processError (const FLAC__StreamDecoder * decoder,
	      FLAC__StreamDecoderErrorStatus status, void *context)
{
  printf ("There was an error in FLAC decoding: %d\n", status);
}

int
fingerprintFile (char *filePath, char **fingerprint)
{
  FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new ();
  ChromaprintContext *ctx;
  ctx = chromaprint_new (CHROMAPRINT_ALGORITHM_DEFAULT);

  if (!chromaprint_start (ctx, SAMPLE_RATE, NUM_CHANNELS))
    {
      fprintf (stderr, "Error initializing Chromaprint context\n");
      exit (2);
    }

  if (FLAC__stream_decoder_init_file
      (decoder, filePath, processFrame, NULL, processError, ctx) != 0)
    {
      fprintf (stderr, "Error initializing libFlac decoder. State: %d\n",
	       FLAC__stream_decoder_get_state (decoder));
      exit (2);
    }
  if (!FLAC__stream_decoder_process_until_end_of_metadata (decoder))
    {
      fprintf (stderr, "Error processing metadata with libFlac. State: %d\n",
	       FLAC__stream_decoder_get_state (decoder));
      exit (2);
    }

  if (!FLAC__stream_decoder_process_until_end_of_stream (decoder))
    {
      FLAC__StreamDecoderState state =
	FLAC__stream_decoder_get_state (decoder);
      if (!(state == FLAC__STREAM_DECODER_END_OF_STREAM ||
            state == FLAC__STREAM_DECODER_ABORTED))
	{
    fprintf(stderr, "Error doing something: %d\n", state);
	}
    }

  if (!chromaprint_finish (ctx))
    {
      fprintf (stderr, "Error finishing Chromaprint feed\n");
      exit (2);
    }
  assert (FLAC__stream_decoder_finish (decoder));
  if (!chromaprint_get_fingerprint (ctx, fingerprint))
    {
      fprintf (stderr, "Error retrieving fingerprint from Chromaprint\n");
      exit (2);
    }
  chromaprint_free (ctx);
  return -1;
}

/**
 * A pthread compatible function which expects an arg_t.
 */
void *
fingerprintThread (void *voidArgs)
{
  arg_t *args = (arg_t *) voidArgs;
  for (int i = args->threadNo; i < args->totalNumFiles; i += NUM_THREADS)
    {
      FileNode_t *cur = &(args->arr)[i];
      cur->duration = fingerprintFile (cur->filename, &(cur->fingerprint));
    }
  return NULL;
}

void
fingerprintFilesInParallel (FileNode_t * files, int numFiles)
{
  pthread_t threads[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
    {
      arg_t *arg = (arg_t *) malloc (sizeof (arg_t));
      arg->arr = files;
      arg->totalNumFiles = numFiles;
      arg->threadNo = i;
      if (pthread_create (&(threads[i]), NULL, fingerprintThread, arg))
	{
	  perror ("Error creating thread");
	}
    }
  for (int i = 0; i < NUM_THREADS; i++)
    {
      if (pthread_join (threads[i], 0))
	{
	  perror ("Error joining with thread");
	  exit (2);
	}
    }
}

