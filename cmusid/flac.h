#ifndef __FLAC_H__
#define __FLAC_H__
#include <FLAC/metadata.h>

void
renameVorbis(FLAC__Metadata_SimpleIterator* flac_iter, bool use, char* title);

extern void 
test(char* title, char* artist, char* album, char* old_filename);
#endif // __FLAC_H__
