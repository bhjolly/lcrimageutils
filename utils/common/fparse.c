#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fparse.h"

char *
fextn(char *fname)
{
  int len = strlen(fname);
  int dot = len - 1;
  char *string;
  
  while(fname[dot] != '.' && dot >= 0) dot--;

  string = (char *)calloc(len - dot, sizeof(char));

  strcpy(string, &fname[dot]);
  
  return(string);
}

char *
froot(char *fname)
{
  int len   = strlen(fname);
  int slash = len - 1;
  int dot   = 0;
  char *string;
  
  while(fname[slash] != '/' && slash >= 0) slash--;

  if(slash >= 0) dot = slash; 
  
  while(fname[dot] != '.' && dot < len) dot++;
  
  string = (char *)calloc(dot - slash, sizeof(char));

  strncpy(string, &fname[slash + 1], dot - slash - 1);
  
  return(string);
}

char *
fnpth(char *fname)
{
  int len   = strlen(fname);
  int slash = len - 1;
  char *string;
  
  while(fname[slash] != '/' && slash >= 0) slash--;

  string = (char *)calloc(len - slash, sizeof(char));

  strncpy(string, &fname[slash + 1], len - slash - 1);
  
  return(string);
}

char *
fpath(char *fname)
{
  int slash = strlen(fname) - 1;
  char *string;
  
  while(fname[slash] != '/' && slash >= 0) slash--;

  if(slash < 0)     /* no path */
    return(".");
  else
    if(!slash)      /* path is a single slash */
      return("/");
  
  string = (char *)calloc(slash + 1, sizeof(char));

  strncpy(string, fname, slash);
  
  return(string);
}

char *
fbase(char *fname)
{
  int len   = strlen(fname);
  int slash = len - 1;
  int dot   = 0;
  char *string;
  
  while(fname[slash] != '/' && slash >= 0) slash--;

  if(slash >= 0) dot = slash; 
  
  while(fname[dot] != '.' && dot < len) dot++;
  
  string = (char *)calloc(dot + 1, sizeof(char));

  strncpy(string, fname, dot);
  
  return(string);
}
