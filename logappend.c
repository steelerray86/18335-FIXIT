#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <ctype.h>

#include "data.h"

typedef struct _CmdLineResult
{
  int time;
  char *token;
  char *employee;
  char *guest;
  int arrive;
  int leave;
  int room;
  char *logpath;
  int good;
  int new;
  int batchCommand;
} CmdLineResult;

int do_batch(char *);

CmdLineResult parse_cmdline(int argc, char *argv[], int is_batch)
{
  CmdLineResult R = {0};
  int opt, r = -1;

  R.time = -1;
  R.token = "-";
  R.employee = "-";
  R.guest = "-";
  R.arrive = -1;
  R.leave = -1;
  R.room = -1;
  R.good = 0;
  R.new = -1;
  R.batchCommand = -1;

  int j;

  // argument data
  char *batchfile = NULL;
  // pick up the switches
  while ((opt = getopt(argc, argv, "T:K:E:G:ALR:B:")) != -1)
  {
    switch (opt)
    {
    case 'B':
      // batch file
      R.batchCommand = 0;
      if (is_batch == 1)
      {
        R.good = -1;
      }
      batchfile = optarg;
      break;

    case 'T':
      // timestamp
      if (R.time == -1){
        //Partial fix for break 0. Can only do -T once.
        for (int i = 0; i < strlen(optarg); i++)
        {
          if (!isdigit(optarg[i]))
          {
            R.good = -1;
            break;
          }
        }
        if (strlen(optarg) > 10) {
          R.good = -1;
        }
        R.time = atoi(optarg);
        if (R.time < 0 || R.time > 1073741823)
        {
          R.good = -1;
        }
      }else{
        R.good = -1;
      }
      break;

    case 'K':
      // secret token
      R.token = optarg;
      for (int i = 0; i < strlen(R.token); i++)
      {
        if (!isalnum(R.token[i]))
        {
          R.good = -1;
          break;
        }
      }
      break;

    case 'A':
      // arrival
      R.arrive = 0;
      break;

    case 'L':
      // departure
      R.leave = 0;
      break;

    case 'E':
      // employee name
      R.employee = optarg;
      for (int i = 0; i < strlen(R.employee); i++)
      {
        if (!isalpha(R.employee[i]))
        {
          R.good = -1;
          break;
        }
      }
      break;

    case 'G':
      // guest name
      R.guest = optarg;
      for (int i = 0; i < strlen(R.guest); i++)
      {
        if (!isalpha(R.guest[i]))
        {
          R.good = -1;
          break;
        }
      }
      break;

    case 'R':
      // room ID
      j = 0;
      for (int i = 0; i < strlen(optarg); i++)
      {
        if (!isdigit(optarg[i]))
        {
          R.good = -1;
          break;
        }
      }
      for (int i = 0; i < strlen(optarg); i++) {
        if (optarg[i] != 48) {
          j = i;
          break;
        }
      }
      char *room = calloc(strlen(optarg) - j + 1,1);
      strncpy(room, &optarg[j], strlen(optarg) - j);
      R.room = atoi(room);
      if (R.room < 0 || R.room > 1073741823)
      {
        R.good = -1;
      }
      break;

    default:
      // unknown option, leave
      break;
    }
  }

  // pick up the positional argument for log path
  if (optind < argc)
  {
    R.logpath = argv[optind];
    for (int i = 0; i < strlen(R.logpath); i++)
    {
      if (!isalnum(R.logpath[i]) && R.logpath[i] != 46 && R.logpath[i] != 47 && R.logpath[i] != 95)
      {
        if (is_batch == 1 && R.logpath[i] == 13)
        {
          R.logpath[strlen(R.logpath) - 1] = '\0';
        }
        else
        {
          R.good = -1;
          break;
        }
      }
    }
  }

  optind = 0;

  if (R.batchCommand == -1)
  {
    if (R.time == -1)
    {
      R.good = -1;
    }

    if (R.arrive == R.leave)
    {
      R.good = -1;
    }

    if (strcmp(R.employee, "-") == strcmp(R.guest, "-"))
    {
      R.good = -1;
    }
  }
  else if (R.batchCommand == 0)
  {
    if (R.time != -1)
    {
      R.good = -1;
    }
    if (strcmp(R.token, "-") != 0)
    {
      R.good = -1;
    }
    if (R.arrive == 0 || R.leave == 0)
    {
      R.good = -1;
    }

    if (strcmp(R.employee, "-") != 0 || strcmp(R.guest, "-") != 0)
    {
      R.good = -1;
    }
    if (R.room != -1)
    {
      R.good = -1;
    }
  }

  if (batchfile != NULL && R.good == 0 && is_batch == 0)
  {
    R.good = do_batch(batchfile);
  }

  if (R.good == 0 && R.batchCommand == -1)
  {
    FILE *file;
    if ((file = fopen(R.logpath, "r")))
    {
      fclose(file);

      Buffer A = read_from_path(R.logpath, R.token);
      if (A.Buf == NULL) {
        R.good = -1;
      }
      else if (strcmp(A.Buf, "") == 0)
      {
        R.good = -1;
      }
      else
      {
        char *data = A.Buf;
        char *rest = strchr(data, '\n');
        char *first = calloc(strlen(data) - strlen(rest) + 1, 1);
        strncpy(first, data, strlen(data) - strlen(rest));

        if (strcmp(first, R.token) != 0)
        {
          R.good = -1;
        }

        if (R.good != -1)
        {
          long index = strlen(first) + 1;
          for (int i = strlen(first); i < strlen(data); i++)
          {
            if (data[i] == '\n')
            {
              if (i != strlen(data) - 1)
              {
                index = i + 1;
              }
            }
          }
          char *last = calloc(strlen(data) - index + 1, 1);
          strncpy(last, &data[index], strlen(data) - index);
          char *lastTime = strtok(last, " ");
          if (R.time <= atoi(lastTime))
          {
            R.good = -1;
          }
          free(last);
        }

        if (R.good != -1)
        {
          char *name;
          int type;
          if (strcmp(R.employee, "-") != 0)
          {
            name = R.employee;
            type = 0;
          }
          else if (strcmp(R.guest, "-") != 0)
          {
            name = R.guest;
            type = 1;
          }

          char *person = calloc(strlen(name) + 3, 1);
          sprintf(person, "%s %d", name, type);

          char *line = calloc(strlen(data) + 1, 1);
          strncpy(line, data, strlen(data));
          char *temp = line;
          char *prev = calloc(strlen(data) + 1, 1);
          int done = 0;
          while (1)
          {
            prev = strcpy(prev, line);
            line = strstr(&line[1], person);
            if (line == NULL)
            {
              break;
            }
          }
          if (strcmp(prev, data) == 0)
          {
            if (R.arrive != 0)
            {
              R.good = -1;
            }
            if (R.room != -1)
            {
              R.good = -1;
            }
          }
          else
          {
            int aOrL;
            int room;
            char *tok = strtok(prev, " ");
            tok = strtok(NULL, " ");
            aOrL = atoi(strtok(NULL, " "));
            room = atoi(strtok(NULL, " "));

            if (R.arrive == 0 && R.room == -1)
            {
              if (aOrL == 1 || room != -1)
              {
                R.good = -1;
              }
            }
            else if (R.arrive == 0 && R.room != -1)
            {
              if (aOrL == 1 && room != -1)
              {
                R.good = -1;
              }
              if (aOrL == 0 && room == -1)
              {
                R.good = -1;
              }
            }
            else if (R.leave == 0 && R.room == -1)
            {
              if (aOrL == 1 && room != -1)
              {
                R.good = -1;
              }
              if (aOrL == 0 && room == -1)
              {
                R.good = -1;
              }
            }
            else if (R.leave == 0 && R.room != -1)
            {
              if (aOrL == 0 || room == -1)
              {
                R.good = -1;
              }
              if (R.room != room)
              {
                R.good = -1;
              }
            }
          }
          free(person);
          free(prev);
          free(temp);
        }
        free(first);
        free(A.Buf);
      }
    }
    else
    {
      if (R.arrive != 0 || R.room != -1)
      {
        R.good = -1;
      }
      R.new = 0;
    }
  }

  return R;
}

int do_batch(char *filepath)
{
  FILE *fp;
  if ((fp = fopen(filepath, "r")))
  {
    fclose(fp);
  }
  else
  {
    return -1;
  }

  FILE *infile;
  char *data;

  if ((infile = fopen(filepath, "r")))
  {
    fseek(infile, 0, SEEK_END);
    long fsize = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    data = calloc(fsize + 1, 1);
    fread(data, fsize, 1, infile);
    fclose(infile);
    data[fsize] = 0;
  }
  char *temp1 = data;
  char *line;
  char *line2;
  char *linedata;
  char *endline;
  int i;

  CmdLineResult R;
  for (;; data = NULL)
  {
    line = strtok_r(data, "\n", &endline);
    if (line == NULL)
    {
      break;
    }
    line2 = calloc(strlen(line) + 1, 1);
    strncpy(line2, line, strlen(line));
    char *temp2 = line2;
    int i = 0;
    char *endline3;
    for (;; line2 = NULL)
    {
      i++;
      linedata = strtok_r(line2, " ", &endline3);
      if (linedata == NULL)
      {
        break;
      }
    }
    char *newArgs[i];
    char *temp3[i];
    newArgs[0] = "./logappend";
    temp3[0] = newArgs[0];
    char *endline2;
    for (int j = 1; j < i; j++, line = NULL)
    {
      linedata = strtok_r(line, " ", &endline2);
      newArgs[j] = calloc(strlen(linedata) + 1, 1);
      strncpy(newArgs[j], linedata, strlen(linedata));
      temp3[j] = newArgs[j];
    }

    R = parse_cmdline(i, newArgs, 1);

    free(temp2);

    int r = 0;
    if (R.good == 0)
    {
      Buffer B = {0};
      char *time = calloc(13, 1);
      long tLen = sprintf(time, "%d", R.time);
      int aOrL = -1;
      if (R.arrive == 0)
        aOrL = 1;
      else if (R.leave == 0)
        aOrL = 0;
      char *name;
      int eOrG;
      if (strcmp(R.employee, "-") != 0)
      {
        name = R.employee;
        eOrG = 0;
      }
      else if (strcmp(R.guest, "-") != 0)
      {
        name = R.guest;
        eOrG = 1;
      }
      char *room = calloc(13, 1);
      long rLen = sprintf(room, "%d", R.room);

      long lineLen = tLen + strlen(name) + 2 + rLen + 5;
      char *line = calloc(lineLen + 1, 1);
      lineLen = sprintf(line, "%s %s %d %d %s\n", time, name, eOrG, aOrL, room);
      B.Buf = line;
      B.Length = lineLen;

      Buffer C = {0};
      if (R.new == 0)
      {
        long firstLen = strlen(R.token) + 1;
        char *first = calloc(firstLen + 1, 1);
        firstLen = sprintf(first, "%s\n", R.token);
        Buffer F = {0};
        F.Buf = first;
        F.Length = firstLen;

        C = concat_buffs(&F, &B);
        r = write_to_path(R.logpath, &C, R.token);

        free(first);
      }
      else
      {
        Buffer A = read_from_path(R.logpath, R.token);
        if (A.Buf == NULL) {
          printf("invalid\n");
        }
        else if (strcmp(A.Buf, "") == 0)
        {
          printf("invalid\n");
        }
        C = concat_buffs(&A, &B);
        r = write_to_path(R.logpath, &C, R.token);
        free(A.Buf);
      }
      free(time);
      free(room);
      free(line);
      free(C.Buf);
    }
    else
    {
      printf("invalid\n");
    }
    for (int a = 1; a < i; a++)
    {
      free(temp3[a]);
    }
  }
  free(temp1);
  return 0;
}

int main(int argc, char *argv[])
{
  int r = 0;
  CmdLineResult R;

  R = parse_cmdline(argc, argv, 0);
  if (R.good == 0 && R.batchCommand == -1)
  {
    Buffer B = {0};
    char *time = calloc(13, 1);
    long tLen = sprintf(time, "%d", R.time);
    int aOrL = -1;
    if (R.arrive == 0)
      aOrL = 1;
    else if (R.leave == 0)
      aOrL = 0;
    char *name;
    int eOrG;
    if (strcmp(R.employee, "-") != 0)
    {
      name = R.employee;
      eOrG = 0;
    }
    else if (strcmp(R.guest, "-") != 0)
    {
      name = R.guest;
      eOrG = 1;
    }
    char *room = calloc(13, 1);
    long rLen = sprintf(room, "%d", R.room);

    long lineLen = tLen + strlen(name) + 2 + rLen + 5;
    char *line = calloc(lineLen + 1, 1);
    lineLen = sprintf(line, "%s %s %d %d %s\n", time, name, eOrG, aOrL, room);
    B.Buf = line;
    B.Length = lineLen;

    Buffer C = {0};
    if (R.new == 0)
    {
      long firstLen = strlen(R.token) + 1;
      char *first = calloc(firstLen + 1, 1);
      firstLen = sprintf(first, "%s\n", R.token);
      Buffer F = {0};
      F.Buf = first;
      F.Length = firstLen;

      C = concat_buffs(&F, &B);
      r = write_to_path(R.logpath, &C, R.token);

      free(first);
    }
    else
    {
      Buffer A = read_from_path(R.logpath, R.token);
      if (A.Buf == NULL) {
        printf("invalid");
        return 255;
      }
      else if (strcmp(A.Buf, "") == 0)
      {
        printf("invalid");
        return 255;
      }
      C = concat_buffs(&A, &B);
      r = write_to_path(R.logpath, &C, R.token);
      free(A.Buf);
    }
    free(time);
    free(room);
    free(line);
    free(C.Buf);
  }
  else if (R.good == -1)
  {
    printf("invalid");
    return 255;
  }

  return r;
}