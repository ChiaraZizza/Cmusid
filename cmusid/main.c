#define _GNU_SOURCE

#include <assert.h>
#include <curl/curl.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <FLAC/metadata.h>
#include <unistd.h>

#include "fingerprinting.h"

typedef struct arg
{
  FileNode_t *arr;
  int threadNo;
  int totalNumFiles;
} arg_t;

typedef FLAC__StreamMetadata data;

#define NO_THREADS 4

const char *CLI_OPTION_LETTERS = "ifrv";
const char *ACOUSTID_API_URL = "http://api.acoustid.org/v2/lookup";
const char *APPLICATION_ID = "jif76R78Wd";

void
fetchMetadata (char *fingerprint, int duration)
{
  printf ("Fetching Metadata...\n | Duration: %d\n | Fingerprint: %s\n",
	  duration, fingerprint);
  CURL *curl = curl_easy_init ();
  if (curl)
    {
      CURLcode res;
      curl_easy_setopt (curl, CURLOPT_URL, ACOUSTID_API_URL);
      char *body;
      asprintf (&body, "client=%s&duration=%d&fingerprint=%s", APPLICATION_ID,
		duration, fingerprint);
      curl_easy_setopt (curl, CURLOPT_POSTFIELDS, body);

      res = curl_easy_perform (curl);
      if (res != CURLE_OK)
	fprintf (stderr, "curl_easy_perform() failed: %s\n",
		 curl_easy_strerror (res));
      curl_easy_cleanup (curl);
      free (body);
    }
  else
    {
      fprintf (stderr, "Error Initializing cURL");
    }
}

FileNode_t *
consumeDirectory (char *directory, int *filecount)
{
  DIR *d = opendir (directory);
  struct dirent *dir;
  *filecount = 0;
  while ((dir = readdir (d)) != NULL)
    {
      if (strstr (dir->d_name, ".flac") != NULL)
	{
	  (*filecount)++;
	}
    }
  closedir (d);

  int i = 0;
  d = opendir (directory);
  FileNode_t *flacFiles = malloc (sizeof (FileNode_t) * (*filecount));
  while ((dir = readdir (d)) != NULL)
    {
      if (strstr (dir->d_name, ".flac") != NULL)
	{
	  FileNode_t *node = &(flacFiles[i]);
	  char *pathname;
	  asprintf (&pathname, "%s/%s", directory, dir->d_name);
	  node->filename = pathname;
	  i++;
	}
    }
  assert (*filecount == i);

  return flacFiles;
}

void *
threadFunction (void *voidArgs)
{
  arg_t *args = (arg_t *) voidArgs;
  for (int i = args->threadNo; i < args->totalNumFiles; i += NO_THREADS)
    {
      FileNode_t *cur = &(args->arr)[i];
      char *fingerprint;
      cur->duration = fingerprintFile (cur->filename, &fingerprint);
      cur->fingerprint = fingerprint;
    }
  return NULL;
}


void
printHelp ()
{
  printf ("\
Usage: cmusid [OPTION]... [DIRECTORY]\n\
Identify and organize audio files within DIRECTORY\n\n\
  -i, --interactive\t prompt user for track names, artists, and albums instead of querying AcoustID database\n\
  -f, --fingerprint\t use Chromaprint library to generate unique fingerprints for each file\n\
  -r, --recursive\t identify files in subfolders of input directory\n\
  -v, --verbose\t\t use verbose logging\n");
}

void
fingerprintFilesInParallel (FileNode_t * files, int numFiles)
{
  pthread_t threads[NO_THREADS];
  for (int i = 0; i < NO_THREADS; i++)
    {
      arg_t *arg = (arg_t *) malloc (sizeof (arg_t));
      arg->arr = files;
      arg->totalNumFiles = numFiles;
      arg->threadNo = i;
      if (pthread_create (&(threads[i]), NULL, threadFunction, arg))
	{
	  perror ("Error creating thread");
	}
    }
  for (int i = 0; i < NO_THREADS; i++)
    {
      if (pthread_join (threads[i], 0))
	{
	  perror ("Error joining with thread");
	  exit (2);
	}
    }
}


// RUN BEFORE EVERY CALL
//!git checkout input/cool.flac
int
main (int argc, char *argv[])
{
  bool shouldBeInteractive, shouldFingerprint, shouldBeVerbose = false;
  int opt;
  while ((opt = getopt (argc, argv, CLI_OPTION_LETTERS)) != -1)
    {
      switch (opt)
	{
	case 'i':
	  shouldBeInteractive = true;
	  break;
	case 'f':
	  shouldFingerprint = true;
	  break;
	case 'v':
	  shouldBeVerbose = true;
	  break;
	}
    }
  if (optind >= argc)
    {				// No non-option arguments provided
      printHelp ();
      exit (EXIT_FAILURE);
    }
  int numFiles;
  FileNode_t *flacFiles = consumeDirectory (argv[optind], &numFiles);

  if (shouldFingerprint)
    {
      fingerprintFilesInParallel (flacFiles, numFiles);
    }

  if (shouldBeVerbose)
    {
      for (int i = 0; i < numFiles; i++)
	{
	  FileNode_t *node = &flacFiles[i];
	  printf ("File: %s\n | Duration: %d\n | Fingerprint: %s\n\n",
		  node->filename, node->duration, node->fingerprint);
	}
    }


  return 0;
}
