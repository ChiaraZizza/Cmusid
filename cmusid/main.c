#define _GNU_SOURCE

#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fingerprinting.h"
#include "metadata.h"

#define NO_THREADS 4

const char *CLI_OPTION_LETTERS = "ifrv";

/**
 * \brief Steps through the directory at the given path string and returns an array of FileNodes
 *    which represent its contents
 */
FileNode_t *
consumeDirectory (char *directory, int *filecount)
{
  // First we step through the directory to count the number of flac files
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

  // Now we'll step through the directory and parse the flac files into our ledger array
  int i = 0;
  d = opendir (directory);
  FileNode_t *flacFiles = malloc (sizeof (FileNode_t) * (*filecount));
  while ((dir = readdir (d)) != NULL)
    {
      if (strstr (dir->d_name, ".flac") != NULL)
	{
	  FileNode_t *node = &(flacFiles[i]);
	  asprintf (&node->filename, "%s/%s", directory, dir->d_name);
	  i++;
	}
    }
  closedir (d);
  assert (*filecount == i);

  return flacFiles;
}

/**
 * Prints program usage instructions
 */
void
printHelp ()
{
  printf ("\
Usage: cmusid [OPTION]... [DIRECTORY]\n\
Identify and organize audio files within DIRECTORY\n\n\
  -i, --interactive\t prompt user for track names, artists, and albums instead of querying AcoustID database\n\
  -f, --fingerprint\t use Chromaprint library to generate unique fingerprints for each file\n\
  -r, --recursive\t identify files in subfolders of input directory\n\
  -v, --verbose\t\t use verbose logging (you'll want to pipe to `less`)\n");
}


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
