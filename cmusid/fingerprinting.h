typedef struct FileNode {
  char * filename;
  char * fingerprint;
  int duration;
} FileNode_t;

int fingerprintFile(char* filePath, char** fingerprint);
