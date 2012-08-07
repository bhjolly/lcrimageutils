#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "gutils.h"

void
errexit(FILE *fp, char *file, int line, char *errstr, ...)
{
  va_list args;

  fprintf(fp,"%s:%d: ", file, line);
  va_start(args, errstr);
  vfprintf(fp, errstr, args);
  va_end(args);
  fprintf(fp,"\n");
  exit(0);
}

void
jobstate(char *fmt, ...)
{
  va_list args;
  
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr,"\n");
}

void
jobprog(long percent)
{
  if(percent < 100)
    fprintf(stderr,"\r%ld %%", percent);
  else
    fprintf(stderr,"\r%ld %%\n", percent);
}

FILE *
opencheck(char *fname, char *mode)
{
  FILE    *fp;
            
  fp = fopen(fname, mode);

  if(!fp)
    {
      fprintf(stderr, "sorry, can't open %s\n", fname);
      exit(0);
    }
  return(fp);
}

char *
currenttime()
{
  time_t t;
  struct tm *tm;  

  time(&t);

  tm = localtime(&t);

  return(asctime(tm));
}

char *
args(int argc, char **argv)
{
  int i;
  int length = 0;
  char *args;
  
  for(i = 1; i < argc; i++)
    length = length + strlen(argv[i]) + 1;  /* strings and spaces */

  args = (char *)calloc(length + 1, sizeof(char));  /* allocate argument string length */ 

  /* fill argument string */

  for(i = 1; i < argc; i++)
    {
      strcat(args, argv[i]);
      strcat(args, " ");
    }

  return(args);
}
