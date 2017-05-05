#define _GNU_SOURCE

#include <assert.h>
#include <chromaprint.h>
#include <curl/curl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <FLAC/Metadata.h>

typedef struct FileNode {
  FILE *file;
  char* filename;
  char* fingerprint;
  int duration;
} FileNode_t;

typedef struct arg {
  FileNode_t* arr;
  int threadNo;
  int totalNumFiles;
} arg_t;

typedef FLAC__StreamMetadata data;

#define NO_THREADS 4
const int sample_rate = 44100;
const int num_channels = 2;
const int FILE_BLOCK_SIZE = 128;
const char* ACOUSTID_API_URL = "http://api.acoustid.org/v2/lookup";
const char* APPLICATION_ID = "jif76R78Wd";

int fingerprintFile(FILE *file, char** fingerprint) {
  ChromaprintContext *ctx;
  ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);

  chromaprint_start(ctx,sample_rate,num_channels);

 int16_t *buffer = (int16_t*) malloc((FILE_BLOCK_SIZE * sizeof(int16_t)));
  int read;
  while ((read = fread(buffer,FILE_BLOCK_SIZE*sizeof(int16_t), 1, file))) {
    if(!chromaprint_feed(ctx,buffer,read/sizeof(int16_t))) {
      fprintf(stderr, "Error feeding Chromaprint from buffer\n");
      exit(2);
    }
  }

  if (!chromaprint_finish(ctx)) {
    fprintf(stderr, "Error finishing Chromaprint feed\n");
    exit(2);
  }
  free(buffer);
  if (!chromaprint_get_fingerprint(ctx, fingerprint)) {
    fprintf(stderr, "Error retrieving fingerprint from Chromaprint\n");
    exit(2);
  }
  int duration = chromaprint_get_item_duration_ms(ctx);
  chromaprint_free(ctx);

  return duration;
}

void fetchMetadata(char* fingerprint, int duration) {
  printf("Fetching Metadata...\n | Duration: %d\n | Fingerprint: %s\n",duration, fingerprint);
  CURL *curl = curl_easy_init();
  if(curl) {
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, ACOUSTID_API_URL);
    char *body;
    asprintf(&body,"client=%s&duration=%d&fingerprint=%s", APPLICATION_ID, duration, fingerprint);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
          curl_easy_strerror(res));
    curl_easy_cleanup(curl);
    free(body);
  } else {
    fprintf(stderr, "Error Initializing cURL");
  }
}

FileNode_t* consumeDirectory(char* directory, int *filecount) {
  DIR *d = opendir(directory);
  struct dirent *dir;
  *filecount = 0;
  while ((dir = readdir(d)) != NULL) {
    if (strstr(dir->d_name, ".flac") != NULL) {
      (*filecount)++;
    }
  }
  closedir(d);

  int i = 0;
  d = opendir(directory);
  FileNode_t *flacFiles = malloc(sizeof(FileNode_t) * (*filecount));
  while ((dir = readdir(d)) != NULL) {
    if (strstr(dir->d_name, ".flac") != NULL) {
      FileNode_t *node = &flacFiles[i];
      char* pathname;
      asprintf(&pathname, "%s/%s", directory, dir->d_name);
      FILE *file = fopen(pathname, "r");
      node->file = file;
      node->filename = dir->d_name;
      i++;
    }
  }
  assert(*filecount == i);

  return flacFiles;
}

void* threadFunction (void* voidArgs) {
  arg_t* args = (arg_t*) voidArgs;
  for (int i = args->threadNo; i < args->totalNumFiles; i+= NO_THREADS) {
    FileNode_t *cur = &(args->arr)[i];
    cur->duration = fingerprintFile(cur->file,&cur->fingerprint);
  }
  return NULL;
}

int main (int argc, char *argv[]) {

  /*
  int totalNumFiles;
  FileNode_t *flacFiles = consumeDirectory(argv[1], &totalNumFiles);
  char *fingerprint;
  int duration;

  pthread_t threads[NO_THREADS];
  for (int i = 0; i < NO_THREADS; i++) {
    arg_t *arg = (arg_t*) malloc(sizeof(arg_t));
    arg->arr = flacFiles;
    arg->totalNumFiles = totalNumFiles;
    arg->threadNo = i;
    if (pthread_create(&(threads[i]), NULL, threadFunction, arg)) {
      perror("Error creating thread");
    }
  }
  for(int i = 0; i < NO_THREADS; i++) {
    if (pthread_join(threads[i],0)) {
      perror("Error joining with thread");
      exit(2);
    }
  }
  for(int i = 0; i < totalNumFiles; i++) {
    FileNode_t *node = &flacFiles[i];
    printf("File: %s\n | Duration: %d\n | Fingerprint: %s\n\n",node->filename, node->duration, node->fingerprint);
  }
  */

  // check if valid file
  // create a flac iterator
  FLAC__Metadata_SimpleIterator *flac_iter = FLAC__metadata_simple_iterator_new();

  // Attach iterator to a FLAC file (this function returns a flac bool)
  if (!FLAC__metadata_simple_iterator_init(flac_iter, "input/cool.flac", false, false)) {
    // error message
  }

  // make sure the file is writable
  if (!FLAC__metadata_simple_iterator_is_writable(flac_iter)) {
    // error? flac isn't writable
    // print something, then exit
  }

  // iterate while there are blocks to read
  while (FLAC__metadata_simple_iterator_next(flac_iter)) {
    FLAC__StreamMetadata* meta = FLAC__metadata_simple_iterator_get_block(flac_iter);
    FLAC__StreamMetadata* temp = FLAC__metadata_object_new (FLAC__METADATA_TYPE_VORBIS_COMMENT);
    //printf("meta contents: %u\n", meta->type);
    if(meta->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
      FLAC__StreamMetadata_VorbisComment vorbis = meta->data.vorbis_comment;
      for(int i = 0; i < vorbis.num_comments; i++) {
	//printf("%s\n", vorbis.comments[i].entry);
      }
    }

    FLAC__StreamMetadata_VorbisComment fill = temp->data.vorbis_comment;

    //create vorbis entry and populate it
    FLAC__StreamMetadata_VorbisComment_Entry* t = (FLAC__StreamMetadata_VorbisComment_Entry*)
      malloc(sizeof(FLAC__StreamMetadata_VorbisComment_Entry));
    char* title = (char*)malloc(sizeof(char) * 20);
    title = "TITLE";
    memcpy(t, title, 10);
    
    //returns FLAC__bool
    //FLAC__metadata_object_vorbiscomment_replace_comment(temp, *t, false, false);



    //gdb
    //print temp->data.vorbis_comment






    

    //populate field
    
    title = "Title";
    FLAC__byte* z = malloc(sizeof(char) * 20);// = x;
    memcpy(z, title, 10);

    FLAC__metadata_object_application_set_data(temp, z, 20, false);
    //FLAC__metadata_object_vorbiscomment_set_comment(temp, 0, *t, false);
    

    
    //fill.comments[0] = malloc(sizeof(FLAC__StreamMetadata_VorbisComment_Entry));
    //a = fill.comments[0].entry;
    //memcpy(fill.comments[0].entry, title, 20);

    //fill.comments[0].entry = title;
    //a = malloc(sizeof(char) * 20);
    //memcpy(a, title, 20);

    //increment num_comments
    fill.num_comments = 1;
    
    for(int i = 0; i < fill.num_comments; i++) {
      printf("%s\n", fill.vendor_string);
    }

    //TESTING
    /*
    char* x = (char*)malloc(sizeof(char) * 20);
    x = "string";
    FLAC__byte* b = malloc(sizeof(char) * 20);// = x;
    memcpy(b, x, 10);
    //printf("String: %s\nByte: %d\n", x, b);
    */

    //pass in three paramaters to function:
    //want TITLE, ARTIST, ALBUM
    //FLAC__metadata_simple_iterator_set_block(flac_iter, temp, true);
 }
  return 0;
}
