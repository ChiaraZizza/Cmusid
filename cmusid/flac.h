#ifndef __FLAC_H__
#define __FLAC_H__
#include "fingerprinting.h"

extern void interactivelyRenameFiles(FileNode_t *files, int fileCount);
extern void modifyFile(char* title, char* artist, char* album, char* old_filename);
#endif // __FLAC_H__
