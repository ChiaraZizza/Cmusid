#include <assert.h>
#include <FLAC/metadata.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void
practice (FLAC__Metadata_SimpleIterator * flac_iter, bool use, char *title,
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


      printf ("meta contents: %u\n", meta->type);
      if (meta->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
	{
            // empty vorbis comment entry to populate
	  FLAC__StreamMetadata_VorbisComment_Entry *new_entry =
	    (FLAC__StreamMetadata_VorbisComment_Entry *)
	    malloc (sizeof (FLAC__StreamMetadata_VorbisComment_Entry) * 3);

	  printf ("num_comments: %d\n",
		  meta->data.vorbis_comment.num_comments);

	  for (int i = 0; i < meta->data.vorbis_comment.num_comments; i++)
	    {
	      printf ("%s\n", meta->data.vorbis_comment.comments[i].entry);

	      if (use)
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

		  FLAC__metadata_object_vorbiscomment_replace_comment (temp,
								       *new_entry,
								       false,
								       true);
		  assert (FLAC__metadata_simple_iterator_set_block
			  (flac_iter, temp, true));
		}
	    }
	}
    }
}

void renameFile(char* old_filename, char* title, char* album, char* artist) {
    int tlen, alen, arlen;
    tlen = strlen(title);
    alen = strlen(album);
    arlen = strlen(artist);
    int new_len = tlen+alen+arlen + 10;
    char* new = malloc (sizeof(char)*new_len);
    snprintf(new, new_len, "%s--%s--%s.flac", artist, album, title);
}

void
test (char* title, char* artist, char* album, char* old_filename)
{
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


  printf ("First time running:\n");
  practice (flac_iter, true, title, album, artist);

  // "refreshes" iterator/iterate from the beginning of the flac file again
  if (!FLAC__metadata_simple_iterator_init
      (flac_iter, old_filename, false, false))
    {
      fprintf(stderr, "%s", "Error with reinitializing iterator\n");
    }

  printf ("\nSecond time running:\n");
  practice (flac_iter, false, title, album, artist);

  // Rename File
  renameFile(old_filename, title, album, artist);
}
