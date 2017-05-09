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
      FLAC__StreamMetadata *meta =
	FLAC__metadata_simple_iterator_get_block (flac_iter);
      FLAC__StreamMetadata *temp =
	FLAC__metadata_object_new (FLAC__METADATA_TYPE_VORBIS_COMMENT);


      printf ("meta contents: %u\n", meta->type);
      if (meta->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
	{
	  FLAC__StreamMetadata_VorbisComment_Entry *t =
	    (FLAC__StreamMetadata_VorbisComment_Entry *)
	    malloc (sizeof (FLAC__StreamMetadata_VorbisComment_Entry) * 3);

	  //temp->data.vorbis_comment.comments = (FLAC__StreamMetadata_VorbisComment_Entry*)malloc(sizeof(FLAC__StreamMetadata_VorbisComment_Entry) * 3);

	//  t[0].entry = (FLAC__byte *) malloc (sizeof (FLAC__byte) * tlen);
	//  t[1].entry = (FLAC__byte *) malloc (sizeof (FLAC__byte) * allen);
	//  t[2].entry = (FLAC__byte *) malloc (sizeof (FLAC__byte) * arlen);

	  //FLAC__StreamMetadata_VorbisComment vorbis = meta->data.vorbis_comment;

	  printf ("num_comments: %d\n",
		  meta->data.vorbis_comment.num_comments);

	  for (int i = 0; i < meta->data.vorbis_comment.num_comments; i++)
	    {
	      printf ("%s\n", meta->data.vorbis_comment.comments[i].entry);

	      if (use)
		{

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

		  //create vorbis entry and populate it
		  t[0].length = tlen;
		  t[0].entry = title_bytes;

		  t[1].length = allen;
		  t[1].entry = album_bytes;

		  t[2].length = arlen;
		  t[2].entry = artist_bytes;

		  temp->data.vorbis_comment.comments = t;
		  temp->data.vorbis_comment.num_comments = 3;

		  FLAC__metadata_object_vorbiscomment_replace_comment (temp,
								       *t,
								       false,
								       true);
		  assert (FLAC__metadata_simple_iterator_set_block
			  (flac_iter, temp, true));
		}
	    }
	}
    }
  printf ("Hello\n");
}

void
test (char* title, char* artist, char* album, char* old_filename)
{
    /*
  char *title = "TITLE=Keep My Cool";
  char *artist = "ARTIST=the Wellness";
  char *album = "ALBUM=doubles";
  */

  FLAC__Metadata_SimpleIterator *flac_iter =
    FLAC__metadata_simple_iterator_new ();

  // Attach iterator to a FLAC file (this function returns a flac bool)
  if (!FLAC__metadata_simple_iterator_init
      (flac_iter, old_filename, false, false))
    {
      // error message
      printf ("Error with initializing iterator\n");
    }

  // make sure the file is writable
  if (!FLAC__metadata_simple_iterator_is_writable (flac_iter))
    {
      // error? flac isn't writable
      // print something, then exit
      printf ("iter1 failed\n");
    }


  printf ("First time running:\n");
  practice (flac_iter, true, title, album, artist);
  FLAC__metadata_simple_iterator_delete (flac_iter);




  FLAC__Metadata_SimpleIterator *flac_iter2 =
    FLAC__metadata_simple_iterator_new ();

  // Attach iterator to a FLAC file (this function returns a flac bool)
  if (!FLAC__metadata_simple_iterator_init
      (flac_iter2, "input/cool.flac", false, false))
    {
      // error message
      printf ("Error with initializing iterator\n");
    }

  // make sure the file is writable
  if (!FLAC__metadata_simple_iterator_is_writable (flac_iter2))
    {
      // error? flac isn't writable
      // print something, then exit
      printf ("iter2 failed\n");
    }


  printf ("\nSecond time running:\n");
  practice (flac_iter2, false, title, album, artist);

}
