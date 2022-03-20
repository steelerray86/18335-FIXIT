#ifndef _BUFFR_H
#define _BUFFR_H

typedef struct _Buffer
{
  long Length;
  char *Buf;
} Buffer;

Buffer read_from_path(char *path, char *key);
int write_to_path(char *path, Buffer *B, char *key);
Buffer concat_buffs(Buffer *A, Buffer *B);

#endif
