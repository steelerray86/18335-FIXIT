#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <ctype.h>

#include "data.h"

int verbose = 0;

struct Person
{
  char *name;
  int hasarrived;
  int currroom;
  int type;

  int *rooms;
  int numRooms;
  int roomSize;

  int arriveTime;
  int timeSoFar;
  SLIST_ENTRY(Person)
  link;
};

SLIST_HEAD(slisthead, Person);

struct myRoomrecord
{
  int roomid;
  SLIST_ENTRY(myRoomrecord)
  link;
};

SLIST_HEAD(rrhead, myRoomrecord);

int main(int argc, char *argv[])
{
  int opt, len;
  char *logpath = NULL;
  int roomlist = 0;
  int printlog = 0;
  int tFlag = 0;
  int iFlag = 0;
  char *token = "-";
  char *employee = "-";
  char *guest = "-";
  int type = -1;

  while ((opt = getopt(argc, argv, "K:SRE:G:TI")) != -1)
  {
    switch (opt)
    {
    case 'K':
      token = optarg;
      for (int i = 0; i < strlen(token); i++)
      {
        if (!isalnum(token[i]))
        {
          printf("invalid");
          return 255;
        }
      }
      break;

    case 'S':
      printlog = 1;
      break;

    case 'R':
      roomlist = 1;
      break;

    case 'T':
      tFlag = 1;
      break;

    case 'I':
      iFlag = 1;
      printf("unimplemented");
      break;

    case 'E':
      employee = optarg;
      type = 0;
      for (int i = 0; i < strlen(employee); i++)
      {
        if (!isalpha(employee[i]))
        {
          printf("invalid");
          return 255;
        }
      }
      break;

    case 'G':
      guest = optarg;
      type = 1;
      for (int i = 0; i < strlen(guest); i++)
      {
        if (!isalpha(guest[i]))
        {
          printf("invalid");
          return 255;
        }
      }
      break;
    }
  }

  if (optind < argc)
  {
    logpath = argv[optind];
    for (int i = 0; i < strlen(logpath); i++)
    {
      if (!isalnum(logpath[i]) && (logpath[i] != 46 && logpath[i] != 47 && logpath[i] != 95))
      {
        printf("invalid");
        return 255;
      }
    }
  }

  if (strcmp(token, "-") == 0)
  {
    printf("invalid");
    return 255;
  }

  if (printlog + roomlist + tFlag + iFlag != 1)
  {
    printf("invalid");
    return 255;
  }

  if (printlog == 1)
  {
    if (strcmp(employee, "-") != 0 || strcmp(guest, "-") != 0)
    {
      printf("invalid");
      return 255;
    }
  }

  if (roomlist == 1 || tFlag == 1)
  {
    if (strcmp(employee, "-") == strcmp(guest, "-"))
    {
      printf("invalid");
      return 255;
    }
  }

  FILE *file;
  if ((file = fopen(logpath, "r")))
  {
    fclose(file);
  }
  else
  {
    //Fix for break 6 and partial fix for 0 and 1. If unimplemented log is
    //passed in print two spaces and be done.
    printf("\n\n");
    return 0;
  }

  Buffer A = read_from_path(logpath, token);
  if (A.Buf == NULL) {
    printf("integrity violation");
    return 255;
  }
  else if (strcmp(A.Buf, "") == 0)
  {
    printf("integrity violation");
    return 255;
  }
  char *data = calloc(A.Length + 1, 1);
  int datalen = sprintf(data, "%s", A.Buf);
  char *endline;
  char *line = strtok_r(data, "\n", &endline);
  char *endline2;
  char *linedata;
  int currTime;
  if (strcmp(line, token) == 0)
  {
    struct slisthead people;
    struct rrhead roominfo;
    struct Person *temp;
    struct myRoomrecord *temproom;

    SLIST_INIT(&people);
    SLIST_INIT(&roominfo);
    while ((line = strtok_r(NULL, "\n", &endline)))
    {
      temp = malloc(sizeof(struct Person));
      linedata = strtok_r(line, " ", &endline2);
      currTime = atoi(linedata);
      linedata = strtok_r(NULL, " ", &endline2);
      temp->name = linedata;
      linedata = strtok_r(NULL, " ", &endline2);
      temp->type = atoi(linedata);
      linedata = strtok_r(NULL, " ", &endline2);
      temp->hasarrived = atoi(linedata);
      linedata = strtok_r(NULL, " ", &endline2);
      temp->currroom = atoi(linedata);
      if (SLIST_EMPTY(&people))
      {
        temp->rooms = calloc(4 * sizeof(int), 1);
        temp->numRooms = 0;
        temp->roomSize = 4;
        temp->arriveTime = currTime;
        temp->timeSoFar = 0;
        SLIST_INSERT_HEAD(&people, temp, link);
      }
      else
      {
        struct Person *ptr = SLIST_FIRST(&people);
        int cmp = strcmp(temp->name, ptr->name);
        if (cmp != 0)
        {
          if (cmp < 0)
          {
            temp->rooms = calloc(4 * sizeof(int), 1);
            temp->numRooms = 0;
            temp->roomSize = 4;
            temp->arriveTime = currTime;
            temp->timeSoFar = 0;
            SLIST_INSERT_HEAD(&people, temp, link);
          }
          else
          {
            SLIST_FOREACH(ptr, &people, link)
            {
              if (SLIST_NEXT(ptr, link) == NULL)
              {
                temp->rooms = calloc(4 * sizeof(int), 1);
                temp->numRooms = 0;
                temp->roomSize = 4;
                temp->arriveTime = currTime;
                temp->timeSoFar = 0;
                SLIST_INSERT_AFTER(ptr, temp, link);
                break;
              }
              else if (strcmp(temp->name, SLIST_NEXT(ptr, link)->name) == 0)
              {
                // check for same name diff role
                if (temp->type != SLIST_NEXT(ptr, link)->type)
                {
                  temp->rooms = calloc(4 * sizeof(int), 1);
                  temp->numRooms = 0;
                  temp->roomSize = 4;
                  temp->arriveTime = currTime;
                  temp->timeSoFar = 0;
                  SLIST_INSERT_AFTER(ptr, temp, link);
                  break;
                }
                else
                {
                  if (temp->hasarrived == 0)
                  {
                    if (temp->currroom == -1)
                    {
                      SLIST_NEXT(ptr, link)->currroom = temp->currroom;
                      SLIST_NEXT(ptr, link)->hasarrived = 0;
                      SLIST_NEXT(ptr, link)->timeSoFar = currTime - SLIST_NEXT(ptr, link)->arriveTime;
                      SLIST_NEXT(ptr, link)->arriveTime = currTime;
                    }
                    else
                    {
                      SLIST_NEXT(ptr, link)->currroom = -1;
                      SLIST_NEXT(ptr, link)->hasarrived = 1;
                    }
                  }
                  else
                  {
                    if (temp->currroom == -1)
                    {
                      SLIST_NEXT(ptr, link)->arriveTime = currTime;
                    }
                    SLIST_NEXT(ptr, link)->currroom = temp->currroom;
                    SLIST_NEXT(ptr, link)->hasarrived = temp->hasarrived;
                    if (SLIST_NEXT(ptr, link)->numRooms == SLIST_NEXT(ptr, link)->roomSize)
                    {
                      SLIST_NEXT(ptr, link)->roomSize *= 2;
                      SLIST_NEXT(ptr, link)->rooms = realloc(SLIST_NEXT(ptr, link)->rooms, SLIST_NEXT(ptr, link)->roomSize * sizeof(int));
                    }
                    SLIST_NEXT(ptr, link)->rooms[SLIST_NEXT(ptr, link)->numRooms] = temp->currroom;
                    SLIST_NEXT(ptr, link)->numRooms += 1;
                  }
                }
                break;
              }
              else
              {
                if (strcmp(temp->name, SLIST_NEXT(ptr, link)->name) < 0)
                {
                  temp->rooms = calloc(4 * sizeof(int), 1);
                  temp->numRooms = 0;
                  temp->roomSize = 4;
                  temp->arriveTime = currTime;
                  temp->timeSoFar = 0;
                  SLIST_INSERT_AFTER(ptr, temp, link);
                  break;
                }
              }
            }
          }
        }
        else
        {
          if (temp->type != ptr->type)
          {
            temp->rooms = calloc(4 * sizeof(int), 1);
            temp->numRooms = 0;
            temp->roomSize = 4;
            temp->arriveTime = currTime;
            temp->timeSoFar = 0;
            SLIST_INSERT_AFTER(ptr, temp, link);
          }
          else
          {
            if (temp->hasarrived == 0)
            {
              if (temp->currroom == -1)
              {
                ptr->currroom = temp->currroom;
                ptr->hasarrived = 0;
                ptr->timeSoFar = currTime - ptr->arriveTime;
                ptr->arriveTime = currTime;
              }
              else
              {
                ptr->currroom = -1;
                ptr->hasarrived = 1;
              }
            }
            else
            {
              if (temp->currroom == -1)
              {
                ptr->arriveTime = currTime;
              }
              ptr->currroom = temp->currroom;
              ptr->hasarrived = temp->hasarrived;
              if (ptr->numRooms == ptr->roomSize)
              {
                ptr->roomSize *= 2;
                ptr->rooms = realloc(ptr->rooms, ptr->roomSize * sizeof(int));
              }
              ptr->rooms[ptr->numRooms] = temp->currroom;
              ptr->numRooms += 1;
            }
          }
        }
      }
      if (temp->currroom != -1)
      {
        temproom = malloc(sizeof(struct myRoomrecord));
        temproom->roomid = temp->currroom;

        if (SLIST_EMPTY(&roominfo))
        {
          SLIST_INSERT_HEAD(&roominfo, temproom, link);
        }
        else
        {
          struct myRoomrecord *rptr = SLIST_FIRST(&roominfo);
          if (rptr->roomid != temproom->roomid)
          {
            if (rptr->roomid > temproom->roomid)
            {
              SLIST_INSERT_HEAD(&roominfo, temproom, link);
            }
            else if (rptr->roomid < temproom->roomid)
            {
              SLIST_FOREACH(rptr, &roominfo, link)
              {
                if (SLIST_NEXT(rptr, link) == NULL)
                {
                  SLIST_INSERT_AFTER(rptr, temproom, link);
                  break;
                }
                else if (temproom->roomid == SLIST_NEXT(rptr, link)->roomid)
                {
                  break;
                }
                else
                {
                  if (temproom->roomid < SLIST_NEXT(rptr, link)->roomid)
                  {
                    SLIST_INSERT_AFTER(rptr, temproom, link);
                    break;
                  }
                }
              }
            }
          }
        }
      }
    }

    if (printlog == 1)
    {
      struct Person *np1;
      int j = 0;
      SLIST_FOREACH(np1, &people, link)
      {
        if (np1->type == 0 && np1->hasarrived == 1)
        {
          if (j == 0)
          {
            printf("%s", np1->name);
            j = 1;
          }
          else
          {
            printf(",%s", np1->name);
          }
        }
      }
      printf("\n");
      j = 0;
      SLIST_FOREACH(np1, &people, link)
      {
        if (np1->type == 1 && np1->hasarrived == 1)
        {
          if (j == 0)
          {
            printf("%s", np1->name);
            j = 1;
          }
          else
          {
            printf(",%s", np1->name);
          }
        }
      }

      j = 0;
      int n = 0;
      struct myRoomrecord *np2;
      SLIST_FOREACH(np2, &roominfo, link)
      {
        SLIST_FOREACH(np1, &people, link)
        {
          if (np1->currroom == np2->roomid)
          {
            if (j == 0)
            {
              n = 1;
              printf("\n%d: ", np2->roomid);
              printf("%s", np1->name);
              j = 1;
            }
            else
            {
              printf(",%s", np1->name);
            }
          }
        }
        j = 0;
      }
      if (n == 0)
        printf("\n");
    }
    else if (roomlist == 1)
    {
      int j = 0;
      struct Person *np1;
      SLIST_FOREACH(np1, &people, link)
      {
        if (type == 0 && np1->type == 0)
        {
          if (strcmp(np1->name, employee) == 0)
          {
            for (int i = 0; i < np1->numRooms; i++)
            {
              if (j == 0)
              {
                printf("%d", np1->rooms[i]);
                j = 1;
              }
              else
              {
                printf(",%d", np1->rooms[i]);
              }
            }
          }
        }
        else if (type == 1 && np1->type == 1)
        {
          if (strcmp(np1->name, guest) == 0)
          {
            for (int i = 0; i < np1->numRooms; i++)
            {
              if (j == 0)
              {
                printf("%d", np1->rooms[i]);
                j = 1;
              }
              else
              {
                printf(",%d", np1->rooms[i]);
              }
            }
          }
        }
      }
    }
    else if (tFlag == 1)
    {
      int j = 0;
      struct Person *np1;
      SLIST_FOREACH(np1, &people, link)
      {
        if (type == 0 && np1->type == 0)
        {
          if (strcmp(np1->name, employee) == 0)
          {
            if (np1->hasarrived == 0)
            {
              printf("%d", np1->timeSoFar);
            }
            else
            {
              printf("%d", np1->timeSoFar + currTime - np1->arriveTime);
            }
          }
        }
        else if (type == 1 && np1->type == 1)
        {
          if (strcmp(np1->name, guest) == 0)
          {
            if (np1->hasarrived == 0)
            {
              printf("%d", np1->timeSoFar);
            }
            else
            {
              printf("%d", np1->timeSoFar + currTime - np1->arriveTime);
            }
          }
        }
      }
    }
  }
  else
  {
    printf("integrity violation");
    return 255;
  }

  return 0;
}
