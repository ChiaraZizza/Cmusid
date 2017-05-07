#ifndef __FINGERPRINTING_H__
#define __FINGERPRINTING_H__

typedef struct FileNode {
  char * filename;
  char * fingerprint;
  int duration;
} FileNode_t;

extern int fingerprintFile(char* filePath, char** fingerprint);

#endif // __FINGERPRINTING_H__
