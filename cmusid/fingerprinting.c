#include "fingerprinting.h"

#include <assert.h>
#include <chromaprint.h>
#include <FLAC/stream_decoder.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

// Chromaprint's sample app fpcalc only samples the first two minutes.
#define MAX_DURATION 120
#define NUM_THREADS 4

typedef struct arg
{
  FileNode_t *arr;
  int threadNo;
  int totalNumFiles;
} arg_t;

/// We will only fingerprint the first MAX_DURATION seconds
long long int streamRemaining = MAX_DURATION * SAMPLE_RATE;

/// But we still need to find the entire file's duration
long long int streamLength = 0;

/**
 * A data-read function for libFLAC which feeds into the Chromaprint context given
 */
FLAC__StreamDecoderWriteStatus
processFrame (const FLAC__StreamDecoder * decoder, const FLAC__Frame * frame,
	      const FLAC__int32 * const buffer[], void *context)
{
  int channels = frame->header.channels;
  int samples = frame->header.blocksize;

  streamRemaining -= samples;
  streamLength += samples;

  int16_t shortBuffer[samples * channels];

  for (int channel = 0; channel < channels; channel++) {
    for (int sample = 0; sample < samples; sample++) {
      shortBuffer[(sample * channels) + channel] = (int16_t) buffer[channel][sample];
    }
  }
  // Endianness?
  if ((streamRemaining > 0) && !chromaprint_feed
      (context, shortBuffer, samples * channels))
    {
      fprintf (stderr, "Error feeding Chromaprint from libFLAC buffer\n");
      exit (2);
    }
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

/**
 * A data-read error handler for libFLAC
 */
void
processError (const FLAC__StreamDecoder * decoder,
	      FLAC__StreamDecoderErrorStatus status, void *context)
{
  fprintf (stderr, "There was an error in FLAC decoding: %d\n", status);
  exit (2);
}

/**
 * Uses Chromaprint and libFLAC to fingerprint the file at filePath
 * Returns the duration of the track
 */
int
fingerprintFile (char *filePath, char **fingerprint)
{
  FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new ();
  ChromaprintContext *ctx = chromaprint_new (CHROMAPRINT_ALGORITHM_DEFAULT);

  if (!chromaprint_start (ctx, SAMPLE_RATE, NUM_CHANNELS))
    {
      fprintf (stderr, "Error initializing Chromaprint context\n");
      exit (2);
    }

  if (FLAC__stream_decoder_init_file
      (decoder, filePath, processFrame, NULL, processError, ctx) != 0)
    {
      fprintf (stderr, "Error initializing libFlac decoder. State: %s\n",
	       FLAC__stream_decoder_get_resolved_state_string (decoder));
      exit (2);
    }
  if (!FLAC__stream_decoder_process_until_end_of_metadata (decoder))
    {
      fprintf (stderr, "Error processing metadata with libFlac. State: %s\n",
	       FLAC__stream_decoder_get_resolved_state_string (decoder));
      exit (2);
    }

  // This is where the magic happens!
  if (!FLAC__stream_decoder_process_until_end_of_stream (decoder))
    {
      FLAC__StreamDecoderState state =
	FLAC__stream_decoder_get_state (decoder);
      // It is normal if we reach the end of stream or if we aborted
      if (!(state == FLAC__STREAM_DECODER_END_OF_STREAM))
	{
	  fprintf (stderr, "Error decoding FLAC stream: %d\n", state);
	}
    }

  if (!chromaprint_finish (ctx))
    {
      fprintf (stderr, "Error finishing Chromaprint feed\n");
      exit (2);
    }
  if (!(FLAC__stream_decoder_finish (decoder)))
    {
      fprintf (stderr, "Error finishing FLAC decoding\n");
      exit (2);
    }
  if (!chromaprint_get_fingerprint (ctx, fingerprint))
    {
      fprintf (stderr, "Error retrieving fingerprint from Chromaprint\n");
      exit (2);
    }
  chromaprint_free (ctx);
  FLAC__stream_decoder_delete(decoder);
  return streamLength / 44100;
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
