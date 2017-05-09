#include "flac.h"

#include <assert.h>
#include <FLAC/metadata.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_METADATA_LENGTH 100

void renameVorbis (FLAC__Metadata_SimpleIterator * flac_iter, char *title,
        char *album, char *artist)
{
    int tlen = strlen (title);
    int allen = strlen (album);
    int arlen = strlen (artist);

    // iterate while there are blocks to read
    while (FLAC__metadata_simple_iterator_next (flac_iter))
    {
        // meta points at the current flac file
        FLAC__StreamMetadata *meta =
            FLAC__metadata_simple_iterator_get_block (flac_iter);

        // temp is a new metadata object to populate
        FLAC__StreamMetadata *temp =
            FLAC__metadata_object_new (FLAC__METADATA_TYPE_VORBIS_COMMENT);

        if (meta->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
        {
            // empty vorbis comment entry to populate
            FLAC__StreamMetadata_VorbisComment_Entry *new_entry =
                (FLAC__StreamMetadata_VorbisComment_Entry *)
                malloc (sizeof (FLAC__StreamMetadata_VorbisComment_Entry) * 3);

            for (int i = 0; i < meta->data.vorbis_comment.num_comments; i++)
            {

                // Copy the function arguments to FLAC__byte* to insert into empty vorbis comment entry
                FLAC__byte *title_bytes =
                    malloc (sizeof (FLAC__byte) * tlen + 1);
                memcpy (title_bytes, title, tlen);
                title_bytes[tlen] = '\0';

                FLAC__byte *album_bytes =
                    malloc (sizeof (FLAC__byte) * allen + 1);
                memcpy (album_bytes, album, allen);
                album_bytes[allen] = '\0';

                FLAC__byte *artist_bytes =
                    malloc (sizeof (FLAC__byte) * arlen + 1);
                memcpy (artist_bytes, artist, arlen);
                artist_bytes[arlen] = '\0';

                // Populate the new vorbis comment entry
                new_entry[0].length = tlen;
                new_entry[0].entry = title_bytes;

                new_entry[1].length = allen;
                new_entry[1].entry = album_bytes;

                new_entry[2].length = arlen;
                new_entry[2].entry = artist_bytes;

                temp->data.vorbis_comment.comments = new_entry;
                temp->data.vorbis_comment.num_comments = 3;

                FLAC__metadata_object_vorbiscomment_replace_comment (temp, *new_entry, false, true);
                assert (FLAC__metadata_simple_iterator_set_block
                        (flac_iter, temp, true));
            }
        }
    }
}

void renameFile(char* old_filename, char* title, char* album, char* artist) {
    // generates new filename string
    int tlen, alen, arlen;
    tlen = strlen(title);
    alen = strlen(album);
    arlen = strlen(artist);
    int new_len = tlen+alen+arlen + 16;
    char* new = malloc (sizeof(char)*new_len);
    snprintf(new, new_len, "input/%s--%s--%s.flac", artist, album, title);

    // rename file
    //ret = rename(old_filename, new);
    char* buffer = (char*) malloc(sizeof(char) * new_len * new_len);
    snprintf(buffer, new_len*new_len, "mv %s %s", old_filename, new);
    assert(system(buffer)==0);

    // free created strings
    free(new);
    free(buffer);
}

    void
modifyFile(char* title, char* artist, char* album, char* old_filename)
{

    // Check if old_filename is a valid path
    FILE *file = fopen(old_filename, "r");
    if (file == NULL) {
        fprintf(stderr, "%s\n", "Please give a valid filepath: FLAC files should be in the 'input' directory.");
        exit(2);
    }
    fclose(file);
    FLAC__Metadata_SimpleIterator *flac_iter =
        FLAC__metadata_simple_iterator_new ();

    // Attach iterator to a FLAC file 
    if (!FLAC__metadata_simple_iterator_init
            (flac_iter, old_filename, false, false))
    {
        fprintf(stderr, "%s", "Error with initializing iterator\n");
    }

    // make sure the file is writable
    if (!FLAC__metadata_simple_iterator_is_writable (flac_iter))
    {
        fprintf(stderr, "%s", "flac iterator is not writable\n");
    }


    renameVorbis(flac_iter, title, album, artist);

    // "refreshes" iterator/iterate from the beginning of the flac file again
    if (!FLAC__metadata_simple_iterator_init
            (flac_iter, old_filename, false, false))
    {
        fprintf(stderr, "%s", "Error with reinitializing iterator\n");
    }

    FLAC__metadata_simple_iterator_delete(flac_iter);

    // Rename File
    renameFile(old_filename, title, album, artist);
}

void interactivelyRenameFiles(FileNode_t *files, int fileCount) {
  for (int i = 0; i < fileCount; i++) {
    FileNode_t *node = &files[i];
    printf("File: %s\n", node->filename);
    if (node->duration) {
      printf(" | Duration: %d:%d\n", node->duration / 60, node->duration % 60);
    }
    if (node->fingerprint) {
      printf(" | Fingerprint: %s\n", node->fingerprint);
    }
    char artist[MAX_METADATA_LENGTH];
    char album[MAX_METADATA_LENGTH];
    char tracktitle[MAX_METADATA_LENGTH];
    printf("\nTrack Name: ");
    scanf("%s",tracktitle);
    printf("\nAlbum Name: ");
    scanf("%s",album);
    printf("\nArtist Name: ");
    scanf("%s",artist);

    modifyFile(tracktitle, artist, album, node->filename);
  }
}
