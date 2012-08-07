#define outerr stderr, __FILE__, __LINE__

#define round(x) floor(x + 0.5)

void errexit(FILE *fp, char *file, int line, char *errstr, ...);
void jobstate(char *fmt, ...);
void jobprog(long percent);

FILE * opencheck(char *fname, char *mode);

char *
currenttime();

char *
args(int argc, char **argv);
