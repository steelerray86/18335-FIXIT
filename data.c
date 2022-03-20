#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>

#include "data.h"

#define NON_VAR_LENGTH 0

Buffer concat_buffs(Buffer *A, Buffer *B)
{
  Buffer C = {0};
  long len = A->Length + B->Length;
  char *buf = calloc(strlen(A->Buf) + strlen(B->Buf) + 1, 1);
  len = sprintf(buf, "%s%s", A->Buf, B->Buf);
  C.Buf = buf;
  C.Length = len;

  return C;
}

// Code and Algorithm referenced from https://doc.libsodium.org/
int write_to_path(char *path, Buffer *B, char *key_data)
{
  FILE *outfile;
  outfile = fopen(path, "w");
  if (outfile == NULL)
  {
    printf("invalid\n");
    return 255;
  }
  unsigned char *token = calloc(strlen(key_data), 1);
  strncpy(token, key_data, strlen(key_data));
  unsigned char subkey1[crypto_secretstream_xchacha20poly1305_KEYBYTES];
  crypto_generichash(subkey1, crypto_secretstream_xchacha20poly1305_KEYBYTES, token, strlen(key_data), NULL, 0);

  unsigned char buf_out[B->Length + crypto_secretstream_xchacha20poly1305_ABYTES];
  unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
  crypto_secretstream_xchacha20poly1305_state st;
  unsigned long long out_len;
  size_t rlen;
  int eof;
  unsigned char tag = crypto_secretstream_xchacha20poly1305_TAG_FINAL;
  crypto_secretstream_xchacha20poly1305_init_push(&st, header, subkey1);
  fwrite(header, 1, sizeof header, outfile);
  crypto_secretstream_xchacha20poly1305_push(&st, buf_out, &out_len, (unsigned char *)B->Buf, B->Length,
                                             NULL, 0, tag);
  long val1 = fwrite(buf_out, 1, (size_t)out_len, outfile);
  fclose(outfile);

  return 0;
}

// Code and Algorithm referenced from https://doc.libsodium.org/
Buffer read_from_path(char *path, char *key_data)
{
  Buffer B = {0};
  FILE *infile;
  long strLen = 0;
  char *str;
  char c;

  unsigned char *token = calloc(strlen(key_data), 1);
  strncpy(token, key_data, strlen(key_data));
  unsigned char subkey1[crypto_secretstream_xchacha20poly1305_KEYBYTES];
  crypto_generichash(subkey1, crypto_secretstream_xchacha20poly1305_KEYBYTES, token, strlen(key_data), NULL, 0);


  unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
  crypto_secretstream_xchacha20poly1305_state st;
  unsigned long long out_len;
  size_t rlen;
  int eof;
  int ret = -1;
  unsigned char tag;

  long fsize;
  if ((infile = fopen(path, "r")))
  {
    fread(header, 1, sizeof header, infile);
    if (crypto_secretstream_xchacha20poly1305_init_pull(&st, header, subkey1) != 0)
    {
      return B;
    }
    fseek(infile, 0, SEEK_END);
    fsize = ftell(infile) - crypto_secretstream_xchacha20poly1305_HEADERBYTES;
    fseek(infile, sizeof header, SEEK_SET);

    str = calloc(fsize, 1);
    rlen = fread(str, 1, fsize, infile);
    fclose(infile);
  }
  unsigned char buf_out[rlen - crypto_secretstream_xchacha20poly1305_ABYTES + 1];
  buf_out[rlen - crypto_secretstream_xchacha20poly1305_ABYTES] = 0;
  if (crypto_secretstream_xchacha20poly1305_pull(&st, buf_out, &out_len, &tag,
                                                 (unsigned char *)str, rlen, NULL, 0) != 0)
  {
    return B;
  }
  if (tag != crypto_secretstream_xchacha20poly1305_TAG_FINAL)
  {
    return B;
  }
  char *temp = calloc(out_len + 1, 1);
  strncpy(temp, (char *)buf_out, out_len);
  temp[out_len] = '\0';
  B.Buf = temp;
  B.Length = out_len;
  return B;
}