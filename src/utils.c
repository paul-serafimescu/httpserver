#include <stdio.h>
#include <dirent.h>
#include <string.h>
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

/*
░█░█░█░█░█▀▀░█▀█
░█▄█░█▀█░█▀▀░█░█
░▀░▀░▀░▀░▀▀▀░▀░▀
░▀█▀░█░█░█▀▀
░░█░░█▀█░█▀▀
░░▀░░▀░▀░▀▀▀
░▀█▀░█▄█░█▀█░█▀█░█▀▀░▀█▀░█▀█░█▀▄
░░█░░█░█░█▀▀░█░█░▀▀█░░█░░█░█░█▀▄
░▀▀▀░▀░▀░▀░░░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░▀
░▀█▀░█▀▀
░░█░░▀▀█
░▀▀▀░▀▀▀
░█▀▀░█░█░█▀▀
░▀▀█░█░█░▀▀█
░▀▀▀░▀▀▀░▀▀▀
*/

void;
