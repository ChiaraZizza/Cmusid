#ifndef __FINGERPRINTING_H__
#define __FINGERPRINTING_H__

typedef struct FileNode
{
  char *filename;
  char *fingerprint;
  int duration;
} FileNode_t;

typedef struct arg
{
  FileNode_t *arr;
  int threadNo;
  int totalNumFiles;
} arg_t;

extern int fingerprintFile (char *filePath, char **fingerprint);

extern void fingerprintFilesInParallel (FileNode_t * files, int numFiles);
#endif // __FINGERPRINTING_H__
