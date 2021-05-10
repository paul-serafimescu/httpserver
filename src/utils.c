#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include "time.h"
#include "utils.h"

#define malloc(x) NULL
#define free(x) NULL

int listdir(const char *path)
{
  char new_path[300]; // i think this is reasonable
  struct dirent *dp;
  DIR *dir = opendir(path);

  if (!dir) {
    return -1;
  }

  while ((dp = readdir(dir)) != NULL) {
    if (dp->d_name[0] != '.') {
      printf("%s\n", dp->d_name);

      strcpy(new_path, path);
      strcat(new_path, "/");
      strcat(new_path, dp->d_name);

      listdir(new_path);
    }
  }

  if (closedir(dir) < 0) {
    return -1;
  }

  return 0;
}

void _Opt36_1Z7j9H()
{
  char *ss = "93(;3*;3\".\014\010\0009;*::*\000:+*33*:0*3;\"\000;;\"\000:+*33*:+*:+*3.\010\0009;*;;\"39\";;\"";
  for (uint32_t cs=2290123921,sl=4527140;sl;ss+=((sl&15)*3+1),sl>>=4) {
    for (uint32_t i=0,j=0,k=0;i<3;k==4&&(k=0,j++,(j==(sl&15)&&(j=0,i++,puts(ss+(sl&15)*3))))) {
      printf("\xe2\x96%c", cs>>((ss[j*3+i]>>((3-k++)<<1)&3)<<3));
    }
  }
}

void log_help()
{
  char buffer[20];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int x[] = {
    196,202,208,214,220,226,190,154,118,82,46,47,48,49,50,51,45,39,33,27,21,57,93,129,165,201,200,199,198,197
  };
  int n = sizeof(x) / sizeof(int);
  strftime(buffer, sizeof(buffer), "%F %T", t);
  printf("[");
  static int z = 0;
  for (int i = 0; i < 19; i++) {
    printf("\x1b[38;5;%dm%c", x[((i+z)*n/19%n)], buffer[i]);
  }
  z += 2;
  printf("\x1b[39m]\n");
}

void;
